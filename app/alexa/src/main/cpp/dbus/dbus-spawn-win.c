#include <stdio.h>
#include <stdlib.h>
#include "../gio/gregistrysettingsbackend.h"
#include "config.h"
#include "dbus-spawn.h"
#include "dbus-sysdeps.h"
#include "dbus-sysdeps-win.h"
#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-protocol.h"
#define SPAWN_DEBUG
#if !defined(SPAWN_DEBUG) || defined(_MSC_VER)
#define PING()
#else
#define PING() \
fprintf(stderr, "%s:%s:%d\n", __FILE__, __FUNCTION__, __LINE__); \
fflush (stderr)
#endif

struct DBusBabysitter {
  DBusAtomic refcount;
  HANDLE start_sync_event;
  char *log_name;
  int argc;
  char **argv;
  char **envp;
  HANDLE thread_handle;
  HANDLE child_handle;
  DBusSocket socket_to_babysitter;
  DBusSocket socket_to_main;
  DBusWatchList *watches;
  DBusWatch *sitter_watch;
  DBusBabysitterFinishedFunc finished_cb;
  void *finished_data;
  dbus_bool_t have_spawn_errno;
  int spawn_errno;
  dbus_bool_t have_child_status;
  int child_status;
};
static void _dbus_babysitter_trace_ref(DBusBabysitter *sitter, int old_refcount, int new_refcount, const char *why) {
#ifdef DBUS_ENABLE_VERBOSE_MODE
  static int enabled = -1;
  _dbus_trace_ref("DBusBabysitter", sitter, old_refcount, new_refcount, why,"DBUS_BABYSITTER_TRACE", &enabled);
#endif
}
static DBusBabysitter* _dbus_babysitter_new(void) {
  /*DBusBabysitter *sitter;
  dbus_int32_t old_refcount;
  sitter = dbus_new0 (DBusBabysitter, 1);
  if (sitter == NULL) return NULL;
  old_refcount = _dbus_atomic_inc (&sitter->refcount);
  _dbus_babysitter_trace_ref (sitter, old_refcount, old_refcount+1, __FUNCTION__);
  sitter->start_sync_event = CreateEvent (NULL, FALSE, FALSE, NULL);
  if (sitter->start_sync_event == NULL) {
      _dbus_babysitter_unref (sitter);
      return NULL;
  }
  sitter->child_handle = NULL;
  sitter->socket_to_babysitter = sitter->socket_to_main = _dbus_socket_get_invalid ();
  sitter->argc = 0;
  sitter->argv = NULL;
  sitter->envp = NULL;
  sitter->watches = _dbus_watch_list_new ();
  if (sitter->watches == NULL) {
      _dbus_babysitter_unref (sitter);
      return NULL;
  }
  sitter->have_spawn_errno = FALSE;
  sitter->have_child_status = FALSE;
  return sitter;*/
  return NULL;
}
DBusBabysitter *_dbus_babysitter_ref(DBusBabysitter *sitter) {
  dbus_int32_t old_refcount;
  PING();
  _dbus_assert(sitter != NULL);
  old_refcount = _dbus_atomic_inc(&sitter->refcount);
  _dbus_assert(old_refcount > 0);
  _dbus_babysitter_trace_ref(sitter, old_refcount, old_refcount+1, __FUNCTION__);
  return sitter;
}
static void close_socket_to_babysitter (DBusBabysitter *sitter) {
  /*_dbus_verbose("Closing babysitter\n");
  if (sitter->sitter_watch != NULL) {
      _dbus_assert(sitter->watches != NULL);
      _dbus_watch_list_remove_watch(sitter->watches,  sitter->sitter_watch);
      _dbus_watch_invalidate(sitter->sitter_watch);
      _dbus_watch_unref(sitter->sitter_watch);
      sitter->sitter_watch = NULL;
  }
  if (sitter->socket_to_babysitter.sock != INVALID_SOCKET) {
      _dbus_close_socket(sitter->socket_to_babysitter, NULL);
      sitter->socket_to_babysitter.sock = INVALID_SOCKET;
  }*/
}
void _dbus_babysitter_unref(DBusBabysitter *sitter) {
  /*int i;
  dbus_int32_t old_refcount;
  PING();
  _dbus_assert(sitter != NULL);
  old_refcount = _dbus_atomic_dec(&sitter->refcount);
  _dbus_assert(old_refcount > 0);
  _dbus_babysitter_trace_ref(sitter, old_refcount, old_refcount-1, __FUNCTION__);
  if (old_refcount == 1) {
      close_socket_to_babysitter(sitter);
      if (sitter->socket_to_main.sock != INVALID_SOCKET) {
          _dbus_close_socket(sitter->socket_to_main, NULL);
          sitter->socket_to_main.sock = INVALID_SOCKET;
      }
      PING();
      if (sitter->argv != NULL) {
          for (i = 0; i < sitter->argc; i++)
            if (sitter->argv[i] != NULL) {
                dbus_free(sitter->argv[i]);
                sitter->argv[i] = NULL;
            }
          dbus_free(sitter->argv);
          sitter->argv = NULL;
      }
      if (sitter->envp != NULL) {
          char **e = sitter->envp;
          while (*e) dbus_free(*e++);
          dbus_free(sitter->envp);
          sitter->envp = NULL;
      }
      if (sitter->child_handle != NULL) {
          CloseHandle(sitter->child_handle);
          sitter->child_handle = NULL;
          if (sitter->sitter_watch) {
              _dbus_watch_invalidate(sitter->sitter_watch);
              _dbus_watch_unref(sitter->sitter_watch);
              sitter->sitter_watch = NULL;
          }
          if (sitter->watches) _dbus_watch_list_free(sitter->watches);
          if (sitter->start_sync_event != NULL) {
              PING();
              CloseHandle(sitter->start_sync_event);
              sitter->start_sync_event = NULL;
          }
          if (sitter->thread_handle) {
              CloseHandle(sitter->thread_handle);
              sitter->thread_handle = NULL;
          }
          dbus_free(sitter->log_name);
          dbus_free(sitter);
      }
  }*/
}
void _dbus_babysitter_kill_child(DBusBabysitter *sitter) {
  /*PING();
  if (sitter->child_handle == NULL) return;
  PING();
  TerminateProcess(sitter->child_handle, 12345);*/
}
dbus_bool_t _dbus_babysitter_get_child_exited(DBusBabysitter *sitter) {
  PING();
  return (sitter->child_handle == NULL);
}
dbus_bool_t _dbus_babysitter_get_child_exit_status(DBusBabysitter *sitter, int *status) {
  /*if (!_dbus_babysitter_get_child_exited(sitter)) _dbus_assert_not_reached("Child has not exited");
  if (!sitter->have_child_status || sitter->child_status == STILL_ACTIVE) return FALSE;
  *status = sitter->child_status;*/
  return TRUE;
}
void _dbus_babysitter_set_child_exit_error(DBusBabysitter *sitter, DBusError *error) {
  PING();
  if (!_dbus_babysitter_get_child_exited(sitter)) return;
  PING();
  if (sitter->have_spawn_errno) {
      char *emsg = _dbus_win_error_string(sitter->spawn_errno);
      dbus_set_error(error, DBUS_ERROR_SPAWN_EXEC_FAILED,"Failed to execute program %s: %s", sitter->log_name, emsg);
      _dbus_win_free_error_string(emsg);
  } else if (sitter->have_child_status) {
      PING();
      dbus_set_error(error, DBUS_ERROR_SPAWN_CHILD_EXITED,"Process %s exited with status %d", sitter->log_name, sitter->child_status);
  } else {
      PING();
      dbus_set_error(error, DBUS_ERROR_FAILED,"Process %s exited, status unknown", sitter->log_name);
  }
  PING();
}
dbus_bool_t _dbus_babysitter_set_watch_functions(DBusBabysitter *sitter, DBusAddWatchFunction add_function, DBusRemoveWatchFunction remove_function,
                                                 DBusWatchToggledFunction toggled_function, void *data, DBusFreeFunction free_data_function) {
  PING();
  return _dbus_watch_list_set_functions(sitter->watches, add_function, remove_function, toggled_function, data, free_data_function);
}
static dbus_bool_t handle_watch(DBusWatch *watch, unsigned int condition, void *data) {
  DBusBabysitter *sitter = data;
  PING();
  close_socket_to_babysitter(sitter);
  PING();
  if (_dbus_babysitter_get_child_exited(sitter) && sitter->finished_cb != NULL) {
      sitter->finished_cb(sitter, sitter->finished_data);
      sitter->finished_cb = NULL;
  }
  return TRUE;
}
static int protect_argv(char  * const *argv, char ***new_argv) {
  int i;
  int argc = 0;
  while(argv[argc]) ++argc;
  *new_argv = dbus_malloc((argc + 1) * sizeof(char*));
  if (*new_argv == NULL) return -1;
  for (i = 0; i < argc; i++) (*new_argv)[i] = NULL;
  for (i = 0; i < argc; i++) {
      const char *p = argv[i];
      char *q;
      int len = 0;
      int need_dblquotes = FALSE;
      while(*p) {
          if (*p == ' ' || *p == '\t') need_dblquotes = TRUE;
          else if (*p == '"') len++;
          else if (*p == '\\') {
              const char *pp = p;
              while(*pp && *pp == '\\') pp++;
              if (*pp == '"') len++;
          }
          len++;
          p++;
      }
      q = (*new_argv)[i] = dbus_malloc(len + need_dblquotes * 2 + 1);
      if (q == NULL) return -1;
      p = argv[i];
      if (need_dblquotes) *q++ = '"';
      while(*p) {
          if (*p == '"') *q++ = '\\';
          else if (*p == '\\') {
              const char *pp = p;
              while(*pp && *pp == '\\') pp++;
              if (*pp == '"') *q++ = '\\';
          }
          *q++ = *p;
          p++;
      }
      if (need_dblquotes) *q++ = '"';
      *q++ = '\0';
  }
  (*new_argv)[argc] = NULL;
  return argc;
}
static char *compose_string(char **strings, char separator) {
  int i;
  int n = 0;
  char *buf;
  char *p;
  if (!strings || !strings[0]) return 0;
  for (i = 0; strings[i]; i++) n += strlen(strings[i]) + 1;
  n++;
  buf = p = malloc(n);
  if (!buf) return NULL;
  for (i = 0; strings[i]; i++) {
      strcpy(p, strings[i]);
      p += strlen(strings[i]);
      *(p++) = separator;
  }
  p--;
  *(p++) = '\0';
  *p = '\0';
  return buf;
}
static char *build_commandline(char **argv) {
  return compose_string(argv, ' ');
}
static char *build_env_string(char** envp) {
  return compose_string(envp, '\0');
}
static HANDLE spawn_program(char* name, char** argv, char** envp) {
  /*PROCESS_INFORMATION pi = { NULL, 0, 0, 0 };
  STARTUPINFOA si;
  char *arg_string, *env_string;
  BOOL result;
#ifdef DBUS_WINCE
  if (argv && argv[0]) arg_string = build_commandline (argv + 1);
  else arg_string = NULL;
#else
  arg_string = build_commandline(argv);
#endif
  if (!arg_string) return INVALID_HANDLE_VALUE;
  env_string = build_env_string(envp);
  memset(&si, 0, sizeof(si));
  si.cb = sizeof(si);
#ifdef DBUS_WINCE
  result = CreateProcessA(name, arg_string, NULL, NULL, FALSE, 0,
#else
  result = CreateProcessA(NULL, arg_string, NULL, NULL, FALSE, 0,
#endif
			             (LPVOID)env_string, NULL, &si, &pi);
  free(arg_string);
  if (env_string) free(env_string);
  if (!result) return INVALID_HANDLE_VALUE;
  CloseHandle(pi.hThread);
  return pi.hProcess;*/
  return NULL;
}
static DWORD __stdcall babysitter(void *parameter) {
  /*int ret = 0;
  DBusBabysitter *sitter = (DBusBabysitter*)parameter;
  HANDLE handle;
  PING();
  _dbus_verbose("babysitter: spawning %s\n", sitter->log_name);
  PING();
  handle = spawn_program(sitter->log_name, sitter->argv, sitter->envp);
  PING();
  if (handle != INVALID_HANDLE_VALUE) sitter->child_handle = handle;
  else {
      sitter->child_handle = NULL;
      sitter->have_spawn_errno = TRUE;
      sitter->spawn_errno = GetLastError();
  }
  PING();
  SetEvent(sitter->start_sync_event);
  if (sitter->child_handle != NULL) {
      DWORD status;
      PING();
      WaitForSingleObject(sitter->child_handle, INFINITE);
      PING();
      ret = GetExitCodeProcess(sitter->child_handle, &status);
      if (ret) {
          sitter->child_status = status;
          sitter->have_child_status = TRUE;
      }
      CloseHandle (sitter->child_handle);
      sitter->child_handle = NULL;
  }
  PING();
  send(sitter->socket_to_main.sock, " ", 1, 0);
  _dbus_babysitter_unref(sitter);
  return ret ? 0 : 1;*/
  return 0;
}
dbus_bool_t _dbus_spawn_async_with_babysitter(DBusBabysitter **sitter_p, const char *log_name, char * const *argv, char **envp, DBusSpawnFlags flags _DBUS_GNUC_UNUSED,
                                              DBusSpawnChildSetupFunc child_setup _DBUS_GNUC_UNUSED, void *user_data _DBUS_GNUC_UNUSED, DBusError *error) {
  /*DBusBabysitter *sitter;
  DWORD sitter_thread_id;
  _DBUS_ASSERT_ERROR_IS_CLEAR (error);
  _dbus_assert(argv[0] != NULL);
  if (sitter_p != NULL) *sitter_p = NULL;
  PING();
  sitter = _dbus_babysitter_new();
  if (sitter == NULL) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  sitter->log_name = _dbus_strdup(log_name);
  if (sitter->log_name == NULL && log_name != NULL) {
      _DBUS_SET_OOM(error);
      goto out0;
  }
  if (sitter->log_name == NULL) sitter->log_name = _dbus_strdup(argv[0]);
  if (sitter->log_name == NULL) {
      _DBUS_SET_OOM(error);
      goto out0;
  }
  PING();
  if (!_dbus_socketpair(&sitter->socket_to_babysitter, &sitter->socket_to_main, FALSE, error)) goto out0;
  sitter->sitter_watch = _dbus_watch_new(sitter->socket_to_babysitter, DBUS_WATCH_READABLE, TRUE, handle_watch, sitter, NULL);
  PING();
  if (sitter->sitter_watch == NULL) {
      _DBUS_SET_OOM(error);
      goto out0;
  }
  PING();
  if (!_dbus_watch_list_add_watch(sitter->watches,  sitter->sitter_watch)) {
      _dbus_watch_invalidate(sitter->sitter_watch);
      _dbus_watch_unref(sitter->sitter_watch);
      sitter->sitter_watch = NULL;
      _DBUS_SET_OOM(error);
      goto out0;
  }
  sitter->argc = protect_argv (argv, &sitter->argv);
  if (sitter->argc == -1) {
      _DBUS_SET_OOM (error);
      goto out0;
  }
  sitter->envp = envp;
  PING();
  sitter->thread_handle = (HANDLE) CreateThread(NULL, 0, babysitter, _dbus_babysitter_ref (sitter), 0, &sitter_thread_id);
  if (sitter->thread_handle == NULL) {
      PING();
      dbus_set_error_const(error, DBUS_ERROR_SPAWN_FORK_FAILED,"Failed to create new thread");
      goto out0;
  }
  PING();
  WaitForSingleObject(sitter->start_sync_event, INFINITE);
  PING();
  if (sitter_p != NULL) *sitter_p = sitter;
  else _dbus_babysitter_unref(sitter);
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  PING();
  return TRUE;
out0:
  _dbus_babysitter_unref(sitter);*/
  return FALSE;
}
void _dbus_babysitter_set_result_function(DBusBabysitter *sitter, DBusBabysitterFinishedFunc finished, void *user_data) {
  sitter->finished_cb = finished;
  sitter->finished_data = user_data;
}
#define LIVE_CHILDREN(sitter)  ((sitter)->child_handle != NULL)
void _dbus_babysitter_block_for_child_exit(DBusBabysitter *sitter) {
  //WaitForSingleObject(sitter->thread_handle, INFINITE);
}