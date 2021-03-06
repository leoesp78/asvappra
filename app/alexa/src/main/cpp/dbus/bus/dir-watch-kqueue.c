#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "../config.h"
#include "../dbus-watch.h"
#include "../dbus-internals.h"
#include "../dbus-list.h"
#include "../dbus-sysdeps-unix.h"
#include "dir-watch.h"
#include "bus.h"

#define MAX_DIRS_TO_WATCH 128

static int kq = -1;
static int fds[MAX_DIRS_TO_WATCH];
static char *dirs[MAX_DIRS_TO_WATCH];
static int num_fds = 0;
static DBusWatch *watch = NULL;
static DBusLoop *loop = NULL;
static dbus_bool_t _handle_kqueue_watch(DBusWatch *watch, unsigned int flags, void *data) {
  /*struct kevent ev;
  struct timespec nullts = { 0, 0 };
  int res;
  pid_t pid;
  res = kevent(kq, NULL, 0, &ev, 1, &nullts);
  usleep(500000);
  if (res > 0) {
      pid = getpid();
      _dbus_verbose("Sending SIGHUP signal on reception of a kevent\n");
      (void)kill(pid, SIGHUP);
  } else if (res < 0 && errno == EBADF) {
      kq = -1;
      if (watch != NULL) {
          _dbus_loop_remove_watch(loop, watch);
          _dbus_watch_invalidate(watch);
          _dbus_watch_unref(watch);
          watch = NULL;
      }
      pid = getpid();
      _dbus_verbose("Sending SIGHUP signal since kqueue has been closed\n");
      (void)kill(pid, SIGHUP);
  }*/
  return TRUE;
}
static void _shutdown_kqueue(void *data) {
  int i;
  if (kq < 0) return;
  for (i = 0; i < MAX_DIRS_TO_WATCH; i++) {
      if (fds[i] >= 0) {
          close(fds[i]);
          fds[i] = -1;
      }
      if (dirs[i] != NULL) {
          dbus_free(dirs[i]);
          dirs[i] = NULL;
      }
  }
  if (loop) {
      _dbus_loop_remove_watch(loop, watch);
      _dbus_loop_unref(loop);
      loop = NULL;
  }
  if (watch) {
      _dbus_watch_invalidate(watch);
      _dbus_watch_unref(watch);
      watch = NULL;
  }
  close(kq);
  kq = -1;
}
static int _init_kqueue(BusContext *context) {
  if (kq < 0) {
      //kq = kqueue();
      if (kq < 0) {
          _dbus_warn("Cannot create kqueue; error '%s'", _dbus_strerror(errno));
          goto out;
      }
      loop = bus_context_get_loop(context);
      _dbus_loop_ref(loop);
      watch = _dbus_watch_new(kq, DBUS_WATCH_READABLE, TRUE, _handle_kqueue_watch, NULL, NULL);
      if (watch == NULL) {
          _dbus_warn("Unable to create kqueue watch");
          goto out1;
      }
      if (!_dbus_loop_add_watch(loop, watch)) {
          _dbus_warn("Unable to add reload watch to main loop");
          goto out2;
      }
      if (!_dbus_register_shutdown_func(_shutdown_kqueue, NULL)) {
          _dbus_warn("Unable to register shutdown function");
          goto out3;
      }
  }
  return 1;
out3:
  _dbus_loop_remove_watch(loop, watch);
out2:
  if (watch) {
      _dbus_watch_invalidate(watch);
      _dbus_watch_unref(watch);
      watch = NULL;
  }
out1:
  if (kq >= 0) {
      close(kq);
      kq = -1;
  }
  if (loop) {
      _dbus_loop_unref(loop);
      loop = NULL;
  }
out:
  return 0;
}
void bus_set_watched_dirs(BusContext *context, DBusList **directories) {
  /*int new_fds[MAX_DIRS_TO_WATCH];
  char *new_dirs[MAX_DIRS_TO_WATCH];
  DBusList *link;
  int i, j, fd;
  struct kevent ev;
#ifdef O_CLOEXEC
  dbus_bool_t cloexec_done = 0;
#endif
  if (!_init_kqueue(context)) return;
  for (i = 0; i < MAX_DIRS_TO_WATCH; i++) {
      new_fds[i] = -1;
      new_dirs[i] = NULL;
  }
  i = 0;
  link = _dbus_list_get_first_link(directories);
  while(link != NULL) {
      new_dirs[i++] = (char*)link->data;
      link = _dbus_list_get_next_link(directories, link);
  }
  for (i = 0; new_dirs[i]; i++) {
      for (j = 0; j < num_fds; j++) {
          if (dirs[j] && strcmp(new_dirs[i], dirs[j]) == 0) {
              new_fds[i] = fds[j];
              new_dirs[i] = dirs[j];
              fds[j] = -1;
              dirs[j] = NULL;
              break;
          }
      }
  }
  for (j = 0; j < num_fds; j++) {
      if (fds[j] != -1) {
          close (fds[j]);
          dbus_free(dirs[j]);
          fds[j] = -1;
          dirs[j] = NULL;
      }
  }
  for (i = 0; new_dirs[i]; i++) {
      if (new_fds[i] == -1) {
      #ifdef O_CLOEXEC
          fd = open (new_dirs[i], O_RDONLY | O_CLOEXEC);
          cloexec_done = (fd >= 0);
          if (fd < 0 && errno == EINVAL)
      #endif
          {
              fd = open(new_dirs[i], O_RDONLY);
          }
          if (fd < 0) {
              if (errno != ENOENT) {
                  _dbus_warn("Cannot open directory '%s'; error '%s'", new_dirs[i], _dbus_strerror(errno));
                  return;
              } else {
                  new_fds[i] = -1;
                  new_dirs[i] = NULL;
                  continue;
              }
          }
      #ifdef O_CLOEXEC
          if (!cloexec_done)
      #endif
          {
              _dbus_fd_set_close_on_exec(fd);
          }
          EV_SET (&ev, fd, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR, NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_RENAME, 0, 0);
          if (kevent(kq, &ev, 1, NULL, 0, NULL) == -1) {
              _dbus_warn("Cannot setup a kevent for '%s'; error '%s'", new_dirs[i], _dbus_strerror(errno));
              close(fd);
              return;
          }
          new_fds[i] = fd;
          new_dirs[i] = _dbus_strdup (new_dirs[i]);
          if (!new_dirs[i]) {
              close (fd);
              new_fds[i] = -1;
          }
      }
  }
  num_fds = i;
  for (i = 0; i < MAX_DIRS_TO_WATCH; i++) {
      fds[i] = new_fds[i];
      dirs[i] = new_dirs[i];
  }*/
}