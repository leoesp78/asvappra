#include "../config.h"
#include "../dbus-list.h"
#include "../dbus-hash.h"
#include "../dbus-internals.h"
#include "../dbus-message-internal.h"
#include "policy.h"
#include "services.h"
#include "test.h"
#include "utils.h"

BusPolicyRule* bus_policy_rule_new(BusPolicyRuleType type, dbus_bool_t allow) {
  BusPolicyRule *rule;
  rule = dbus_new0(BusPolicyRule, 1);
  if (rule == NULL) return NULL;
  rule->type = type;
  rule->refcount = 1;
  rule->allow = allow;
  switch(rule->type) {
      case BUS_POLICY_RULE_USER: rule->d.user.uid = DBUS_UID_UNSET; break;
      case BUS_POLICY_RULE_GROUP: rule->d.group.gid = DBUS_GID_UNSET; break;
      case BUS_POLICY_RULE_SEND:
          rule->d.send.message_type = DBUS_MESSAGE_TYPE_INVALID;
          rule->d.send.requested_reply = rule->allow;
          break;
      case BUS_POLICY_RULE_RECEIVE:
          rule->d.receive.message_type = DBUS_MESSAGE_TYPE_INVALID;
          rule->d.receive.requested_reply = rule->allow;
          break;
      case BUS_POLICY_RULE_OWN: break;
      default: _dbus_assert_not_reached("invalid rule");
  }
  return rule;
}
BusPolicyRule *bus_policy_rule_ref(BusPolicyRule *rule) {
  _dbus_assert(rule->refcount > 0);
  rule->refcount += 1;
  return rule;
}
void bus_policy_rule_unref(BusPolicyRule *rule) {
  _dbus_assert(rule->refcount > 0);
  rule->refcount -= 1;
  if (rule->refcount == 0) {
      switch(rule->type) {
          case BUS_POLICY_RULE_SEND:
              dbus_free(rule->d.send.path);
              dbus_free(rule->d.send.interface);
              dbus_free(rule->d.send.member);
              dbus_free(rule->d.send.error);
              dbus_free(rule->d.send.destination);
              break;
          case BUS_POLICY_RULE_RECEIVE:
              dbus_free(rule->d.receive.path);
              dbus_free(rule->d.receive.interface);
              dbus_free(rule->d.receive.member);
              dbus_free(rule->d.receive.error);
              dbus_free(rule->d.receive.origin);
              break;
          case BUS_POLICY_RULE_OWN: dbus_free(rule->d.own.service_name); break;
          case BUS_POLICY_RULE_USER: case BUS_POLICY_RULE_GROUP: break;
          default: _dbus_assert_not_reached("invalid rule");
      }
      dbus_free(rule);
  }
}
struct BusPolicy {
  int refcount;
  DBusList *default_rules;
  DBusList *mandatory_rules;
  DBusHashTable *rules_by_uid;
  DBusHashTable *rules_by_gid;
  DBusList *at_console_true_rules;
  DBusList *at_console_false_rules;
};
static void free_rule_func(void *data, void *user_data) {
  BusPolicyRule *rule = data;
  bus_policy_rule_unref(rule);
}
static void free_rule_list_func(void *data) {
  DBusList **list = data;
  if (list == NULL) return;
  _dbus_list_foreach(list, free_rule_func, NULL);
  _dbus_list_clear(list);
  dbus_free(list);
}
BusPolicy* bus_policy_new(void) {
  BusPolicy *policy;
  policy = dbus_new0(BusPolicy, 1);
  if (policy == NULL) return NULL;
  policy->refcount = 1;
  policy->rules_by_uid = _dbus_hash_table_new(DBUS_HASH_UINTPTR,NULL, free_rule_list_func);
  if (policy->rules_by_uid == NULL) goto failed;
  policy->rules_by_gid = _dbus_hash_table_new(DBUS_HASH_UINTPTR,NULL, free_rule_list_func);
  if (policy->rules_by_gid == NULL) goto failed;
  return policy;
failed:
  bus_policy_unref(policy);
  return NULL;
}
BusPolicy *bus_policy_ref(BusPolicy *policy) {
  _dbus_assert(policy->refcount > 0);
  policy->refcount += 1;
  return policy;
}
void bus_policy_unref(BusPolicy *policy) {
  _dbus_assert(policy->refcount > 0);
  policy->refcount -= 1;
  if (policy->refcount == 0) {
      _dbus_list_foreach(&policy->default_rules, free_rule_func, NULL);
      _dbus_list_clear(&policy->default_rules);
      _dbus_list_foreach(&policy->mandatory_rules, free_rule_func, NULL);
      _dbus_list_clear(&policy->mandatory_rules);
      _dbus_list_foreach(&policy->at_console_true_rules, free_rule_func, NULL);
      _dbus_list_clear(&policy->at_console_true_rules);
      _dbus_list_foreach(&policy->at_console_false_rules, free_rule_func, NULL);
      _dbus_list_clear(&policy->at_console_false_rules);
      if (policy->rules_by_uid) {
          _dbus_hash_table_unref(policy->rules_by_uid);
          policy->rules_by_uid = NULL;
      }
      if (policy->rules_by_gid) {
          _dbus_hash_table_unref(policy->rules_by_gid);
          policy->rules_by_gid = NULL;
      }
      dbus_free(policy);
  }
}
static dbus_bool_t add_list_to_client(DBusList **list, BusClientPolicy *client) {
  DBusList *link;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      BusPolicyRule *rule = link->data;
      link = _dbus_list_get_next_link(list, link);
      switch(rule->type) {
          case BUS_POLICY_RULE_USER: case BUS_POLICY_RULE_GROUP: break;
          case BUS_POLICY_RULE_OWN: case BUS_POLICY_RULE_SEND: case BUS_POLICY_RULE_RECEIVE:
              if (!bus_client_policy_append_rule(client, rule)) return FALSE;
              break;
          default: _dbus_assert_not_reached("invalid rule");
      }
  }
  return TRUE;
}
BusClientPolicy* bus_policy_create_client_policy(BusPolicy *policy, DBusConnection *connection, DBusError *error) {
  BusClientPolicy *client;
  dbus_uid_t uid;
  dbus_bool_t at_console;
  _dbus_assert(dbus_connection_get_is_authenticated(connection));
  _DBUS_ASSERT_ERROR_IS_CLEAR(error);
  client = bus_client_policy_new();
  if (client == NULL) goto nomem;
  if (!add_list_to_client(&policy->default_rules, client)) goto nomem;
  if (_dbus_hash_table_get_n_entries(policy->rules_by_gid) > 0) {
      unsigned long *groups;
      int n_groups;
      int i;
      if (!bus_connection_get_unix_groups(connection, &groups, &n_groups, error)) goto failed;
      i = 0;
      while(i < n_groups) {
          DBusList **list;
          list = _dbus_hash_table_lookup_uintptr(policy->rules_by_gid, groups[i]);
          if (list != NULL) {
              if (!add_list_to_client(list, client)) {
                  dbus_free(groups);
                  goto nomem;
              }
          }
          ++i;
      }
      dbus_free(groups);
  }
  if (dbus_connection_get_unix_user(connection, &uid)) {
      if (_dbus_hash_table_get_n_entries(policy->rules_by_uid) > 0) {
          DBusList **list;
          list = _dbus_hash_table_lookup_uintptr(policy->rules_by_uid, uid);
          if (list != NULL) {
              if (!add_list_to_client(list, client)) goto nomem;
          }
      }
      at_console = _dbus_unix_user_is_at_console(uid, error);
      if (at_console) {
          if (!add_list_to_client(&policy->at_console_true_rules, client)) goto nomem;
      } else if (dbus_error_is_set(error) == TRUE) goto failed;
      else if (!add_list_to_client(&policy->at_console_false_rules, client)) goto nomem;
  }
  if (!add_list_to_client(&policy->mandatory_rules, client)) goto nomem;
  bus_client_policy_optimize(client);
  return client;
nomem:
  BUS_SET_OOM(error);
failed:
  _DBUS_ASSERT_ERROR_IS_SET(error);
  if (client) bus_client_policy_unref(client);
  return NULL;
}
static dbus_bool_t list_allows_user(dbus_bool_t def, DBusList **list, unsigned long uid, const unsigned long *group_ids, int n_group_ids) {
  DBusList *link;
  dbus_bool_t allowed;
  allowed = def;
  link = _dbus_list_get_first_link(list);
  while(link != NULL) {
      BusPolicyRule *rule = link->data;
      link = _dbus_list_get_next_link(list, link);
      if (rule->type == BUS_POLICY_RULE_USER) {
          _dbus_verbose("List %p user rule uid="DBUS_UID_FORMAT"\n", list, rule->d.user.uid);
          if (rule->d.user.uid == DBUS_UID_UNSET);
          else if (rule->d.user.uid != uid) continue;
      } else if (rule->type == BUS_POLICY_RULE_GROUP) {
          _dbus_verbose("List %p group rule gid="DBUS_GID_FORMAT"\n", list, rule->d.group.gid);
          if (rule->d.group.gid == DBUS_GID_UNSET);
          else {
              int i;
              i = 0;
              while(i < n_group_ids) {
                  if (rule->d.group.gid == group_ids[i]) break;
                  ++i;
              }
              if (i == n_group_ids) continue;
          }
      } else continue;
      allowed = rule->allow;
  }
  return allowed;
}
dbus_bool_t bus_policy_allow_unix_user(BusPolicy *policy, unsigned long uid) {
  dbus_bool_t allowed;
  unsigned long *group_ids;
  int n_group_ids;
  if (!_dbus_unix_groups_from_uid(uid, &group_ids, &n_group_ids)) {
      _dbus_verbose("Did not get any groups for UID %lu\n", uid);
      return FALSE;
  }
  allowed = _dbus_unix_user_is_process_owner(uid);
  allowed = list_allows_user(allowed, &policy->default_rules, uid, group_ids, n_group_ids);
  allowed = list_allows_user(allowed, &policy->mandatory_rules, uid, group_ids, n_group_ids);
  dbus_free(group_ids);
  _dbus_verbose("UID %lu allowed = %d\n", uid, allowed);
  return allowed;
}
dbus_bool_t bus_policy_allow_windows_user(BusPolicy *policy, const char *windows_sid) {
  return _dbus_windows_user_is_process_owner(windows_sid);
}
dbus_bool_t bus_policy_append_default_rule(BusPolicy *policy, BusPolicyRule *rule) {
  if (!_dbus_list_append(&policy->default_rules, rule)) return FALSE;
  bus_policy_rule_ref(rule);
  return TRUE;
}
dbus_bool_t bus_policy_append_mandatory_rule(BusPolicy *policy, BusPolicyRule *rule) {
  if (!_dbus_list_append(&policy->mandatory_rules, rule)) return FALSE;
  bus_policy_rule_ref(rule);
  return TRUE;
}
static DBusList** get_list(DBusHashTable *hash, unsigned long key) {
  DBusList **list;
  list = _dbus_hash_table_lookup_uintptr(hash, key);
  if (list == NULL) {
      list = dbus_new0(DBusList*, 1);
      if (list == NULL) return NULL;
      if (!_dbus_hash_table_insert_uintptr(hash, key, list)) {
          dbus_free(list);
          return NULL;
      }
  }
  return list;
}
dbus_bool_t bus_policy_append_user_rule(BusPolicy *policy, dbus_uid_t uid, BusPolicyRule *rule) {
  DBusList **list;
  list = get_list(policy->rules_by_uid, uid);
  if (list == NULL) return FALSE;
  if (!_dbus_list_append(list, rule)) return FALSE;
  bus_policy_rule_ref(rule);
  return TRUE;
}
dbus_bool_t bus_policy_append_group_rule(BusPolicy *policy, dbus_gid_t gid, BusPolicyRule *rule) {
  DBusList **list;
  list = get_list(policy->rules_by_gid, gid);
  if (list == NULL) return FALSE;
  if (!_dbus_list_append(list, rule)) return FALSE;
  bus_policy_rule_ref(rule);
  return TRUE;
}
dbus_bool_t bus_policy_append_console_rule(BusPolicy *policy, dbus_bool_t at_console, BusPolicyRule *rule) {
  if (at_console) {
      if (!_dbus_list_append(&policy->at_console_true_rules, rule)) return FALSE;
  } else {
      if (!_dbus_list_append(&policy->at_console_false_rules, rule)) return FALSE;
  }
  bus_policy_rule_ref(rule);
  return TRUE;
}
static dbus_bool_t append_copy_of_policy_list(DBusList **list, DBusList **to_append) {
  DBusList *link;
  DBusList *tmp_list;
  tmp_list = NULL;
  link = _dbus_list_get_first_link(to_append);
  while(link != NULL) {
      if (!_dbus_list_append(&tmp_list, link->data)) {
          _dbus_list_clear(&tmp_list);
          return FALSE;
      }
      link = _dbus_list_get_next_link(to_append, link);
  }
  while((link = _dbus_list_pop_first_link(&tmp_list))) {
      bus_policy_rule_ref(link->data);
      _dbus_list_append_link(list, link);
  }
  return TRUE;
}
static dbus_bool_t merge_id_hash(DBusHashTable *dest, DBusHashTable *to_absorb) {
  DBusHashIter iter;
  _dbus_hash_iter_init(to_absorb, &iter);
  while(_dbus_hash_iter_next(&iter)) {
      unsigned long id = _dbus_hash_iter_get_uintptr_key(&iter);
      DBusList **list = _dbus_hash_iter_get_value(&iter);
      DBusList **target = get_list(dest, id);
      if (target == NULL) return FALSE;
      if (!append_copy_of_policy_list(target, list)) return FALSE;
  }
  return TRUE;
}
dbus_bool_t bus_policy_merge(BusPolicy *policy, BusPolicy *to_absorb) {
  if (!append_copy_of_policy_list(&policy->default_rules, &to_absorb->default_rules)) return FALSE;
  if (!append_copy_of_policy_list(&policy->mandatory_rules, &to_absorb->mandatory_rules)) return FALSE;
  if (!append_copy_of_policy_list(&policy->at_console_true_rules, &to_absorb->at_console_true_rules)) return FALSE;
  if (!append_copy_of_policy_list(&policy->at_console_false_rules, &to_absorb->at_console_false_rules)) return FALSE;
  if (!merge_id_hash(policy->rules_by_uid, to_absorb->rules_by_uid)) return FALSE;
  if (!merge_id_hash(policy->rules_by_gid, to_absorb->rules_by_gid)) return FALSE;
  return TRUE;
}
struct BusClientPolicy {
  int refcount;
  DBusList *rules;
};
BusClientPolicy* bus_client_policy_new(void) {
  BusClientPolicy *policy;
  policy = dbus_new0(BusClientPolicy, 1);
  if (policy == NULL) return NULL;
  policy->refcount = 1;
  return policy;
}
BusClientPolicy *bus_client_policy_ref(BusClientPolicy *policy) {
  _dbus_assert(policy->refcount > 0);
  policy->refcount += 1;
  return policy;
}
static void rule_unref_foreach(void *data, void *user_data) {
  BusPolicyRule *rule = data;
  bus_policy_rule_unref(rule);
}
void bus_client_policy_unref(BusClientPolicy *policy) {
  _dbus_assert(policy->refcount > 0);
  policy->refcount -= 1;
  if (policy->refcount == 0) {
      _dbus_list_foreach(&policy->rules, rule_unref_foreach,NULL);
      _dbus_list_clear(&policy->rules);
      dbus_free(policy);
  }
}
static void remove_rules_by_type_up_to(BusClientPolicy *policy, BusPolicyRuleType type, DBusList *up_to) {
  DBusList *link;
  link = _dbus_list_get_first_link(&policy->rules);
  while(link != up_to) {
      BusPolicyRule *rule = link->data;
      DBusList *next = _dbus_list_get_next_link(&policy->rules, link);
      if (rule->type == type) {
          _dbus_list_remove_link(&policy->rules, link);
          bus_policy_rule_unref(rule);
      }
      link = next;
  }
}
void bus_client_policy_optimize(BusClientPolicy *policy) {
  DBusList *link;
  _dbus_verbose("Optimizing policy with %d rules\n", _dbus_list_get_length(&policy->rules));
  link = _dbus_list_get_first_link(&policy->rules);
  while(link != NULL) {
      BusPolicyRule *rule;
      DBusList *next;
      dbus_bool_t remove_preceding;
      next = _dbus_list_get_next_link(&policy->rules, link);
      rule = link->data;
      remove_preceding = FALSE;
      _dbus_assert(rule != NULL);
      switch(rule->type) {
          case BUS_POLICY_RULE_SEND:
              remove_preceding = rule->d.send.message_type == DBUS_MESSAGE_TYPE_INVALID && rule->d.send.path == NULL && rule->d.send.interface == NULL &&
                                 rule->d.send.member == NULL && rule->d.send.error == NULL && rule->d.send.destination == NULL;
              break;
          case BUS_POLICY_RULE_RECEIVE:
              remove_preceding = rule->d.receive.message_type == DBUS_MESSAGE_TYPE_INVALID && rule->d.receive.path == NULL && rule->d.receive.interface == NULL &&
                                 rule->d.receive.member == NULL && rule->d.receive.error == NULL && rule->d.receive.origin == NULL;
              break;
          case BUS_POLICY_RULE_OWN: remove_preceding = rule->d.own.service_name == NULL; break;
          default: _dbus_assert_not_reached("invalid rule");
      }
      if (remove_preceding) remove_rules_by_type_up_to(policy, rule->type, link);
      link = next;
  }
  _dbus_verbose("After optimization, policy has %d rules\n", _dbus_list_get_length(&policy->rules));
}
dbus_bool_t bus_client_policy_append_rule(BusClientPolicy *policy, BusPolicyRule *rule) {
  _dbus_verbose("Appending rule %p with type %d to policy %p\n", rule, rule->type, policy);
  if (!_dbus_list_append(&policy->rules, rule)) return FALSE;
  bus_policy_rule_ref(rule);
  return TRUE;
}
dbus_bool_t bus_client_policy_check_can_send(BusClientPolicy *policy, BusRegistry *registry, dbus_bool_t requested_reply, DBusConnection *receiver,
                                             DBusMessage *message, dbus_int32_t *toggles, dbus_bool_t *log) {
  DBusList *link;
  dbus_bool_t allowed;
  _dbus_verbose("  (policy) checking send rules\n");
  *toggles = 0;
  allowed = FALSE;
  link = _dbus_list_get_first_link(&policy->rules);
  while(link != NULL) {
      BusPolicyRule *rule = link->data;
      link = _dbus_list_get_next_link(&policy->rules, link);
      if (rule->type != BUS_POLICY_RULE_SEND) {
          _dbus_verbose("  (policy) skipping non-send rule\n");
          continue;
      }
      if (rule->d.send.message_type != DBUS_MESSAGE_TYPE_INVALID) {
          if (dbus_message_get_type(message) != rule->d.send.message_type) {
              _dbus_verbose("  (policy) skipping rule for different message type\n");
              continue;
          }
      }
      if (dbus_message_get_reply_serial(message) != 0) {
          if (!requested_reply && rule->allow && rule->d.send.requested_reply && !rule->d.send.eavesdrop) {
              _dbus_verbose("  (policy) skipping allow rule since it only applies to requested replies and does not allow eavesdropping\n");
              continue;
          }
          if (requested_reply && !rule->allow && !rule->d.send.requested_reply) {
              _dbus_verbose("  (policy) skipping deny rule since it only applies to unrequested replies\n");
              continue;
          }
      }
      if (rule->d.send.path != NULL) {
          if (dbus_message_get_path(message) != NULL && strcmp(dbus_message_get_path(message), rule->d.send.path) != 0) {
              _dbus_verbose("  (policy) skipping rule for different path\n");
              continue;
          }
      }
      if (rule->d.send.interface != NULL) {
          dbus_bool_t no_interface;
          no_interface = dbus_message_get_interface(message) == NULL;
          if ((no_interface && rule->allow) || (!no_interface && strcmp(dbus_message_get_interface(message), rule->d.send.interface) != 0)) {
              _dbus_verbose("  (policy) skipping rule for different interface\n");
              continue;
          }
      }
      if (rule->d.send.member != NULL) {
          if (dbus_message_get_member(message) != NULL && strcmp(dbus_message_get_member(message), rule->d.send.member) != 0) {
              _dbus_verbose("  (policy) skipping rule for different member\n");
              continue;
          }
      }
      if (rule->d.send.error != NULL) {
          if (dbus_message_get_error_name(message) != NULL && strcmp(dbus_message_get_error_name(message), rule->d.send.error) != 0) {
              _dbus_verbose("  (policy) skipping rule for different error name\n");
              continue;
          }
      }
      if (rule->d.send.broadcast != BUS_POLICY_TRISTATE_ANY) {
          if (dbus_message_get_destination(message) == NULL && dbus_message_get_type(message) == DBUS_MESSAGE_TYPE_SIGNAL) {
              if (rule->d.send.broadcast == BUS_POLICY_TRISTATE_FALSE) {
                  _dbus_verbose("  (policy) skipping rule because message is a broadcast\n");
                  continue;
              }
          } else if (rule->d.send.broadcast == BUS_POLICY_TRISTATE_TRUE) {
              _dbus_verbose("  (policy) skipping rule because message is not a broadcast\n");
              continue;
          }
      }
      if (rule->d.send.destination != NULL) {
          if (receiver == NULL) {
              if (!dbus_message_has_destination(message, rule->d.send.destination)) {
                  _dbus_verbose("  (policy) skipping rule because message dest is not %s\n", rule->d.send.destination);
                  continue;
              }
          } else {
              DBusString str;
              BusService *service;
              _dbus_string_init_const(&str, rule->d.send.destination);
              service = bus_registry_lookup(registry, &str);
              if (service == NULL) {
                  _dbus_verbose("  (policy) skipping rule because dest %s doesn't exist\n", rule->d.send.destination);
                  continue;
              }
              if (!bus_service_has_owner(service, receiver)) {
                  _dbus_verbose("  (policy) skipping rule because dest %s isn't owned by receiver\n", rule->d.send.destination);
                  continue;
              }
          }
      }
      if (rule->d.send.min_fds > 0 || rule->d.send.max_fds < DBUS_MAXIMUM_MESSAGE_UNIX_FDS) {
          unsigned int n_fds = _dbus_message_get_n_unix_fds(message);
          if (n_fds < rule->d.send.min_fds || n_fds > rule->d.send.max_fds) {
              _dbus_verbose("  (policy) skipping rule because message has %u fds and that is outside range [%u,%u]", n_fds, rule->d.send.min_fds,
                            rule->d.send.max_fds);
              continue;
          }
      }
      allowed = rule->allow;
      *log = rule->d.send.log;
      (*toggles)++;
      _dbus_verbose("  (policy) used rule, allow now = %d\n", allowed);
  }
  return allowed;
}
dbus_bool_t bus_client_policy_check_can_receive(BusClientPolicy *policy, BusRegistry *registry, dbus_bool_t requested_reply, DBusConnection *sender,
                                                DBusConnection *addressed_recipient, DBusConnection *proposed_recipient, DBusMessage *message, dbus_int32_t *toggles) {
  DBusList *link;
  dbus_bool_t allowed;
  dbus_bool_t eavesdropping;
  eavesdropping = addressed_recipient != proposed_recipient && dbus_message_get_destination(message) != NULL;
  _dbus_verbose("  (policy) checking receive rules, eavesdropping = %d\n", eavesdropping);
  *toggles = 0;
  allowed = FALSE;
  link = _dbus_list_get_first_link(&policy->rules);
  while(link != NULL) {
      BusPolicyRule *rule = link->data;
      link = _dbus_list_get_next_link(&policy->rules, link);
      if (rule->type != BUS_POLICY_RULE_RECEIVE) {
          _dbus_verbose("  (policy) skipping non-receive rule\n");
          continue;
      }
      if (rule->d.receive.message_type != DBUS_MESSAGE_TYPE_INVALID) {
          if (dbus_message_get_type(message) != rule->d.receive.message_type) {
              _dbus_verbose("  (policy) skipping rule for different message type\n");
              continue;
          }
      }
      if (eavesdropping && rule->allow && !rule->d.receive.eavesdrop) {
          _dbus_verbose("  (policy) skipping allow rule since it doesn't apply to eavesdropping\n");
          continue;
      }
      if (!eavesdropping && !rule->allow && rule->d.receive.eavesdrop) {
          _dbus_verbose("  (policy) skipping deny rule since it only applies to eavesdropping\n");
          continue;
      }
      if (dbus_message_get_reply_serial(message) != 0) {
          if (!requested_reply && rule->allow && rule->d.receive.requested_reply && !rule->d.receive.eavesdrop) {
              _dbus_verbose("  (policy) skipping allow rule since it only applies to requested replies and does not allow eavesdropping\n");
              continue;
          }
          if (requested_reply && !rule->allow && !rule->d.receive.requested_reply) {
              _dbus_verbose("  (policy) skipping deny rule since it only applies to unrequested replies\n");
              continue;
          }
      }
      if (rule->d.receive.path != NULL) {
          if (dbus_message_get_path(message) != NULL && strcmp(dbus_message_get_path(message), rule->d.receive.path) != 0) {
              _dbus_verbose("  (policy) skipping rule for different path\n");
              continue;
          }
      }
      if (rule->d.receive.interface != NULL) {
          dbus_bool_t no_interface;
          no_interface = dbus_message_get_interface(message) == NULL;
          if ((no_interface && rule->allow) || (!no_interface && strcmp(dbus_message_get_interface(message), rule->d.receive.interface) != 0)) {
              _dbus_verbose("  (policy) skipping rule for different interface\n");
              continue;
          }
      }
      if (rule->d.receive.member != NULL) {
          if (dbus_message_get_member(message) != NULL && strcmp(dbus_message_get_member(message), rule->d.receive.member) != 0) {
              _dbus_verbose("  (policy) skipping rule for different member\n");
              continue;
          }
      }
      if (rule->d.receive.error != NULL) {
          if (dbus_message_get_error_name(message) != NULL && strcmp(dbus_message_get_error_name(message), rule->d.receive.error) != 0) {
              _dbus_verbose("  (policy) skipping rule for different error name\n");
              continue;
          }
      }
      if (rule->d.receive.origin != NULL) {
          if (sender == NULL) {
              if (!dbus_message_has_sender(message, rule->d.receive.origin)) {
                  _dbus_verbose("  (policy) skipping rule because message sender is not %s\n", rule->d.receive.origin);
                  continue;
              }
          } else {
              BusService *service;
              DBusString str;
              _dbus_string_init_const(&str, rule->d.receive.origin);
              service = bus_registry_lookup(registry, &str);
              if (service == NULL) {
                  _dbus_verbose("  (policy) skipping rule because origin %s doesn't exist\n", rule->d.receive.origin);
                  continue;
              }
              if (!bus_service_has_owner(service, sender)) {
                  _dbus_verbose("  (policy) skipping rule because origin %s isn't owned by sender\n", rule->d.receive.origin);
                  continue;
              }
          }
      }
      if (rule->d.receive.min_fds > 0 || rule->d.receive.max_fds < DBUS_MAXIMUM_MESSAGE_UNIX_FDS) {
          unsigned int n_fds = _dbus_message_get_n_unix_fds(message);
          if (n_fds < rule->d.receive.min_fds || n_fds > rule->d.receive.max_fds) {
              _dbus_verbose("  (policy) skipping rule because message has %u fds and that is outside range [%u,%u]", n_fds, rule->d.receive.min_fds,
                            rule->d.receive.max_fds);
              continue;
          }
      }
      allowed = rule->allow;
      (*toggles)++;
      _dbus_verbose("  (policy) used rule, allow now = %d\n", allowed);
  }
  return allowed;
}
static dbus_bool_t bus_rules_check_can_own(DBusList *rules, const DBusString *service_name) {
  DBusList *link;
  dbus_bool_t allowed;
  allowed = FALSE;
  link = _dbus_list_get_first_link(&rules);
  while(link != NULL) {
      BusPolicyRule *rule = link->data;
      link = _dbus_list_get_next_link(&rules, link);
      if (rule->type != BUS_POLICY_RULE_OWN) continue;
      if (!rule->d.own.prefix && rule->d.own.service_name != NULL) {
          if (!_dbus_string_equal_c_str(service_name, rule->d.own.service_name))
            continue;
      } else if (rule->d.own.prefix) {
          const char *data;
          char next_char;
          if (!_dbus_string_starts_with_c_str(service_name, rule->d.own.service_name)) continue;
          data = _dbus_string_get_const_data(service_name);
          next_char = data[strlen(rule->d.own.service_name)];
          if (next_char != '\0' && next_char != '.') continue;
      }
      allowed = rule->allow;
  }
  return allowed;
}
dbus_bool_t bus_client_policy_check_can_own(BusClientPolicy *policy, const DBusString *service_name) {
  return bus_rules_check_can_own(policy->rules, service_name);
}
#ifdef DBUS_ENABLE_EMBEDDED_TESTS
dbus_bool_t bus_policy_check_can_own(BusPolicy *policy, const DBusString *service_name) {
  return bus_rules_check_can_own(policy->default_rules, service_name);
}
#endif