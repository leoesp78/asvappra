#ifndef BUS_SIGNALS_H
#define BUS_SIGNALS_H

#include "../dbus.h"
#include "../dbus-string.h"
#include "../dbus-sysdeps.h"
#include "connection.h"

typedef enum {
  BUS_MATCH_MESSAGE_TYPE = 1 << 0,
  BUS_MATCH_INTERFACE = 1 << 1,
  BUS_MATCH_MEMBER = 1 << 2,
  BUS_MATCH_SENDER = 1 << 3,
  BUS_MATCH_DESTINATION = 1 << 4,
  BUS_MATCH_PATH = 1 << 5,
  BUS_MATCH_ARGS = 1 << 6,
  BUS_MATCH_PATH_NAMESPACE = 1 << 7,
  BUS_MATCH_CLIENT_IS_EAVESDROPPING = 1 << 8
} BusMatchFlags;
BusMatchRule* bus_match_rule_new(DBusConnection *matches_go_to);
BusMatchRule* bus_match_rule_ref(BusMatchRule *rule);
void bus_match_rule_unref(BusMatchRule *rule);
dbus_bool_t bus_match_rule_set_message_type(BusMatchRule *rule, int type);
dbus_bool_t bus_match_rule_set_interface(BusMatchRule *rule, const char *interface);
dbus_bool_t bus_match_rule_set_member(BusMatchRule *rule, const char *member);
dbus_bool_t bus_match_rule_set_sender(BusMatchRule *rule, const char *sender);
dbus_bool_t bus_match_rule_set_destination(BusMatchRule *rule, const char *destination);
dbus_bool_t bus_match_rule_set_path(BusMatchRule *rule, const char *path, dbus_bool_t is_namespace);
dbus_bool_t bus_match_rule_set_arg(BusMatchRule *rule, int arg, const DBusString *value, dbus_bool_t is_path, dbus_bool_t is_namespace);
void bus_match_rule_set_client_is_eavesdropping(BusMatchRule *rule, dbus_bool_t is_eavesdropping);
dbus_bool_t bus_match_rule_get_client_is_eavesdropping(BusMatchRule *rule);
BusMatchRule* bus_match_rule_parse(DBusConnection *matches_go_to, const DBusString *rule_text, DBusError *error);
#ifdef DBUS_ENABLE_STATS
dbus_bool_t bus_match_rule_dump(BusMatchmaker *matchmaker, DBusConnection *conn_filter, DBusMessageIter *arr_iter);
#endif
BusMatchmaker* bus_matchmaker_new(void);
BusMatchmaker* bus_matchmaker_ref(BusMatchmaker *matchmaker);
void bus_matchmaker_unref(BusMatchmaker *matchmaker);
dbus_bool_t bus_matchmaker_add_rule(BusMatchmaker *matchmaker, BusMatchRule *rule);
dbus_bool_t bus_matchmaker_remove_rule_by_value(BusMatchmaker *matchmaker, BusMatchRule *value, DBusError *error);
void bus_matchmaker_remove_rule(BusMatchmaker *matchmaker, BusMatchRule *rule);
void bus_matchmaker_disconnected(BusMatchmaker *matchmaker, DBusConnection *connection);
dbus_bool_t bus_matchmaker_get_recipients(BusMatchmaker *matchmaker, BusConnections *connections, DBusConnection *sender, DBusConnection *addressed_recipient,
                                          DBusMessage *message, DBusList **recipients_p);

#endif