#if defined (G_ATOMIC_ARM)
#include <sched.h>
#endif
#include "gatomic.h"
#include "gthreadprivate.h"
#include "gthread.h"

#if defined (__GNUC__)
# if defined (G_ATOMIC_I486)
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  __asm__ __volatile__ ("lock; xaddl %0,%1"
                        : "=r" (result), "=m" (*atomic) 
			            : "0" (val), "m" (*atomic));
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  __asm__ __volatile__ ("lock; addl %1,%0"
			            : "=m" (*atomic)
			            : "ir" (val), "m" (*atomic));
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gint result;
  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
			            : "=a" (result), "=m" (*atomic)
			            : "r" (newval), "m" (*atomic), "0" (oldval));
  return result == oldval;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result;
  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
			            : "=a" (result), "=m" (*atomic)
			            : "r" (newval), "m" (*atomic), "0" (oldval));
  return result == oldval;
}
# elif defined (G_ATOMIC_SPARCV9)
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)			          \
  ({ 									                                  \
     gint __result;							                              \
     __asm__ __volatile__ ("cas [%4], %2, %0"				              \
                           : "=r" (__result), "=m" (*(atomic))		      \
                           : "r" (oldval), "m" (*(atomic)), "r" (atomic), \
                           "0" (newval));				                  \
     __result == oldval;						                          \
  })
#  if GLIB_SIZEOF_VOID_P == 4
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer  oldval, gpointer newval) {
  gpointer result;
  __asm__ __volatile__ ("cas [%4], %2, %0"
                        : "=r" (result), "=m" (*atomic)
                        : "r" (oldval), "m" (*atomic), "r" (atomic),
                        "0" (newval));
  return result == oldval;
}
#  elif GLIB_SIZEOF_VOID_P == 8
gboolean
g_atomic_pointer_compare_and_exchange (volatile gpointer G_GNUC_MAY_ALIAS *atomic, 
				       gpointer           oldval, 
				       gpointer           newval)
{
  gpointer result;
  gpointer *a = atomic;
  __asm__ __volatile__ ("casx [%4], %2, %0"
			: "=r" (result), "=m" (*a)
			: "r" (oldval), "m" (*a), "r" (a),
			"0" (newval));
  return result == oldval;
}
#  else
#    error "Your system has an unsupported pointer size"
#  endif
#  define G_ATOMIC_MEMORY_BARRIER	__asm__ __volatile__ ("membar #LoadLoad | #LoadStore | #StoreLoad | #StoreStore" : : : "memory")
# elif defined (G_ATOMIC_ALPHA)
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval) \
  ({ 									              \
     gint __result;							          \
     gint __prev;							          \
     __asm__ __volatile__ (						      \
        "       mb\n"							      \
        "1:     ldl_l   %0,%2\n"					  \
        "       cmpeq   %0,%3,%1\n"					  \
        "       beq     %1,2f\n"					  \
        "       mov     %4,%1\n"					  \
        "       stl_c   %1,%2\n"					  \
        "       beq     %1,1b\n"					  \
        "       mb\n"							      \
        "2:"								          \
        : "=&r" (__prev), 						      \
          "=&r" (__result)						      \
        : "m" (*(atomic)),						      \
          "Ir" (oldval),						      \
          "Ir" (newval)							      \
        : "memory");							      \
     __result != 0;							          \
  })
#  if GLIB_SIZEOF_VOID_P == 4
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gint result;
  gpointer prev;
  __asm__ __volatile__ (
        "       mb\n"
        "1:     ldl_l   %0,%2\n"
        "       cmpeq   %0,%3,%1\n"
        "       beq     %1,2f\n"
        "       mov     %4,%1\n"
        "       stl_c   %1,%2\n"
        "       beq     %1,1b\n"
        "       mb\n"
        "2:"
        : "=&r" (prev), 
          "=&r" (result)
        : "m" (*atomic),
          "Ir" (oldval),
          "Ir" (newval)
        : "memory");
  return result != 0;
}
#  elif GLIB_SIZEOF_VOID_P == 8
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gint result;
  gpointer prev;
  __asm__ __volatile__ (
        "       mb\n"
        "1:     ldq_l   %0,%2\n"
        "       cmpeq   %0,%3,%1\n"
        "       beq     %1,2f\n"
        "       mov     %4,%1\n"
        "       stq_c   %1,%2\n"
        "       beq     %1,1b\n"
        "       mb\n"
        "2:"
        : "=&r" (prev), 
          "=&r" (result)
        : "m" (*atomic),
          "Ir" (oldval),
          "Ir" (newval)
        : "memory");
  return result != 0;
}
#  else
#   error "Your system has an unsupported pointer size"
#  endif
#  define G_ATOMIC_MEMORY_BARRIER  __asm__ ("mb" : : : "memory")
# elif defined (G_ATOMIC_X86_64)
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  __asm__ __volatile__ ("lock; xaddl %0,%1"
                        : "=r" (result), "=m" (*atomic) 
			            : "0" (val), "m" (*atomic));
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  __asm__ __volatile__ ("lock; addl %1,%0"
                        : "=m" (*atomic)
                        : "ir" (val), "m" (*atomic));
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gint result;
  __asm__ __volatile__ ("lock; cmpxchgl %2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));
  return result == oldval;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result;
  __asm__ __volatile__ ("lock; cmpxchgq %q2, %1"
                        : "=a" (result), "=m" (*atomic)
                        : "r" (newval), "m" (*atomic), "0" (oldval));
  return result == oldval;
}
# elif defined (G_ATOMIC_POWERPC)
#   ifdef __OPTIMIZE__
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result, temp;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    1b"
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ (".Lieaa%=:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    .Lieaa%="
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#endif
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result, temp;  
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("1:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    1b"
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ (".Lia%=:       lwarx   %0,0,%3\n"
                        "         add     %1,%0,%4\n"
                        "         stwcx.  %1,0,%3\n"
                        "         bne-    .Lia%="
                        : "=&b" (result), "=&r" (temp), "=m" (*atomic)
                        : "b" (atomic), "r" (val), "m" (*atomic)
                        : "cr0", "memory");
#endif
}
#   else
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  do {
      result = *atomic;
  } while(!g_atomic_int_compare_and_exchange (atomic, result, result + val));
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  do {
      result = *atomic;
  } while(!g_atomic_int_compare_and_exchange (atomic, result, result + val));
}
#   endif
#   if GLIB_SIZEOF_VOID_P == 4
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gint result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1icae%=: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2icae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1icae%=\n"
                        ".L2icae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1pcae%=: lwarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2pcae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1pcae%=\n"
                        ".L2pcae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
#   elif GLIB_SIZEOF_VOID_P == 8
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gpointer result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: lwarx   %0,0,%1\n"
                        "   extsw   %0,%0\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1icae%=: lwarx   %0,0,%1\n"
                        "   extsw   %0,%0\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2icae%=\n"
                        "   stwcx.  %3,0,%1\n"
                        "   bne-    .L1icae%=\n"
                        ".L2icae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result;
#if ASM_NUMERIC_LABELS
  __asm__ __volatile__ ("sync\n"
                        "1: ldarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     2f\n"
                        "   stdcx.  %3,0,%1\n"
                        "   bne-    1b\n"
                        "2: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#else
  __asm__ __volatile__ ("sync\n"
                        ".L1pcae%=: ldarx   %0,0,%1\n"
                        "   subf.   %0,%2,%0\n"
                        "   bne     .L2pcae%=\n"
                        "   stdcx.  %3,0,%1\n"
                        "   bne-    .L1pcae%=\n"
                        ".L2pcae%=: isync"
                        : "=&r" (result)
                        : "b" (atomic), "r" (oldval), "r" (newval)
                        : "cr0", "memory");
#endif
  return result == 0;
}
#  else
#   error "Your system has an unsupported pointer size"
#  endif
#  define G_ATOMIC_MEMORY_BARRIER __asm__ ("sync" : : : "memory")
# elif defined (G_ATOMIC_IA64)
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  return __sync_fetch_and_add(atomic, val);
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  __sync_fetch_and_add (atomic, val);
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  return __sync_bool_compare_and_swap (atomic, oldval, newval);
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  return __sync_bool_compare_and_swap((long *)atomic, (long)oldval, (long)newval);
}
#  define G_ATOMIC_MEMORY_BARRIER __sync_synchronize ()
# elif defined (G_ATOMIC_S390)
#  define ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)			     \
  ({ 									                             \
     gint __result = oldval;					                     \
     __asm__ __volatile__ ("cs %0, %2, %1"				             \
                           : "+d" (__result), "=Q" (*(atomic))		 \
                           : "d" (newval), "m" (*(atomic)) : "cc" ); \
     __result == oldval;						                     \
  })
#  if GLIB_SIZEOF_VOID_P == 4
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result = oldval;
  __asm__ __volatile__ ("cs %0, %2, %1"
                        : "+d" (result), "=Q" (*(atomic))
                        : "d" (newval), "m" (*(atomic)) : "cc" );
  return result == oldval;
}
#  elif GLIB_SIZEOF_VOID_P == 8
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gpointer result = oldval;
  gpointer *a = atomic;
  __asm__ __volatile__ ("csg %0, %2, %1"
                        : "+d" (result), "=Q" (*a)
                        : "d" ((long)(newval)), "m" (*a) : "cc" );
  return result == oldval;
}
#  else
#    error "Your system has an unsupported pointer size"
#  endif
# elif defined (G_ATOMIC_ARM)
static volatile int atomic_spin = 0;
static int atomic_spin_trylock(void) {
  int result;
  asm volatile (
    "swp %0, %1, [%2]\n"
    : "=&r,&r" (result)
    : "r,0" (1), "r,r" (&atomic_spin)
    : "memory");
  if (result == 0) return 0;
  else return -1;
}
static void atomic_spin_lock (void) {
  while (atomic_spin_trylock())
    sched_yield();
}
static void atomic_spin_unlock (void) {
  atomic_spin = 0;
}
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  atomic_spin_lock();  
  result = *atomic;
  *atomic += val;
  atomic_spin_unlock();
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  atomic_spin_lock();
  *atomic += val;
  atomic_spin_unlock();
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gboolean result;
  atomic_spin_lock();
  if (*atomic == oldval) {
      result = TRUE;
      *atomic = newval;
  } else result = FALSE;
  atomic_spin_unlock();
  return result;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gboolean result;
  atomic_spin_lock();
  if (*atomic == oldval) {
      result = TRUE;
      *atomic = newval;
  } else result = FALSE;
  atomic_spin_unlock();
  return result;
}
# elif defined (G_ATOMIC_CRIS) || defined (G_ATOMIC_CRISV32)
#  ifdef G_ATOMIC_CRIS
#   define CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)		  \
  ({									                          \
     gboolean __result;							                  \
     __asm__ __volatile__ ("\n"						              \
                           "0:\tclearf\n\t"				          \
                           "cmp.d [%[Atomic]], %[OldVal]\n\t"	  \
                           "bne 1f\n\t"					          \
                           "ax\n\t"					              \
                           "move.d %[NewVal], [%[Atomic]]\n\t"	  \
                           "bwf 0b\n"					          \
                           "1:\tseq %[Result]"				      \
                           : [Result] "=&r" (__result),			  \
                                      "=m" (*(atomic))			  \
                           : [Atomic] "r" (atomic),			      \
                             [OldVal] "r" (oldval),			      \
                             [NewVal] "r" (newval),			      \
                                      "g" (*(gpointer*) (atomic)) \
                           : "memory");					          \
     __result;								                      \
  })
#  else
#   define CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval)		  \
  ({									                          \
     gboolean __result;							                  \
     __asm__ __volatile__ ("\n"						              \
                           "0:\tclearf p\n\t"				      \
                           "cmp.d [%[Atomic]], %[OldVal]\n\t"	  \
                           "bne 1f\n\t"					          \
                           "ax\n\t"					              \
                           "move.d %[NewVal], [%[Atomic]]\n\t"	  \
                           "bcs 0b\n"					          \
                           "1:\tseq %[Result]"				      \
                           : [Result] "=&r" (__result),			  \
                                      "=m" (*(atomic))			  \
                           : [Atomic] "r" (atomic),			      \
                             [OldVal] "r" (oldval),			      \
                             [NewVal] "r" (newval),			      \
                                      "g" (*(gpointer*) (atomic)) \
                           : "memory");					          \
     __result;								                      \
  })
#  endif
#define CRIS_CACHELINE_SIZE 32
#define CRIS_ATOMIC_BREAKS_CACHELINE(atomic)  (((gulong)(atomic) & (CRIS_CACHELINE_SIZE - 1)) > (CRIS_CACHELINE_SIZE - sizeof (atomic)))
gint __g_atomic_int_exchange_and_add(volatile gint   G_GNUC_MAY_ALIAS *atomic, gint val);
void __g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val);
gboolean __g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval);
gboolean __g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval);
gboolean
g_atomic_pointer_compare_and_exchange (volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  if (G_UNLIKELY (CRIS_ATOMIC_BREAKS_CACHELINE(atomic))) return __g_atomic_pointer_compare_and_exchange(atomic, oldval, newval);
  return CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval);
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  if (G_UNLIKELY (CRIS_ATOMIC_BREAKS_CACHELINE(atomic))) return __g_atomic_int_compare_and_exchange(atomic, oldval, newval);
  return CRIS_ATOMIC_INT_CMP_XCHG(atomic, oldval, newval);
}
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  if (G_UNLIKELY (CRIS_ATOMIC_BREAKS_CACHELINE(atomic))) return __g_atomic_int_exchange_and_add(atomic, val);
  do {
    result = *atomic;
  } while(!CRIS_ATOMIC_INT_CMP_XCHG(atomic, result, result + val));
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  if (G_UNLIKELY (CRIS_ATOMIC_BREAKS_CACHELINE(atomic))) return __g_atomic_int_add(atomic, val);
  do {
    result = *atomic;
  } while(!CRIS_ATOMIC_INT_CMP_XCHG(atomic, result, result + val));
}
#  define DEFINE_WITH_MUTEXES
#  define g_atomic_int_exchange_and_add __g_atomic_int_exchange_and_add
#  define g_atomic_int_add __g_atomic_int_add
#  define g_atomic_int_compare_and_exchange __g_atomic_int_compare_and_exchange
#  define g_atomic_pointer_compare_and_exchange __g_atomic_pointer_compare_and_exchange
# else
#  define DEFINE_WITH_MUTEXES
# endif
#else
# ifdef G_PLATFORM_WIN32
#  define DEFINE_WITH_WIN32_INTERLOCKED
# else
#  define DEFINE_WITH_MUTEXES
# endif
#endif
#ifdef DEFINE_WITH_WIN32_INTERLOCKED
# include <windows.h>
# if WINVER > 0x0400
#  define HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
# endif
gint32 g_atomic_int_exchange_and_add(volatile gint32 G_GNUC_MAY_ALIAS *atomic, gint32 val) {
  return InterlockedExchangeAdd (atomic, val);
}
void g_atomic_int_add(volatile gint32 G_GNUC_MAY_ALIAS *atomic, gint32 val) {
  InterlockedExchangeAdd (atomic, val);
}
gboolean g_atomic_int_compare_and_exchange(volatile gint32 G_GNUC_MAY_ALIAS *atomic, gint32 oldval, gint32 newval) {
#ifndef HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
  return (guint32)InterlockedCompareExchange((PVOID*)atomic, (PVOID)newval, (PVOID)oldval) == oldval;
#else
  return InterlockedCompareExchange(atomic, newval, oldval) == oldval;
#endif
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
# ifdef HAVE_INTERLOCKED_COMPARE_EXCHANGE_POINTER
  return InterlockedCompareExchangePointer (atomic, newval, oldval) == oldval;
# else
#  if GLIB_SIZEOF_VOID_P != 4
#   error "InterlockedCompareExchangePointer needed"
#  else
   return InterlockedCompareExchange (atomic, newval, oldval) == oldval;
#  endif
# endif
}
#endif
#ifdef DEFINE_WITH_MUTEXES
static GMutex *g_atomic_mutex;
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  g_mutex_lock(g_atomic_mutex);
  result = *atomic;
  *atomic += val;
  g_mutex_unlock(g_atomic_mutex);
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  g_mutex_lock(g_atomic_mutex);
  *atomic += val;
  g_mutex_unlock(g_atomic_mutex);
}
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  gboolean result;
  g_mutex_lock(g_atomic_mutex);
  if (*atomic == oldval) {
      result = TRUE;
      *atomic = newval;
  } else result = FALSE;
  g_mutex_unlock(g_atomic_mutex);
  return result;
}
gboolean g_atomic_pointer_compare_and_exchange(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer oldval, gpointer newval) {
  gboolean result;
  g_mutex_lock(g_atomic_mutex);
  if (*atomic == oldval) {
      result = TRUE;
      *atomic = newval;
  } else result = FALSE;
  g_mutex_unlock(g_atomic_mutex);
  return result;
}
#ifdef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
gint (g_atomic_int_get)(volatile gint G_GNUC_MAY_ALIAS *atomic) {
  gint result;
  g_mutex_lock(g_atomic_mutex);
  result = *atomic;
  g_mutex_unlock(g_atomic_mutex);
  return result;
}
void (g_atomic_int_set)(volatile gint G_GNUC_MAY_ALIAS *atomic, gint newval) {
  g_mutex_lock (g_atomic_mutex);
  *atomic = newval;
  g_mutex_unlock (g_atomic_mutex);
}
gpointer (g_atomic_pointer_get)(volatile gpointer G_GNUC_MAY_ALIAS *atomic) {
  gpointer result;
  g_mutex_lock(g_atomic_mutex);
  result = *atomic;
  g_mutex_unlock(g_atomic_mutex);
  return result;
}
void (g_atomic_pointer_set)(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer newval) {
  g_mutex_lock(g_atomic_mutex);
  *atomic = newval;
  g_mutex_unlock(g_atomic_mutex);
}
#endif
#elif defined (G_ATOMIC_OP_MEMORY_BARRIER_NEEDED)
gint (g_atomic_int_get)(volatile gint G_GNUC_MAY_ALIAS *atomic) {
  G_ATOMIC_MEMORY_BARRIER;
  return *atomic;
}
void (g_atomic_int_set)(volatile gint G_GNUC_MAY_ALIAS *atomic, gint newval) {
  *atomic = newval;
  G_ATOMIC_MEMORY_BARRIER; 
}
gpointer (g_atomic_pointer_get)(volatile gpointer G_GNUC_MAY_ALIAS *atomic) {
  G_ATOMIC_MEMORY_BARRIER;
  return *atomic;
}
void (g_atomic_pointer_set)(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer newval) {
  *atomic = newval;
  G_ATOMIC_MEMORY_BARRIER; 
}
#endif
#ifdef ATOMIC_INT_CMP_XCHG
gboolean g_atomic_int_compare_and_exchange(volatile gint G_GNUC_MAY_ALIAS *atomic, gint oldval, gint newval) {
  return ATOMIC_INT_CMP_XCHG (atomic, oldval, newval);
}
gint g_atomic_int_exchange_and_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  do {
    result = *atomic;
  } while(!ATOMIC_INT_CMP_XCHG(atomic, result, result + val));
  return result;
}
void g_atomic_int_add(volatile gint G_GNUC_MAY_ALIAS *atomic, gint val) {
  gint result;
  do {
    result = *atomic;
  } while(!ATOMIC_INT_CMP_XCHG (atomic, result, result + val));
}
#endif
void _g_atomic_thread_init(void) {
#ifdef DEFINE_WITH_MUTEXES
  g_atomic_mutex = g_mutex_new();
#endif
}
#ifndef G_ATOMIC_OP_MEMORY_BARRIER_NEEDED
gint (g_atomic_int_get)(volatile gint G_GNUC_MAY_ALIAS *atomic) {
  return g_atomic_int_get(atomic);
}
void (g_atomic_int_set)(volatile gint G_GNUC_MAY_ALIAS *atomic, gint newval) {
  g_atomic_int_set(atomic, newval);
}
gpointer (g_atomic_pointer_get)(volatile gpointer G_GNUC_MAY_ALIAS *atomic) {
  return g_atomic_pointer_get(atomic);
}
void (g_atomic_pointer_set)(volatile gpointer G_GNUC_MAY_ALIAS *atomic, gpointer newval) {
  g_atomic_pointer_set(atomic, newval);
}
#endif