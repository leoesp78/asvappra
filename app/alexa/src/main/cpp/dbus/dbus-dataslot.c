#include "config.h"
#include "dbus-dataslot.h"
#include "dbus-threads-internal.h"
#include "dbus-test-tap.h"

dbus_bool_t _dbus_data_slot_allocator_init(DBusDataSlotAllocator *allocator, DBusGlobalLock lock) {
  allocator->allocated_slots = NULL;
  allocator->n_allocated_slots = 0;
  allocator->n_used_slots = 0;
  allocator->lock = lock;
  return TRUE;
}
dbus_bool_t _dbus_data_slot_allocator_alloc(DBusDataSlotAllocator *allocator, dbus_int32_t *slot_id_p) {
  dbus_int32_t slot;
  if (!_dbus_lock(allocator->lock)) return FALSE;
  if (*slot_id_p >= 0) {
      slot = *slot_id_p;
      _dbus_assert(slot < allocator->n_allocated_slots);
      _dbus_assert(allocator->allocated_slots[slot].slot_id == slot);
      allocator->allocated_slots[slot].refcount += 1;
      goto out;
  }
  _dbus_assert(*slot_id_p < 0);
  if (allocator->n_used_slots < allocator->n_allocated_slots) {
      slot = 0;
      while (slot < allocator->n_allocated_slots) {
          if (allocator->allocated_slots[slot].slot_id < 0) {
              allocator->allocated_slots[slot].slot_id = slot;
              allocator->allocated_slots[slot].refcount = 1;
              allocator->n_used_slots += 1;
              break;
          }
          ++slot;
      }
      _dbus_assert(slot < allocator->n_allocated_slots);
  } else {
      DBusAllocatedSlot *tmp;
      slot = -1;
      tmp = dbus_realloc(allocator->allocated_slots,sizeof(DBusAllocatedSlot) * (allocator->n_allocated_slots + 1));
      if (tmp == NULL) goto out;
      allocator->allocated_slots = tmp;
      slot = allocator->n_allocated_slots;
      allocator->n_allocated_slots += 1;
      allocator->n_used_slots += 1;
      allocator->allocated_slots[slot].slot_id = slot;
      allocator->allocated_slots[slot].refcount = 1;
  }
  _dbus_assert(slot >= 0);
  _dbus_assert(slot < allocator->n_allocated_slots);
  _dbus_assert(*slot_id_p < 0);
  _dbus_assert(allocator->allocated_slots[slot].slot_id == slot);
  _dbus_assert(allocator->allocated_slots[slot].refcount == 1);
  *slot_id_p = slot;
  _dbus_verbose("Allocated slot %d on allocator %p total %d slots allocated %d used\n", slot, allocator, allocator->n_allocated_slots, allocator->n_used_slots);
out:
  _dbus_unlock(allocator->lock);
  return slot >= 0;
}
void _dbus_data_slot_allocator_free(DBusDataSlotAllocator *allocator, dbus_int32_t *slot_id_p) {
  if (!_dbus_lock(allocator->lock)) _dbus_assert_not_reached("we should have initialized global locks before we allocated this slot");
  _dbus_assert(*slot_id_p < allocator->n_allocated_slots);
  _dbus_assert(allocator->allocated_slots[*slot_id_p].slot_id == *slot_id_p);
  _dbus_assert(allocator->allocated_slots[*slot_id_p].refcount > 0);
  allocator->allocated_slots[*slot_id_p].refcount -= 1;
  if (allocator->allocated_slots[*slot_id_p].refcount > 0) {
      _dbus_unlock(allocator->lock);
      return;
  }
  _dbus_verbose("Freeing slot %d on allocator %p total %d allocated %d used\n", *slot_id_p, allocator, allocator->n_allocated_slots, allocator->n_used_slots);
  allocator->allocated_slots[*slot_id_p].slot_id = -1;
  *slot_id_p = -1;
  allocator->n_used_slots -= 1;
  if (allocator->n_used_slots == 0) {
      dbus_free(allocator->allocated_slots);
      allocator->allocated_slots = NULL;
      allocator->n_allocated_slots = 0;
  }
  _dbus_unlock(allocator->lock);
}
void _dbus_data_slot_list_init(DBusDataSlotList *list) {
  list->slots = NULL;
  list->n_slots = 0;
}
dbus_bool_t _dbus_data_slot_list_set(DBusDataSlotAllocator *allocator, DBusDataSlotList *list, int slot, void *data, DBusFreeFunction free_data_func,
                                     DBusFreeFunction *old_free_func, void **old_data) {
#ifdef DBUS_DISABLE_ASSERT
  if (!_dbus_lock(allocator->lock)) _dbus_assert_not_reached("we should have initialized global locks before we allocated this slot");
  _dbus_assert(slot < allocator->n_allocated_slots);
  _dbus_assert(allocator->allocated_slots[slot].slot_id == slot);
  _dbus_unlock(allocator->lock);
#endif
  if (slot >= list->n_slots) {
      DBusDataSlot *tmp;
      int i;
      tmp = dbus_realloc(list->slots,sizeof(DBusDataSlot) * (slot + 1));
      if (tmp == NULL) return FALSE;
      list->slots = tmp;
      i = list->n_slots;
      list->n_slots = slot + 1;
      while(i < list->n_slots) {
          list->slots[i].data = NULL;
          list->slots[i].free_data_func = NULL;
          ++i;
      }
  }
  _dbus_assert(slot < list->n_slots);
  *old_data = list->slots[slot].data;
  *old_free_func = list->slots[slot].free_data_func;
  list->slots[slot].data = data;
  list->slots[slot].free_data_func = free_data_func;
  return TRUE;
}
void* _dbus_data_slot_list_get(DBusDataSlotAllocator *allocator, DBusDataSlotList *list, int slot) {
#ifdef DBUS_DISABLE_ASSERT
  if (!_dbus_lock(allocator->lock)) _dbus_assert_not_reached("we should have initialized global locks before we allocated this slot");
  _dbus_assert(slot >= 0);
  _dbus_assert(slot < allocator->n_allocated_slots);
  _dbus_assert(allocator->allocated_slots[slot].slot_id == slot);
  _dbus_unlock(allocator->lock);
#endif
  if (slot >= list->n_slots) return NULL;
  else return list->slots[slot].data;
}
void _dbus_data_slot_list_clear(DBusDataSlotList *list) {
  int i;
  i = 0;
  while(i < list->n_slots) {
      if (list->slots[i].free_data_func) (*list->slots[i].free_data_func)(list->slots[i].data);
      list->slots[i].data = NULL;
      list->slots[i].free_data_func = NULL;
      ++i;
  }
}
void _dbus_data_slot_list_free(DBusDataSlotList *list) {
  _dbus_data_slot_list_clear(list);
  dbus_free(list->slots);
  list->slots = NULL;
  list->n_slots = 0;
}
#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#include "dbus-test.h"
#include <stdio.h>

static int free_counter;
static void test_free_slot_data_func(void *data) {
  int i = _DBUS_POINTER_TO_INT(data);
  _dbus_assert(free_counter == i);
  ++free_counter;
}
dbus_bool_t _dbus_data_slot_test(void) {
  DBusDataSlotAllocator allocator;
  DBusDataSlotList list;
  int i;
  DBusFreeFunction old_free_func;
  void *old_data;
  if (!_dbus_data_slot_allocator_init(&allocator, _DBUS_LOCK_server_slots)) _dbus_test_fatal("no memory for allocator");
  _dbus_data_slot_list_init(&list);
#define N_SLOTS 100
  i = 0;
  while(i < N_SLOTS) {
      dbus_int32_t tmp = -1;
      _dbus_data_slot_allocator_alloc(&allocator, &tmp);
      if (tmp != i) _dbus_test_fatal("did not allocate slots in numeric order");
      ++i;
  }
  i = 0;
  while(i < N_SLOTS) {
      if (!_dbus_data_slot_list_set(&allocator, &list, i, _DBUS_INT_TO_POINTER (i), test_free_slot_data_func, &old_free_func, &old_data))
          _dbus_test_fatal("no memory to set data");
      _dbus_assert(old_free_func == NULL);
      _dbus_assert(old_data == NULL);
      _dbus_assert(_dbus_data_slot_list_get(&allocator, &list, i) == _DBUS_INT_TO_POINTER(i));
      ++i;
  }
  free_counter = 0;
  i = 0;
  while(i < N_SLOTS) {
      if (!_dbus_data_slot_list_set(&allocator, &list, i, _DBUS_INT_TO_POINTER(i), test_free_slot_data_func, &old_free_func, &old_data))
          _dbus_test_fatal("no memory to set data");
      _dbus_assert(old_free_func == test_free_slot_data_func);
      _dbus_assert(_DBUS_POINTER_TO_INT(old_data) == i);
      (*old_free_func)(old_data);
      _dbus_assert(i == (free_counter - 1));
      _dbus_assert(_dbus_data_slot_list_get(&allocator, &list, i) == _DBUS_INT_TO_POINTER(i));
      ++i;
  }
  free_counter = 0;
  _dbus_data_slot_list_free(&list);
  _dbus_assert(N_SLOTS == free_counter);
  i = 0;
  while(i < N_SLOTS) {
      dbus_int32_t tmp = i;
      _dbus_data_slot_allocator_free(&allocator, &tmp);
      _dbus_assert(tmp == -1);
      ++i;
  }
  return TRUE;
}
#endif