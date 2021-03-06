#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "dbus-object-tree.h"
#include "dbus-connection-internal.h"
#include "dbus-internals.h"
#include "dbus-hash.h"
#include "dbus-protocol.h"
#include "dbus-string.h"
#include "dbus-test-tap.h"

typedef struct DBusObjectSubtree DBusObjectSubtree;
static DBusObjectSubtree* _dbus_object_subtree_new(const char *name, const DBusObjectPathVTable *vtable, void *user_data);
static DBusObjectSubtree* _dbus_object_subtree_ref(DBusObjectSubtree *subtree);
static void _dbus_object_subtree_unref(DBusObjectSubtree *subtree);
struct DBusObjectTree {
  int refcount;
  DBusConnection *connection;
  DBusObjectSubtree *root;
};
struct DBusObjectSubtree {
  DBusAtomic refcount;
  DBusObjectSubtree *parent;
  DBusObjectPathUnregisterFunction unregister_function;
  DBusObjectPathMessageFunction message_function;
  void *user_data;
  DBusObjectSubtree **subtrees;
  int n_subtrees;
  int max_subtrees;
  unsigned int invoke_as_fallback : 1;
  char name[1];
};
DBusObjectTree* _dbus_object_tree_new(DBusConnection *connection) {
  DBusObjectTree *tree;
  tree = dbus_new0(DBusObjectTree, 1);
  if (tree == NULL) goto oom;
  tree->refcount = 1;
  tree->connection = connection;
  tree->root = _dbus_object_subtree_new("/", NULL, NULL);
  if (tree->root == NULL) goto oom;
  tree->root->invoke_as_fallback = TRUE;
  return tree;
oom:
  if (tree) dbus_free(tree);
  return NULL;
}
DBusObjectTree *_dbus_object_tree_ref(DBusObjectTree *tree) {
  _dbus_assert(tree->refcount > 0);
  tree->refcount += 1;
  return tree;
}
void _dbus_object_tree_unref(DBusObjectTree *tree) {
  _dbus_assert(tree->refcount > 0);
  tree->refcount -= 1;
  if (tree->refcount == 0) {
      _dbus_object_tree_free_all_unlocked(tree);
      dbus_free(tree);
  }
}
#define VERBOSE_FIND 0
static DBusObjectSubtree* find_subtree_recurse(DBusObjectSubtree *subtree, const char **path, dbus_bool_t create_if_not_found, int *index_in_parent,
                                               dbus_bool_t *exact_match) {
  int i, j;
  dbus_bool_t return_deepest_match;
  return_deepest_match = exact_match != NULL;
  _dbus_assert(!(return_deepest_match && create_if_not_found));
  if (path[0] == NULL) {
  #if VERBOSE_FIND
      _dbus_verbose("  path exhausted, returning %s\n", subtree->name);
  #endif
      if (exact_match != NULL) *exact_match = TRUE;
      return subtree;
  }
#if VERBOSE_FIND
  _dbus_verbose("  searching children of %s for %s\n", subtree->name, path[0]);
#endif
  i = 0;
  j = subtree->n_subtrees;
  while(i < j) {
      int k, v;
      k = (i + j) / 2;
      v = strcmp(path[0], subtree->subtrees[k]->name);
  #if VERBOSE_FIND
      _dbus_verbose("  %s cmp %s = %d\n", path[0], subtree->subtrees[k]->name, v);
  #endif
      if (v == 0) {
          if (index_in_parent) {
          #if VERBOSE_FIND
              _dbus_verbose("  storing parent index %d\n", k);
          #endif
              *index_in_parent = k;
          }
          if (return_deepest_match) {
              DBusObjectSubtree *next;
              next = find_subtree_recurse(subtree->subtrees[k],&path[1], create_if_not_found, index_in_parent, exact_match);
              if (next == NULL && subtree->invoke_as_fallback) {
              #if VERBOSE_FIND
                  _dbus_verbose("  no deeper match found, returning %s\n", subtree->name);
              #endif
                  if (exact_match != NULL) *exact_match = FALSE;
                  return subtree;
              } else return next;
          } else return find_subtree_recurse(subtree->subtrees[k],&path[1], create_if_not_found, index_in_parent, exact_match);
      } else if (v < 0) j = k;
      else i = k + 1;
  }
#if VERBOSE_FIND
  _dbus_verbose("  no match found, current tree %s, create_if_not_found = %d\n", subtree->name, create_if_not_found);
#endif
  if (create_if_not_found) {
      DBusObjectSubtree* child;
      int child_pos, new_n_subtrees;
  #if VERBOSE_FIND
      _dbus_verbose("  creating subtree %s\n", path[0]);
  #endif
      child = _dbus_object_subtree_new(path[0],NULL, NULL);
      if (child == NULL) return NULL;
      new_n_subtrees = subtree->n_subtrees + 1;
      if (new_n_subtrees > subtree->max_subtrees) {
          int new_max_subtrees;
          DBusObjectSubtree **new_subtrees;
          new_max_subtrees = subtree->max_subtrees == 0 ? 1 : 2 * subtree->max_subtrees;
          new_subtrees = dbus_realloc(subtree->subtrees,new_max_subtrees * sizeof(DBusObjectSubtree*));
          if (new_subtrees == NULL) {
              _dbus_object_subtree_unref(child);
              return NULL;
          }
          subtree->subtrees = new_subtrees;
          subtree->max_subtrees = new_max_subtrees;
      }
      child_pos = i;
      _dbus_assert(child_pos < new_n_subtrees && new_n_subtrees <= subtree->max_subtrees);
      if (child_pos + 1 < new_n_subtrees)
          memmove(&subtree->subtrees[child_pos+1], &subtree->subtrees[child_pos],(new_n_subtrees - child_pos - 1) * sizeof(subtree->subtrees[0]));
      subtree->subtrees[child_pos] = child;
      if (index_in_parent) *index_in_parent = child_pos;
      subtree->n_subtrees = new_n_subtrees;
      child->parent = subtree;
      return find_subtree_recurse(child,&path[1], create_if_not_found, index_in_parent, exact_match);
  } else {
      if (exact_match != NULL) *exact_match = FALSE;
      return (return_deepest_match && subtree->invoke_as_fallback) ? subtree : NULL;
  }
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
static DBusObjectSubtree* find_subtree(DBusObjectTree *tree, const char **path, int *index_in_parent) {
  DBusObjectSubtree *subtree;
#if VERBOSE_FIND
  _dbus_verbose("Looking for exact registered subtree\n");
#endif
  subtree = find_subtree_recurse(tree->root, path, FALSE, index_in_parent, NULL);
  if (subtree && subtree->message_function == NULL) return NULL;
  else return subtree;
}
#endif
static DBusObjectSubtree* lookup_subtree(DBusObjectTree *tree, const char **path) {
#if VERBOSE_FIND
  _dbus_verbose("Looking for subtree\n");
#endif
  return find_subtree_recurse(tree->root, path, FALSE, NULL, NULL);
}
static DBusObjectSubtree* find_handler(DBusObjectTree *tree, const char **path, dbus_bool_t *exact_match) {
#if VERBOSE_FIND
  _dbus_verbose("Looking for deepest handler\n");
#endif
  _dbus_assert(exact_match != NULL);
  *exact_match = FALSE;
  return find_subtree_recurse(tree->root, path, FALSE, NULL, exact_match);
}
static DBusObjectSubtree* ensure_subtree(DBusObjectTree *tree, const char **path) {
#if VERBOSE_FIND
  _dbus_verbose("Ensuring subtree\n");
#endif
  return find_subtree_recurse(tree->root, path, TRUE, NULL, NULL);
}
static char *flatten_path(const char **path);
dbus_bool_t _dbus_object_tree_register(DBusObjectTree *tree, dbus_bool_t fallback, const char **path, const DBusObjectPathVTable *vtable, void *user_data,
                                       DBusError *error) {
  DBusObjectSubtree *subtree;
  _dbus_assert(tree != NULL);
  _dbus_assert(vtable->message_function != NULL);
  _dbus_assert(path != NULL);
  subtree = ensure_subtree(tree, path);
  if (subtree == NULL) {
      _DBUS_SET_OOM(error);
      return FALSE;
  }
  if (subtree->message_function != NULL) {
      if (error != NULL) {
          char *complete_path = flatten_path(path);
          dbus_set_error(error, DBUS_ERROR_OBJECT_PATH_IN_USE, "A handler is already registered for %s", complete_path ? complete_path :
                         "(cannot represent path: out of memory!)");
          dbus_free(complete_path);
      }
      return FALSE;
  }
  subtree->message_function = vtable->message_function;
  subtree->unregister_function = vtable->unregister_function;
  subtree->user_data = user_data;
  subtree->invoke_as_fallback = fallback != FALSE;
  return TRUE;
}
static dbus_bool_t unregister_subtree(DBusObjectSubtree *subtree, DBusObjectPathUnregisterFunction *unregister_function_out, void **user_data_out) {
  _dbus_assert(subtree != NULL);
  _dbus_assert(unregister_function_out != NULL);
  _dbus_assert(user_data_out != NULL);
  if (subtree->message_function != NULL) {
      subtree->message_function = NULL;
      *unregister_function_out = subtree->unregister_function;
      *user_data_out = subtree->user_data;
      subtree->unregister_function = NULL;
      subtree->user_data = NULL;
      return TRUE;
  } else {
      _dbus_assert(subtree->parent == NULL || subtree->n_subtrees > 0);
      return FALSE;
    }
}
static dbus_bool_t attempt_child_removal(DBusObjectSubtree *parent, int child_index) {
  DBusObjectSubtree* candidate;
  _dbus_assert(parent != NULL);
  _dbus_assert(child_index >= 0 && child_index < parent->n_subtrees);
  candidate = parent->subtrees[child_index];
  _dbus_assert(candidate != NULL);
  if (candidate->n_subtrees == 0 && candidate->message_function == NULL) {
      memmove(&parent->subtrees[child_index],&parent->subtrees[child_index + 1],(parent->n_subtrees - child_index - 1) * sizeof(parent->subtrees[0]));
      parent->n_subtrees -= 1;
      candidate->parent = NULL;
      _dbus_object_subtree_unref(candidate);
      return TRUE;
  }
  return FALSE;
}
static dbus_bool_t unregister_and_free_path_recurse(DBusObjectSubtree *subtree, const char **path, dbus_bool_t *continue_removal_attempts,
                                                    DBusObjectPathUnregisterFunction *unregister_function_out, void **user_data_out) {
  int i, j;
  _dbus_assert(continue_removal_attempts != NULL);
  _dbus_assert(*continue_removal_attempts);
  _dbus_assert(unregister_function_out != NULL);
  _dbus_assert(user_data_out != NULL);
  if (path[0] == NULL) return unregister_subtree(subtree, unregister_function_out, user_data_out);
  i = 0;
  j = subtree->n_subtrees;
  while(i < j) {
      int k, v;
      k = (i + j) / 2;
      v = strcmp(path[0], subtree->subtrees[k]->name);
      if (v == 0) {
          dbus_bool_t freed;
          freed = unregister_and_free_path_recurse(subtree->subtrees[k],&path[1], continue_removal_attempts, unregister_function_out, user_data_out);
          if (freed && *continue_removal_attempts) *continue_removal_attempts = attempt_child_removal(subtree, k);
          return freed;
      } else if (v < 0) j = k;
      else i = k + 1;
  }
  return FALSE;
}
void _dbus_object_tree_unregister_and_unlock(DBusObjectTree *tree, const char **path) {
  dbus_bool_t found_subtree;
  dbus_bool_t continue_removal_attempts;
  DBusObjectPathUnregisterFunction unregister_function;
  void *user_data;
  DBusConnection *connection;
  _dbus_assert(tree != NULL);
  _dbus_assert(path != NULL);
  continue_removal_attempts = TRUE;
  unregister_function = NULL;
  user_data = NULL;
  found_subtree = unregister_and_free_path_recurse(tree->root, path, &continue_removal_attempts, &unregister_function, &user_data);
#ifdef DBUS_DISABLE_CHECKS
  if (found_subtree == FALSE) {
      _dbus_warn("Attempted to unregister path (path[0] = %s path[1] = %s) which isn't registered", path[0] ? path[0] : "null", (path[0] && path[1]) ?
                 path[1] : "null");
      goto unlock;    
  }
#else
  _dbus_assert(found_subtree == TRUE);
#endif
unlock:
  connection = tree->connection;
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (connection)
#endif
  {
      _dbus_connection_ref_unlocked(connection);
      _dbus_verbose("unlock\n");
      _dbus_connection_unlock(connection);
  }
  if (unregister_function) (*unregister_function)(connection, user_data);
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (connection)
#endif
    dbus_connection_unref(connection);
}
static void free_subtree_recurse(DBusConnection *connection, DBusObjectSubtree *subtree) {
  while(subtree->n_subtrees > 0) {
      DBusObjectSubtree *child;
      child = subtree->subtrees[subtree->n_subtrees - 1];
      subtree->subtrees[subtree->n_subtrees - 1] = NULL;
      subtree->n_subtrees -= 1;
      child->parent = NULL;
      free_subtree_recurse(connection, child);
  }
  if (subtree->unregister_function) (*subtree->unregister_function)(connection, subtree->user_data);
  subtree->message_function = NULL;
  subtree->unregister_function = NULL;
  subtree->user_data = NULL;
  _dbus_object_subtree_unref(subtree);
}
void _dbus_object_tree_free_all_unlocked(DBusObjectTree *tree) {
  if (tree->root) free_subtree_recurse(tree->connection, tree->root);
  tree->root = NULL;
}
static dbus_bool_t _dbus_object_tree_list_registered_unlocked(DBusObjectTree *tree, const char **parent_path, char ***child_entries) {
  DBusObjectSubtree *subtree;
  char **retval;
  _dbus_assert(parent_path != NULL);
  _dbus_assert(child_entries != NULL);
  *child_entries = NULL;
  subtree = lookup_subtree(tree, parent_path);
  if (subtree == NULL) retval = dbus_new0(char*, 1);
  else {
      int i;
      retval = dbus_new0(char*, subtree->n_subtrees + 1);
      if (retval == NULL) goto out;
      i = 0;
      while(i < subtree->n_subtrees) {
          retval[i] = _dbus_strdup(subtree->subtrees[i]->name);
          if (retval[i] == NULL) {
              dbus_free_string_array(retval);
              retval = NULL;
              goto out;
          }
          ++i;
      }
  }
out:
  *child_entries = retval;
  return retval != NULL;
}
static DBusHandlerResult handle_default_introspect_and_unlock(DBusObjectTree *tree, DBusMessage *message, const char **path) {
  DBusString xml;
  DBusHandlerResult result;
  char **children;
  int i;
  DBusMessage *reply;
  DBusMessageIter iter;
  const char *v_STRING;
  dbus_bool_t already_unlocked;
  already_unlocked = FALSE;
  _dbus_verbose(" considering default Introspect() handler...\n");
  reply = NULL;
  if (!dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE,"Introspect")) {
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
      if (tree->connection)
  #endif
      {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  _dbus_verbose(" using default Introspect() handler!\n");
  if (!_dbus_string_init(&xml)) {
  #ifndef DBUS_ENABLE_EMBEDDED_TESTS
      if (tree->connection)
  #endif
      {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
      return DBUS_HANDLER_RESULT_NEED_MEMORY;
  }
  result = DBUS_HANDLER_RESULT_NEED_MEMORY;
  children = NULL;
  if (!_dbus_object_tree_list_registered_unlocked(tree, path, &children)) goto out;
  if (!_dbus_string_append(&xml, DBUS_INTROSPECT_1_0_XML_DOCTYPE_DECL_NODE)) goto out;
  if (!_dbus_string_append(&xml, "<node>\n")) goto out;
  i = 0;
  while(children[i] != NULL) {
      if (!_dbus_string_append_printf(&xml, "  <node name=\"%s\"/>\n", children[i])) goto out;
      ++i;
  }
  if (!_dbus_string_append(&xml, "</node>\n")) goto out;
  reply = dbus_message_new_method_return(message);
  if (reply == NULL) goto out;
  dbus_message_iter_init_append(reply, &iter);
  v_STRING = _dbus_string_get_const_data(&xml);
  if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &v_STRING)) goto out;
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (tree->connection)
#endif
  {
      already_unlocked = TRUE;
      if (!_dbus_connection_send_and_unlock(tree->connection, reply, NULL)) goto out;
  }
  result = DBUS_HANDLER_RESULT_HANDLED;
out:
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (tree->connection)
#endif
  {
      if (!already_unlocked) {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
  }
  _dbus_string_free(&xml);
  dbus_free_string_array(children);
  if (reply) dbus_message_unref(reply);
  return result;
}
DBusHandlerResult _dbus_object_tree_dispatch_and_unlock(DBusObjectTree *tree, DBusMessage *message, dbus_bool_t *found_object) {
  char **path;
  dbus_bool_t exact_match;
  DBusList *list;
  DBusList *link;
  DBusHandlerResult result;
  DBusObjectSubtree *subtree;
#if 0
  _dbus_verbose("Dispatch of message by object path\n");
#endif
  path = NULL;
  if (!dbus_message_get_path_decomposed(message, &path)) {
  #ifdef DBUS_ENABLE_EMBEDDED_TESTS
      if (tree->connection)
  #endif
      {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
      _dbus_verbose("No memory to get decomposed path\n");
      return DBUS_HANDLER_RESULT_NEED_MEMORY;
  }
  if (path == NULL) {
  #ifdef DBUS_ENABLE_EMBEDDED_TESTS
      if (tree->connection)
  #endif
      {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
      _dbus_verbose("No path field in message\n");
      return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  }
  subtree = find_handler(tree, (const char**)path, &exact_match);
  if (found_object) *found_object = !!subtree;
  list = NULL;
  while(subtree != NULL) {
      if (subtree->message_function != NULL && (exact_match || subtree->invoke_as_fallback)) {
          _dbus_object_subtree_ref(subtree);
          if (!_dbus_list_append(&list, subtree)) {
              result = DBUS_HANDLER_RESULT_NEED_MEMORY;
              _dbus_object_subtree_unref(subtree);
              goto free_and_return;
          }
      }
      exact_match = FALSE;
      subtree = subtree->parent;
  }
  _dbus_verbose("%d handlers in the path tree for this message\n", _dbus_list_get_length(&list));
  result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
  link = _dbus_list_get_first_link(&list);
  while(link != NULL) {
      DBusList *next = _dbus_list_get_next_link(&list, link);
      subtree = link->data;
      if (subtree->message_function) {
          DBusObjectPathMessageFunction message_function;
          void *user_data;
          message_function = subtree->message_function;
          user_data = subtree->user_data;
      #if 0
          _dbus_verbose("  (invoking a handler)\n");
      #endif
      #ifndef DBUS_ENABLE_EMBEDDED_TESTS
          if (tree->connection)
      #endif
          {
              _dbus_verbose("unlock\n");
              _dbus_connection_unlock(tree->connection);
          }
          result = (*message_function)(tree->connection, message, user_data);
      #ifdef DBUS_ENABLE_EMBEDDED_TESTS
          if (tree->connection)
      #endif
          _dbus_connection_lock(tree->connection);
          if (result != DBUS_HANDLER_RESULT_NOT_YET_HANDLED) goto free_and_return;
      }
      link = next;
  }
free_and_return:
  if (result == DBUS_HANDLER_RESULT_NOT_YET_HANDLED) result = handle_default_introspect_and_unlock(tree, message, (const char**)path);
  else {
  #ifdef DBUS_ENABLE_EMBEDDED_TESTS
      if (tree->connection)
  #endif
      {
          _dbus_verbose("unlock\n");
          _dbus_connection_unlock(tree->connection);
      }
  }
  while(list != NULL) {
      link = _dbus_list_get_first_link(&list);
      _dbus_object_subtree_unref(link->data);
      _dbus_list_remove_link(&list, link);
  }
  dbus_free_string_array(path);
  return result;
}
void* _dbus_object_tree_get_user_data_unlocked(DBusObjectTree *tree, const char **path) {
  dbus_bool_t exact_match;
  DBusObjectSubtree *subtree;
  _dbus_assert(tree != NULL);
  _dbus_assert(path != NULL);
  subtree = find_handler(tree, (const char**)path, &exact_match);
  if ((subtree == NULL) || !exact_match) {
      _dbus_verbose("No object at specified path found\n");
      return NULL;
  }
  return subtree->user_data;
}
static DBusObjectSubtree* allocate_subtree_object(const char *name) {
  int len;
  DBusObjectSubtree *subtree;
  const size_t front_padding = _DBUS_STRUCT_OFFSET(DBusObjectSubtree, name);
  _dbus_assert(name != NULL);
  len = strlen(name);
  subtree = dbus_malloc0(MAX(front_padding + (len + 1), sizeof(DBusObjectSubtree)));
  if (subtree == NULL) return NULL;
  memcpy(subtree->name, name, len + 1);
  return subtree;
}
static DBusObjectSubtree* _dbus_object_subtree_new(const char *name, const DBusObjectPathVTable *vtable, void *user_data) {
  DBusObjectSubtree *subtree;
  subtree = allocate_subtree_object(name);
  if (subtree == NULL) goto oom;
  _dbus_assert(name != NULL);
  subtree->parent = NULL;
  if (vtable) {
      subtree->message_function = vtable->message_function;
      subtree->unregister_function = vtable->unregister_function;
  } else {
      subtree->message_function = NULL;
      subtree->unregister_function = NULL;
  }
  subtree->user_data = user_data;
  _dbus_atomic_inc(&subtree->refcount);
  subtree->subtrees = NULL;
  subtree->n_subtrees = 0;
  subtree->max_subtrees = 0;
  subtree->invoke_as_fallback = FALSE;
  return subtree;
oom:
  return NULL;
}
static DBusObjectSubtree *_dbus_object_subtree_ref(DBusObjectSubtree *subtree) {
#ifdef DBUS_DISABLE_ASSERT
  _dbus_atomic_inc(&subtree->refcount);
#else
  dbus_int32_t old_value;
  old_value = _dbus_atomic_inc(&subtree->refcount);
  _dbus_assert(old_value > 0);
#endif
  return subtree;
}
static void _dbus_object_subtree_unref(DBusObjectSubtree *subtree) {
  dbus_int32_t old_value;
  old_value = _dbus_atomic_dec(&subtree->refcount);
  _dbus_assert(old_value > 0);
  if (old_value == 1) {
      _dbus_assert(subtree->unregister_function == NULL);
      _dbus_assert(subtree->message_function == NULL);
      dbus_free(subtree->subtrees);
      dbus_free(subtree);
  }
}
dbus_bool_t _dbus_object_tree_list_registered_and_unlock(DBusObjectTree *tree, const char **parent_path, char ***child_entries) {
  dbus_bool_t result;
  result = _dbus_object_tree_list_registered_unlocked(tree, parent_path, child_entries);
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
  if (tree->connection)
#endif
  {
      _dbus_verbose("unlock\n");
      _dbus_connection_unlock(tree->connection);
  }
  return result;
}
#define VERBOSE_DECOMPOSE 0
dbus_bool_t _dbus_decompose_path(const char* data, int len, char ***path, int *path_len) {
  char **retval;
  int n_components;
  int i, j, comp;
  _dbus_assert(data != NULL);
  _dbus_assert(path != NULL);
#if VERBOSE_DECOMPOSE
  _dbus_verbose("Decomposing path \"%s\"\n", data);
#endif
  n_components = 0;
  if (len > 1) {
      i = 0;
      while(i < len) {
          _dbus_assert(data[i] != '\0');
          if (data[i] == '/') n_components += 1;
          ++i;
      }
  }
  retval = dbus_new0(char*, n_components + 1);
  if (retval == NULL) return FALSE;
  comp = 0;
  if (n_components == 0) i = 1;
  else i = 0;
  while(comp < n_components) {
      _dbus_assert(i < len);
      if (data[i] == '/') ++i;
      j = i;
      while(j < len && data[j] != '/') ++j;
      _dbus_assert(i < j);
      _dbus_assert(data[i] != '/');
      _dbus_assert(j == len || data[j] == '/');
  #if VERBOSE_DECOMPOSE
      _dbus_verbose("  (component in [%d,%d))\n", i, j);
  #endif
      retval[comp] = _dbus_memdup(&data[i], j - i + 1);
      if (retval[comp] == NULL) {
          dbus_free_string_array(retval);
          return FALSE;
      }
      retval[comp][j-i] = '\0';
  #if VERBOSE_DECOMPOSE
      _dbus_verbose("  (component %d = \"%s\")\n", comp, retval[comp]);
  #endif
      ++comp;
      i = j;
  }
  _dbus_assert(i == len);
  *path = retval;
  if (path_len) *path_len = n_components;
  return TRUE;
}
static char* flatten_path(const char **path) {
  DBusString str;
  char *s;
  if (!_dbus_string_init(&str)) return NULL;
  if (path[0] == NULL) {
      if (!_dbus_string_append_byte(&str, '/')) goto nomem;
  } else {
      int i;
      i = 0;
      while (path[i]) {
          if (!_dbus_string_append_byte(&str, '/')) goto nomem;
          if (!_dbus_string_append(&str, path[i])) goto nomem;
          ++i;
      }
  }
  if (!_dbus_string_steal_data(&str, &s)) goto nomem;
  _dbus_string_free(&str);
  return s;
nomem:
  _dbus_string_free(&str);
  return NULL;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include <stdio.h>
#include "dbus-test.h"

typedef enum {
  STR_EQUAL,
  STR_PREFIX,
  STR_DIFFERENT
} StrComparison;
static StrComparison path_contains(const char **container, const char **child) {
  int i;
  i = 0;
  while(child[i] != NULL) {
      int v;
      if (container[i] == NULL) return STR_PREFIX;
      _dbus_assert(container[i] != NULL);
      _dbus_assert(child[i] != NULL);
      v = strcmp(container[i], child[i]);
      if (v != 0) return STR_DIFFERENT;
      ++i;
  }
  if (container[i] == NULL) return STR_EQUAL;
  else return STR_DIFFERENT;
}
#if 0
static void spew_subtree_recurse(DBusObjectSubtree *subtree, int indent) {
  int i;
  i = 0;
  while(i < indent) {
      _dbus_verbose(" ");
      ++i;
  }
  _dbus_verbose("%s (%d children)\n", subtree->name, subtree->n_subtrees);
  i = 0;
  while(i < subtree->n_subtrees) {
      spew_subtree_recurse(subtree->subtrees[i], indent + 2);
      ++i;
  }
}
static void spew_tree(DBusObjectTree *tree) {
  spew_subtree_recurse(tree->root, 0);
}
#endif
typedef struct {
  const char **path;
  dbus_bool_t handler_fallback;
  dbus_bool_t message_handled;
  dbus_bool_t handler_unregistered;
} TreeTestData;
static void test_unregister_function(DBusConnection *connection, void *user_data) {
  TreeTestData *ttd = user_data;
  ttd->handler_unregistered = TRUE;
}
static DBusHandlerResult test_message_function(DBusConnection *connection, DBusMessage *message, void *user_data) {
  TreeTestData *ttd = user_data;
  ttd->message_handled = TRUE;
  return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
static dbus_bool_t do_register(DBusObjectTree *tree, const char **path, dbus_bool_t fallback, int i, TreeTestData *tree_test_data) {
  DBusObjectPathVTable vtable = { test_unregister_function, test_message_function, NULL };
  tree_test_data[i].message_handled = FALSE;
  tree_test_data[i].handler_unregistered = FALSE;
  tree_test_data[i].handler_fallback = fallback;
  tree_test_data[i].path = path;
  if (!_dbus_object_tree_register(tree, fallback, path, &vtable, &tree_test_data[i],NULL)) return FALSE;
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked (tree, path) == &tree_test_data[i]);
  return TRUE;
}
static dbus_bool_t do_test_dispatch(DBusObjectTree *tree, const char **path, int i, TreeTestData *tree_test_data, int n_test_data) {
  DBusMessage *message;
  int j;
  DBusHandlerResult result;
  char *flat;
  message = NULL;
  flat = flatten_path(path);
  if (flat == NULL) goto oom;
  message = dbus_message_new_method_call(NULL, flat,"org.freedesktop.TestInterface", "Foo");
  dbus_free(flat);
  if (message == NULL) goto oom;
  j = 0;
  while(j < n_test_data) {
      tree_test_data[j].message_handled = FALSE;
      ++j;
  }
  result = _dbus_object_tree_dispatch_and_unlock(tree, message, NULL);
  if (result == DBUS_HANDLER_RESULT_NEED_MEMORY) goto oom;
  _dbus_assert(tree_test_data[i].message_handled);
  j = 0;
  while(j < n_test_data) {
      if (tree_test_data[j].message_handled) {
          if (tree_test_data[j].handler_fallback) { _dbus_assert(path_contains(tree_test_data[j].path, path) != STR_DIFFERENT); }
          else { _dbus_assert(path_contains(tree_test_data[j].path, path) == STR_EQUAL); }
      } else {
          if (tree_test_data[j].handler_fallback) { _dbus_assert(path_contains(tree_test_data[j].path, path) == STR_DIFFERENT); }
          else { _dbus_assert(path_contains(tree_test_data[j].path, path) != STR_EQUAL); }
	  }
      ++j;
  }
  dbus_message_unref(message);
  return TRUE;
oom:
  if (message) dbus_message_unref(message);
  return FALSE;
}
typedef struct {
  const char *path;
  const char *result[20];
} DecomposePathTest;
static DecomposePathTest decompose_tests[] = {
  { "/foo", { "foo", NULL } },
  { "/foo/bar", { "foo", "bar", NULL } },
  { "/", { NULL } },
  { "/a/b", { "a", "b", NULL } },
  { "/a/b/c", { "a", "b", "c", NULL } },
  { "/a/b/c/d", { "a", "b", "c", "d", NULL } },
  { "/foo/bar/q", { "foo", "bar", "q", NULL } },
  { "/foo/bar/this/is/longer", { "foo", "bar", "this", "is", "longer", NULL } }
};
static dbus_bool_t run_decompose_tests(void) {
  int i;
  i = 0;
  while(i < _DBUS_N_ELEMENTS(decompose_tests)) {
      char **result;
      int result_len;
      int expected_len;
      if (!_dbus_decompose_path(decompose_tests[i].path, strlen(decompose_tests[i].path), &result, &result_len)) return FALSE;
      expected_len = _dbus_string_array_length(decompose_tests[i].result);
      if (result_len != (int)_dbus_string_array_length((const char**)result) || expected_len != result_len || path_contains(decompose_tests[i].result,
          (const char**)result) != STR_EQUAL) {
          int real_len = _dbus_string_array_length((const char**)result);
          _dbus_warn("Expected decompose of %s to have len %d, returned %d, appears to have %d", decompose_tests[i].path, expected_len, result_len,
                     real_len);
          _dbus_warn("Decompose resulted in elements: { ");
          i = 0;
          while(i < real_len) {
              _dbus_warn("\"%s\"%s", result[i], (i + 1) == real_len ? "" : ", ");
              ++i;
          }
          _dbus_warn("}");
          _dbus_test_fatal("path decompose failed");
      }
      dbus_free_string_array(result);
      ++i;
  }
  return TRUE;
}
static DBusObjectSubtree* find_subtree_registered_or_unregistered(DBusObjectTree *tree, const char **path) {
#if VERBOSE_FIND
  _dbus_verbose("Looking for exact subtree, registered or unregistered\n");
#endif
  return find_subtree_recurse(tree->root, path, FALSE, NULL, NULL);
}
static dbus_bool_t object_tree_test_iteration(void *data, dbus_bool_t have_memory) {
  const char *path0[] = { NULL };
  const char *path1[] = { "foo", NULL };
  const char *path2[] = { "foo", "bar", NULL };
  const char *path3[] = { "foo", "bar", "baz", NULL };
  const char *path4[] = { "foo", "bar", "boo", NULL };
  const char *path5[] = { "blah", NULL };
  const char *path6[] = { "blah", "boof", NULL };
  const char *path7[] = { "blah", "boof", "this", "is", "really", "long", NULL };
  const char *path8[] = { "childless", NULL };
  const char *path9[] = { "blah", "a", NULL };
  const char *path10[] = { "blah", "b", NULL };
  const char *path11[] = { "blah", "c", NULL };
  const char *path12[] = { "blah", "a", "d", NULL };
  const char *path13[] = { "blah", "b", "d", NULL };
  const char *path14[] = { "blah", "c", "d", NULL };
  DBusObjectPathVTable test_vtable = { NULL, test_message_function, NULL };
  DBusObjectTree *tree;
  TreeTestData tree_test_data[9];
  int i;
  dbus_bool_t exact_match;
  if (!run_decompose_tests()) return TRUE;
  tree = NULL;
  tree = _dbus_object_tree_new(NULL);
  if (tree == NULL) goto out;
  if (!do_register(tree, path0, TRUE, 0, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  _dbus_assert(find_handler(tree, path0, &exact_match) && exact_match);
  _dbus_assert(find_handler(tree, path1, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path2, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path3, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path4, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path5, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path6, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path7, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path8, &exact_match) == tree->root && !exact_match);
  if (!do_register(tree, path1, TRUE, 1, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  _dbus_assert(find_handler(tree, path0, &exact_match) &&  exact_match);
  _dbus_assert(find_handler(tree, path1, &exact_match) &&  exact_match);
  _dbus_assert(find_handler(tree, path2, &exact_match) && !exact_match);
  _dbus_assert(find_handler(tree, path3, &exact_match) && !exact_match);
  _dbus_assert(find_handler(tree, path4, &exact_match) && !exact_match);
  _dbus_assert(find_handler(tree, path5, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path6, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path7, &exact_match) == tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path8, &exact_match) == tree->root && !exact_match);
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  if (!do_register(tree, path3, TRUE, 3, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  if (!do_register(tree, path4, TRUE, 4, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  if (!do_register(tree, path5, TRUE, 5, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  _dbus_assert(find_handler(tree, path0, &exact_match) == tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path1, &exact_match) != tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path2, &exact_match) != tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path3, &exact_match) != tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path4, &exact_match) != tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path5, &exact_match) != tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path6, &exact_match) != tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path7, &exact_match) != tree->root && !exact_match);
  _dbus_assert(find_handler(tree, path8, &exact_match) == tree->root && !exact_match);
  if (!do_register(tree, path6, TRUE, 6, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  if (!do_register(tree, path7, TRUE, 7, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  if (!do_register(tree, path8, TRUE, 8, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_assert(find_handler(tree, path0, &exact_match) == tree->root &&  exact_match);
  _dbus_assert(find_handler(tree, path1, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path2, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path3, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path4, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path5, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path6, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path7, &exact_match) != tree->root && exact_match);
  _dbus_assert(find_handler(tree, path8, &exact_match) != tree->root && exact_match);
  {
      const char *root[] = { NULL };
      char **child_entries;
      int nb;
      _dbus_object_tree_list_registered_unlocked(tree, path1, &child_entries);
      if (child_entries != NULL) {
          nb = _dbus_string_array_length((const char**)child_entries);
          _dbus_assert(nb == 1);
          dbus_free_string_array(child_entries);
      }
      _dbus_object_tree_list_registered_unlocked(tree, path2, &child_entries);
      if (child_entries != NULL) {
          nb = _dbus_string_array_length((const char**)child_entries);
          _dbus_assert(nb == 2);
          dbus_free_string_array(child_entries);
      }
      _dbus_object_tree_list_registered_unlocked(tree, path8, &child_entries);
      if (child_entries != NULL) {
          nb = _dbus_string_array_length((const char**)child_entries);
          _dbus_assert(nb == 0);
          dbus_free_string_array(child_entries);
      }
      _dbus_object_tree_list_registered_unlocked(tree, root, &child_entries);
      if (child_entries != NULL) {
          nb = _dbus_string_array_length((const char**)child_entries);
          _dbus_assert(nb == 3);
          dbus_free_string_array(child_entries);
      }
  }
  _dbus_object_tree_unref(tree);
  i = 0;
  while(i < (int) _DBUS_N_ELEMENTS(tree_test_data)) {
      _dbus_assert(tree_test_data[i].handler_unregistered);
      _dbus_assert(!tree_test_data[i].message_handled);
      ++i;
  }
  tree = _dbus_object_tree_new(NULL);
  if (tree == NULL) goto out;
  if (!do_register(tree, path0, TRUE, 0, tree_test_data)) goto out;
  if (!do_register(tree, path1, TRUE, 1, tree_test_data)) goto out;
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  if (!do_register(tree, path3, TRUE, 3, tree_test_data)) goto out;
  if (!do_register(tree, path4, TRUE, 4, tree_test_data)) goto out;
  if (!do_register(tree, path5, TRUE, 5, tree_test_data)) goto out;
  if (!do_register(tree, path6, TRUE, 6, tree_test_data)) goto out;
  if (!do_register(tree, path7, TRUE, 7, tree_test_data)) goto out;
  if (!do_register(tree, path8, TRUE, 8, tree_test_data)) goto out;
  _dbus_object_tree_unregister_and_unlock(tree, path0);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path0) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path1);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path1) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path2) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path3);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path3) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path4);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path4) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path5);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path5) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path6);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path6) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path7);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path7) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(find_subtree(tree, path8, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path8);
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path8) == NULL);
  _dbus_assert(!find_subtree(tree, path0, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree(tree, path5, NULL));
  _dbus_assert(!find_subtree(tree, path6, NULL));
  _dbus_assert(!find_subtree(tree, path7, NULL));
  _dbus_assert(!find_subtree(tree, path8, NULL));
  i = 0;
  while(i < (int) _DBUS_N_ELEMENTS(tree_test_data)) {
      _dbus_assert(tree_test_data[i].handler_unregistered);
      _dbus_assert(!tree_test_data[i].message_handled);
      ++i;
  }
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  _dbus_assert(!find_subtree (tree, path1, NULL));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
#if 0
  _dbus_object_tree_unregister_and_unlock(tree, path1);
#endif
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  if (!do_register(tree, path1, TRUE, 1, tree_test_data)) goto out;
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path1);
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(find_subtree(tree, path2, NULL));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_assert(!find_subtree(tree, path1, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  if (!_dbus_object_tree_register(tree, TRUE, path2, &test_vtable,NULL, NULL)) goto out;
  _dbus_assert(_dbus_object_tree_get_user_data_unlocked(tree, path2) == NULL);
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_assert(!find_subtree(tree, path2, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  if (!do_register(tree, path3, TRUE, 3, tree_test_data)) goto out;
  _dbus_object_tree_unregister_and_unlock(tree, path3);
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path3));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path0));
  if (!do_register(tree, path3, TRUE, 3, tree_test_data)) goto out;
  if (!do_register(tree, path4, TRUE, 4, tree_test_data)) goto out;
  _dbus_assert(find_subtree(tree, path3, NULL));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_object_tree_unregister_and_unlock (tree, path3);
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path3));
  _dbus_assert(find_subtree(tree, path4, NULL));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path4));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path1));
  _dbus_object_tree_unregister_and_unlock (tree, path4);
  _dbus_assert(!find_subtree(tree, path4, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path4));
  _dbus_assert(!find_subtree(tree, path3, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path3));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path2));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path1));
  if (!_dbus_object_tree_register(tree, TRUE, path12, &test_vtable,NULL, NULL)) goto out;
  _dbus_assert(find_subtree(tree, path12, NULL));
  if (!_dbus_object_tree_register(tree, TRUE, path13, &test_vtable,NULL,NULL)) goto out;
  _dbus_assert(find_subtree(tree, path13, NULL));
  if (!_dbus_object_tree_register(tree, TRUE, path14, &test_vtable,NULL,NULL)) goto out;
  _dbus_assert(find_subtree(tree, path14, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path12);
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path12));
  _dbus_assert(find_subtree(tree, path13, NULL));
  _dbus_assert(find_subtree(tree, path14, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path9));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path5));
  if (!_dbus_object_tree_register(tree, TRUE, path12, &test_vtable,NULL,NULL)) goto out;
  _dbus_assert(find_subtree(tree, path12, NULL));
  _dbus_object_tree_unregister_and_unlock(tree, path13);
  _dbus_assert(find_subtree(tree, path12, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path13));
  _dbus_assert(find_subtree(tree, path14, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path10));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path5));
  if (!_dbus_object_tree_register(tree, TRUE, path13, &test_vtable,NULL,NULL)) goto out;
  _dbus_assert(find_subtree(tree, path13, NULL));
  _dbus_object_tree_unregister_and_unlock (tree, path14);
  _dbus_assert(find_subtree(tree, path12, NULL));
  _dbus_assert(find_subtree(tree, path13, NULL));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path14));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path11));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path5));
  _dbus_object_tree_unregister_and_unlock(tree, path12);
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path12));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path9));
  _dbus_assert(find_subtree_registered_or_unregistered(tree, path5));
  _dbus_object_tree_unregister_and_unlock(tree, path13);
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path13));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path10));
  _dbus_assert(!find_subtree_registered_or_unregistered(tree, path5));
#if 0
  _dbus_object_tree_unregister_and_unlock(tree, path0);
  _dbus_object_tree_unregister_and_unlock(tree, path1);
  _dbus_object_tree_unregister_and_unlock(tree, path2);
  _dbus_object_tree_unregister_and_unlock(tree, path3);
  _dbus_object_tree_unregister_and_unlock(tree, path4);
#endif
  if (!do_register(tree, path0, TRUE, 0, tree_test_data)) goto out;
  if (!do_register(tree, path1, FALSE, 1, tree_test_data)) goto out;
  if (!do_register(tree, path2, TRUE, 2, tree_test_data)) goto out;
  if (!do_register(tree, path3, TRUE, 3, tree_test_data)) goto out;
  if (!do_register(tree, path4, TRUE, 4, tree_test_data)) goto out;
  if (!do_register(tree, path5, TRUE, 5, tree_test_data)) goto out;
  if (!do_register(tree, path6, FALSE, 6, tree_test_data)) goto out;
  if (!do_register(tree, path7, TRUE, 7, tree_test_data)) goto out;
  if (!do_register(tree, path8, TRUE, 8, tree_test_data)) goto out;
#if 0
  spew_tree (tree);
#endif
  if (!do_test_dispatch(tree, path0, 0, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path1, 1, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path2, 2, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path3, 3, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path4, 4, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path5, 5, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path6, 6, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path7, 7, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
  if (!do_test_dispatch(tree, path8, 8, tree_test_data, _DBUS_N_ELEMENTS(tree_test_data))) goto out;
out:
  if (tree) {
      _dbus_object_tree_ref(tree);
      _dbus_object_tree_unref(tree);
      _dbus_object_tree_unref(tree);
  }
  return TRUE;
}
dbus_bool_t _dbus_object_tree_test(void) {
  return _dbus_test_oom_handling("object tree", object_tree_test_iteration, NULL);
}
#endif
#endif