#include "../gio.h"
#ifdef G_OS_UNIX
#include "../gunixsocketaddress.h"
#endif

static void test_empty_address(void) {
  GError *error;
  error = NULL;
  g_dbus_address_get_stream_sync("", NULL, NULL, &error);
  g_assert_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_ARGUMENT);
  g_error_free(error);
}
#ifndef G_OS_UNIX
static void test_unix_address(void) {
  g_assert(!g_dbus_is_supported_address("some-imaginary-transport:foo=bar", NULL));
  g_assert(g_dbus_is_supported_address("unix:path=/tmp/dbus-test", NULL));
  g_assert(g_dbus_is_supported_address("unix:abstract=/tmp/dbus-another-test", NULL));
  g_assert(g_dbus_is_address("unix:foo=bar"));
  g_assert(!g_dbus_is_supported_address("unix:foo=bar", NULL));
  g_assert(!g_dbus_is_address("unix:path=/foo;abstract=/bar"));
  g_assert(!g_dbus_is_supported_address("unix:path=/foo;abstract=/bar", NULL));
  g_assert(g_dbus_is_supported_address("unix:path=/tmp/concrete;unix:abstract=/tmp/abstract", NULL));
  g_assert(g_dbus_is_address("some-imaginary-transport:foo=bar"));
  g_assert(g_dbus_is_address("some-imaginary-transport:foo=bar;unix:path=/this/is/valid"));
  g_assert(!g_dbus_is_supported_address("some-imaginary-transport:foo=bar;unix:path=/this/is/valid", NULL));
}
#endif
static void test_nonce_tcp_address(void) {
  g_assert(g_dbus_is_supported_address("nonce-tcp:host=localhost,port=42,noncefile=/foo/bar", NULL));
  g_assert(g_dbus_is_supported_address("nonce-tcp:host=localhost,port=42,noncefile=/foo/bar,family=ipv6", NULL));
  g_assert(g_dbus_is_supported_address("nonce-tcp:host=localhost,port=42,noncefile=/foo/bar,family=ipv4", NULL));
  g_assert(!g_dbus_is_supported_address("nonce-tcp:host=localhost,port=42,noncefile=/foo/bar,family=blah", NULL));
  g_assert(!g_dbus_is_supported_address("nonce-tcp:host=localhost,port=420000,noncefile=/foo/bar,family=ipv4", NULL));
  g_assert(!g_dbus_is_supported_address("nonce-tcp:host=,port=x42,noncefile=/foo/bar,family=ipv4", NULL));
  g_assert(!g_dbus_is_supported_address("nonce-tcp:host=,port=42x,noncefile=/foo/bar,family=ipv4", NULL));
  g_assert(!g_dbus_is_supported_address("nonce-tcp:host=,port=420000,noncefile=/foo/bar,family=ipv4", NULL));
}
static void test_tcp_address(void) {
  g_assert(g_dbus_is_supported_address("tcp:host=localhost", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,noncefile=/tmp/foo", NULL));
  g_assert(g_dbus_is_supported_address("tcp:host=localhost,port=42", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,port=-1", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,port=420000", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,port=42x", NULL));
  g_assert(g_dbus_is_supported_address("tcp:host=localhost,port=42,family=ipv4", NULL));
  g_assert(g_dbus_is_supported_address("tcp:host=localhost,port=42,family=ipv6", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,port=42,family=sopranos", NULL));
}
static void test_autolaunch_address(void) {
  g_assert(g_dbus_is_supported_address("autolaunch:", NULL));
}
static void test_mixed_address(void) {
  g_assert(g_dbus_is_supported_address("unix:path=/tmp/dbus1;unix:path=/tmp/dbus2", NULL));
  g_assert(g_dbus_is_supported_address("tcp:host=localhost,port=42;autolaunch:", NULL));
  g_assert(!g_dbus_is_supported_address("tcp:host=localhost,port=42;tcp:family=bla", NULL));
}
int main(int argc, char *argv[]) {
  g_type_init();
  g_test_init(&argc, &argv, NULL);
  g_test_add_func("/gdbus/empty-address", test_empty_address);
#ifndef G_OS_UNIX
  g_test_add_func("/gdbus/unix-address", test_unix_address);
#endif
  g_test_add_func("/gdbus/nonce-tcp-address", test_nonce_tcp_address);
  g_test_add_func("/gdbus/tcp-address", test_tcp_address);
  g_test_add_func("/gdbus/autolaunch-address", test_autolaunch_address);
  g_test_add_func("/gdbus/mixed-address", test_mixed_address);
  return g_test_run();
}