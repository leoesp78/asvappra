#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include "../config.h"
#include "../dbus-internals.h"
#include "../dbus-watch.h"
#include "../dbus-sysdeps-unix.h"
#include "bus.h"
#include "driver.h"
#include "selinux.h"
#include "apparmor.h"
#include "audit.h"

static BusContext *context;
#ifdef DBUS_UNIX
static DBusSocket reload_pipe[2];
#define RELOAD_READ_END 0
#define RELOAD_WRITE_END 1
static void close_reload_pipe (DBusWatch **);
typedef enum {
   ACTION_RELOAD = 'r',
   ACTION_QUIT = 'q'
} SignalAction;
static void signal_handler(int sig) {
  int saved_errno = errno;
  switch (sig) {
      case SIGHUP: {
              DBusString str;
              char action[2] = { ACTION_RELOAD, '\0' };
              _dbus_string_init_const (&str, action);
              if ((reload_pipe[RELOAD_WRITE_END].fd > 0) && !_dbus_write_socket (reload_pipe[RELOAD_WRITE_END], &str, 0, 1)) {
                  static const char message[] = "Unable to write to reload pipe - buffer full?\n";
                  if (write (STDERR_FILENO, message, strlen (message)) != (ssize_t) strlen (message));
              }
          }
          break;
      case SIGTERM: {
              DBusString str;
              char action[2] = { ACTION_QUIT, '\0' };
              _dbus_string_init_const (&str, action);
              if ((reload_pipe[RELOAD_WRITE_END].fd < 0) || !_dbus_write_socket (reload_pipe[RELOAD_WRITE_END], &str, 0, 1)) {
                  static const char message[] = "Unable to write termination signal to pipe - buffer full?\nWill exit instead.\n";
                  if (write (STDERR_FILENO, message, strlen (message)) != (ssize_t) strlen (message));
                  _exit (1);
              }
          }
          break;
  }
  errno = saved_errno;
}
#endif
static void usage(void) _DBUS_GNUC_NORETURN;
static void usage(void) {
  fprintf (stderr,
      DBUS_DAEMON_NAME
      " [--version]"
      " [--session]"
      " [--system]"
      " [--config-file=FILE]"
      " [--print-address[=DESCRIPTOR]]"
      " [--print-pid[=DESCRIPTOR]]"
      " [--introspect]"
      " [--address=ADDRESS]"
      " [--nopidfile]"
      " [--nosyslog]"
      " [--syslog]"
      " [--syslog-only]"
      " [--nofork]"
#ifdef DBUS_UNIX
      " [--fork]"
      " [--systemd-activation]"
#endif
      "\n");
  exit(1);
}
static void version(void) _DBUS_GNUC_NORETURN;
static void version(void) {
  printf("D-Bus Message Bus Daemon %s\nCopyright (C) 2002, 2003 Red Hat, Inc., CodeFactory AB, and others\nThis is free software; see the source for copying "
         "conditions.\nThere is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n", DBUS_VERSION_STRING);
  exit(0);
}
static void introspect(void) _DBUS_GNUC_NORETURN;
static void introspect(void) {
  DBusString xml;
  const char *v_STRING;
  if (!_dbus_string_init(&xml)) goto oom;
  if (!bus_driver_generate_introspect_string(&xml, TRUE, NULL)) {
      _dbus_string_free(&xml);
      goto oom;
  }
  v_STRING = _dbus_string_get_const_data(&xml);
  printf("%s\n", v_STRING);
  exit(0);
oom:
  _dbus_warn("Can not introspect - Out of memory");
  exit(1);
}
static void check_two_config_files(const DBusString *config_file, const char *extra_arg) {
  if (_dbus_string_get_length(config_file) > 0) {
      fprintf(stderr, "--%s specified but configuration file %s already requested\n", extra_arg, _dbus_string_get_const_data(config_file));
      exit(1);
  }
}
static void check_two_addresses(const DBusString *address, const char *extra_arg) {
  if (_dbus_string_get_length(address) > 0) {
      fprintf(stderr, "--%s specified but address %s already requested\n", extra_arg, _dbus_string_get_const_data(address));
      exit(1);
  }
}
static void check_two_addr_descriptors(const DBusString *addr_fd, const char *extra_arg) {
  if (_dbus_string_get_length(addr_fd) > 0) {
      fprintf(stderr, "--%s specified but printing address to %s already requested\n", extra_arg, _dbus_string_get_const_data(addr_fd));
      exit(1);
  }
}
static void check_two_pid_descriptors(const DBusString *pid_fd, const char *extra_arg) {
  if (_dbus_string_get_length(pid_fd) > 0) {
      fprintf(stderr, "--%s specified but printing pid to %s already requested\n", extra_arg, _dbus_string_get_const_data(pid_fd));
      exit(1);
  }
}
#ifdef DBUS_UNIX
static dbus_bool_t handle_reload_watch(DBusWatch *watch, unsigned int flags, void *data) {
  DBusError error;
  DBusString str;
  char *action_str;
  char action = '\0';
  while(!_dbus_string_init(&str)) _dbus_wait_for_memory();
  if ((reload_pipe[RELOAD_READ_END].fd > 0) && _dbus_read_socket(reload_pipe[RELOAD_READ_END], &str, 1) != 1) {
      _dbus_warn("Couldn't read from reload pipe.");
      close_reload_pipe(&watch);
      return TRUE;
  }
  action_str = _dbus_string_get_data(&str);
  if (action_str != NULL) action = action_str[0];
  _dbus_string_free(&str);
  dbus_error_init(&error);
  switch(action) {
      case ACTION_RELOAD:
          if (! bus_context_reload_config(context, &error)) {
              _DBUS_ASSERT_ERROR_IS_SET(&error);
              _dbus_assert(dbus_error_has_name(&error, DBUS_ERROR_FAILED) || dbus_error_has_name(&error, DBUS_ERROR_NO_MEMORY));
              _dbus_warn("Unable to reload configuration: %s", error.message);
              dbus_error_free(&error);
          }
          break;
      case ACTION_QUIT: {
              DBusLoop *loop;
              loop = bus_context_get_loop(context);
              if (loop != NULL) _dbus_loop_quit(loop);
          }
          break;
  }
  return TRUE;
}
static void setup_reload_pipe(DBusLoop *loop) {
  DBusError error;
  DBusWatch *watch;
  dbus_error_init(&error);
  if (!_dbus_socketpair(&reload_pipe[0], &reload_pipe[1], TRUE, &error)) {
      _dbus_warn("Unable to create reload pipe: %s", error.message);
      dbus_error_free(&error);
      exit(1);
  }
  watch = _dbus_watch_new(_dbus_socket_get_pollable(reload_pipe[RELOAD_READ_END]),DBUS_WATCH_READABLE, TRUE, handle_reload_watch, NULL,
           NULL);
  if (watch == NULL) {
      _dbus_warn("Unable to create reload watch: %s", error.message);
      dbus_error_free(&error);
      exit(1);
  }
  if (!_dbus_loop_add_watch(loop, watch)) {
      _dbus_warn("Unable to add reload watch to main loop: %s", error.message);
      dbus_error_free(&error);
      exit(1);
  }
}
static void close_reload_pipe(DBusWatch **watch) {
    _dbus_loop_remove_watch(bus_context_get_loop(context), *watch);
    _dbus_watch_invalidate(*watch);
    _dbus_watch_unref(*watch);
    *watch = NULL;
    _dbus_close_socket(reload_pipe[RELOAD_READ_END], NULL);
    _dbus_socket_invalidate(&reload_pipe[RELOAD_READ_END]);
    _dbus_close_socket(reload_pipe[RELOAD_WRITE_END], NULL);
    _dbus_socket_invalidate(&reload_pipe[RELOAD_WRITE_END]);
}
#endif
int main(int argc, char **argv) {
  DBusError error;
  DBusString config_file;
  DBusString address;
  DBusString addr_fd;
  DBusString pid_fd;
  const char *prev_arg;
  DBusPipe print_addr_pipe;
  DBusPipe print_pid_pipe;
  int i;
  dbus_bool_t print_address;
  dbus_bool_t print_pid;
  BusContextFlags flags;
#ifdef DBUS_UNIX
  const char *error_str;
  if (!_dbus_ensure_standard_fds(DBUS_FORCE_STDIN_NULL, &error_str)) {
      fprintf(stderr,"dbus-daemon: fatal error setting up standard fds: %s: %s\n", error_str, _dbus_strerror(errno));
      return 1;
  }
#endif
  if (!_dbus_string_init(&config_file)) return 1;
  if (!_dbus_string_init(&address)) return 1;
  if (!_dbus_string_init(&addr_fd)) return 1;
  if (!_dbus_string_init(&pid_fd)) return 1;
  print_address = FALSE;
  print_pid = FALSE;
  flags = BUS_CONTEXT_FLAG_WRITE_PID_FILE;
  prev_arg = NULL;
  i = 1;
  while(i < argc) {
      const char *arg = argv[i];
      if (strcmp(arg, "--help") == 0 || strcmp(arg, "-h") == 0 || strcmp(arg, "-?") == 0) usage();
      else if (strcmp(arg, "--version") == 0) version();
      else if (strcmp(arg, "--introspect") == 0) introspect();
      else if (strcmp(arg, "--nosyslog") == 0) {
          flags &= ~BUS_CONTEXT_FLAG_SYSLOG_ALWAYS;
          flags |= BUS_CONTEXT_FLAG_SYSLOG_NEVER;
      } else if (strcmp(arg, "--syslog") == 0) {
          flags &= ~BUS_CONTEXT_FLAG_SYSLOG_NEVER;
          flags |= BUS_CONTEXT_FLAG_SYSLOG_ALWAYS;
      } else if (strcmp(arg, "--syslog-only") == 0) {
          flags &= ~BUS_CONTEXT_FLAG_SYSLOG_NEVER;
          flags |= (BUS_CONTEXT_FLAG_SYSLOG_ALWAYS|BUS_CONTEXT_FLAG_SYSLOG_ONLY);
      } else if (strcmp(arg, "--nofork") == 0) {
          flags &= ~BUS_CONTEXT_FLAG_FORK_ALWAYS;
          flags |= BUS_CONTEXT_FLAG_FORK_NEVER;
      }
#ifdef DBUS_UNIX
      else if (strcmp(arg, "--fork") == 0) {
          flags &= ~BUS_CONTEXT_FLAG_FORK_NEVER;
          flags |= BUS_CONTEXT_FLAG_FORK_ALWAYS;
      } else if (strcmp(arg, "--systemd-activation") == 0) flags |= BUS_CONTEXT_FLAG_SYSTEMD_ACTIVATION;
#endif
      else if (strcmp(arg, "--nopidfile") == 0) flags &= ~BUS_CONTEXT_FLAG_WRITE_PID_FILE;
      else if (strcmp(arg, "--system") == 0) {
          check_two_config_files(&config_file, "system");
          if (!_dbus_get_system_config_file(&config_file)) exit(1);
      } else if (strcmp(arg, "--session") == 0) {
          check_two_config_files(&config_file, "session");
          if (!_dbus_get_session_config_file(&config_file)) exit(1);
      } else if (strstr(arg, "--config-file=") == arg) {
          const char *file;
          check_two_config_files(&config_file, "config-file");
          file = strchr(arg, '=');
          ++file;
          if (!_dbus_string_append(&config_file, file)) exit(1);
      } else if (prev_arg && strcmp(prev_arg, "--config-file") == 0) {
          check_two_config_files(&config_file, "config-file");
          if (!_dbus_string_append(&config_file, arg)) exit(1);
      } else if (strcmp(arg, "--config-file") == 0);
      else if (strstr(arg, "--address=") == arg) {
          const char *file;
          check_two_addresses(&address, "address");
          file = strchr(arg, '=');
          ++file;
          if (!_dbus_string_append(&address, file)) exit(1);
      } else if (prev_arg && strcmp(prev_arg, "--address") == 0) {
          check_two_addresses(&address, "address");
          if (!_dbus_string_append(&address, arg)) exit(1);
      } else if (strcmp(arg, "--address") == 0);
      else if (strstr(arg, "--print-address=") == arg) {
          const char *desc;
          check_two_addr_descriptors(&addr_fd, "print-address");
          desc = strchr(arg, '=');
          ++desc;
          if (!_dbus_string_append(&addr_fd, desc)) exit(1);
          print_address = TRUE;
      } else if (prev_arg && strcmp(prev_arg, "--print-address") == 0) {
          check_two_addr_descriptors(&addr_fd, "print-address");
          if (!_dbus_string_append(&addr_fd, arg)) exit(1);
          print_address = TRUE;
      } else if (strcmp (arg, "--print-address") == 0) print_address = TRUE;
      else if (strstr (arg, "--print-pid=") == arg) {
          const char *desc;
          check_two_pid_descriptors(&pid_fd, "print-pid");
          desc = strchr(arg, '=');
          ++desc;
          if (!_dbus_string_append(&pid_fd, desc)) exit(1);
          print_pid = TRUE;
      } else if (prev_arg && strcmp(prev_arg, "--print-pid") == 0) {
          check_two_pid_descriptors(&pid_fd, "print-pid");
          if (!_dbus_string_append(&pid_fd, arg)) exit(1);
          print_pid = TRUE;
      } else if (strcmp (arg, "--print-pid") == 0) print_pid = TRUE;
      else usage();
      prev_arg = arg;
      ++i;
  }
  if (_dbus_string_get_length(&config_file) == 0) {
      fprintf(stderr, "No configuration file specified.\n");
      usage();
  }
  _dbus_pipe_invalidate(&print_addr_pipe);
  if (print_address) {
      _dbus_pipe_init_stdout(&print_addr_pipe);
      if (_dbus_string_get_length(&addr_fd) > 0) {
          long val;
          int end;
          if (!_dbus_string_parse_int(&addr_fd, 0, &val, &end) || end != _dbus_string_get_length(&addr_fd) || val < 0 || val > _DBUS_INT_MAX) {
              fprintf(stderr, "Invalid file descriptor: \"%s\"\n", _dbus_string_get_const_data(&addr_fd));
              exit(1);
          }
          _dbus_pipe_init(&print_addr_pipe, val);
      }
  }
  _dbus_string_free(&addr_fd);
  _dbus_pipe_invalidate(&print_pid_pipe);
  if (print_pid) {
      _dbus_pipe_init_stdout(&print_pid_pipe);
      if (_dbus_string_get_length(&pid_fd) > 0) {
          long val;
          int end;
          if (!_dbus_string_parse_int(&pid_fd, 0, &val, &end) || end != _dbus_string_get_length(&pid_fd) || val < 0 || val > _DBUS_INT_MAX) {
              fprintf(stderr, "Invalid file descriptor: \"%s\"\n", _dbus_string_get_const_data(&pid_fd));
              exit(1);
          }
          _dbus_pipe_init(&print_pid_pipe, val);
      }
  }
  _dbus_string_free(&pid_fd);
  if (!bus_selinux_pre_init()) {
      _dbus_warn("SELinux pre-initialization failed");
      exit(1);
  }
  if (!bus_apparmor_pre_init()) {
      _dbus_warn("AppArmor pre-initialization failed: out of memory");
      exit(1);
  }
  dbus_error_init(&error);
  context = bus_context_new(&config_file, flags, &print_addr_pipe, &print_pid_pipe,_dbus_string_get_length(&address) > 0 ? &address : NULL, &error);
  _dbus_string_free(&config_file);
  if (context == NULL) {
      _dbus_warn("Failed to start message bus: %s", error.message);
      dbus_error_free(&error);
      exit(1);
  }
#ifdef DBUS_UNIX
  setup_reload_pipe (bus_context_get_loop(context));
  _dbus_set_signal_handler(SIGTERM, signal_handler);
  _dbus_set_signal_handler(SIGHUP, signal_handler);
#endif
  _dbus_verbose("We are on D-Bus...\n");
  _dbus_loop_run (bus_context_get_loop(context));
  bus_context_shutdown(context);
  bus_context_unref(context);
  bus_selinux_shutdown();
  bus_apparmor_shutdown();
  bus_audit_shutdown();
  return 0;
}