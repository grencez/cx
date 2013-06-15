/**
 * \file synhax.h
 * All important macros.
 *
 * Included from def.h.
 **/
#include <assert.h>

#define Stringify(a) #a
#define Concatify(a,b) a ## b
#define ConcatifyDef(a,b)  Concatify(a,b)

/** Get size of an array (allocated on the stack in your current scope).**/
#define ArraySz( a )  (sizeof(a) / sizeof(*a))

/** Given the memory address of a structure's field,
 * get the address of the structure.
 * \param T      Type.
 * \param field  Name of the field.
 * \param p      Memory address of the field.
 **/
#define CastUp( T, field, p ) \
    ((T*) ((size_t) p - offsetof( T, field )))

#define EltZ( a, idx, elsz ) \
    ((void*) ((size_t) a + (size_t) ((idx) * (elsz))))

#define Elt( a, idx )  (&(a)[idx])

#define EltInZ( a, e, n, elsz ) \
    (((size_t) (a) <= (size_t) (e)) && \
     ((size_t) (e) < ((size_t) (a) + (n * elsz))))

#define IdxEltZ( a, e, elsz ) \
    ((size_t) ((size_t) (e) - (size_t) (a)) / (elsz))

#define IdxElt( a, e ) \
    IdxEltZ( a, e, sizeof(*a) )

/** Ceiling of integer division.
 * \param a  Dividend.
 * \param b  Divisor.
 **/
#define CeilQuot( a, b ) \
    (((a) + (b) - 1) / (b))

#define InitDomMax( a )  do { a = 0; a = ~(a); } while (0)

#define BSfx( a, op, b, sfx )  (a)sfx op (b)sfx

#define UFor( i, bel )  for (i = 0; i < (bel); ++i)

#define AccepTok( line, tok ) \
    ((0 == strncmp ((line), (tok), strlen(tok))) \
     ? ((line) = &(line)[strlen(tok)]) \
     : 0)

/** Declare a variable pointing to an "anonymous" object on the stack.
 * \param T  Type of variable when dereferenced.
 * \param x  Name of variable.
 **/
#define DecloStack( T, x )  T onstack_##x; T* const restrict x = &onstack_##x

/** Declare a variable pointing to an "anonymous" object on the stack
 * and initialize that object.
 * \param T  Type of variable when dereferenced.
 * \param x  Name of variable.
 * \param v  Initial value for the object.
 **/
#define DecloStack1( T, x, v ) \
    T onstack_##x = (v); T* const restrict x = &onstack_##x

/** Allocate memory via malloc() with lest casting.
 * \param T  Type.
 * \param n  Number of elements.
 * \return  NULL when the number of elements is zero.
 **/
#define AllocT( T, n ) \
    ((n) == 0 ? (T*) 0 : \
     (T*) malloc ((n) * sizeof (T)))

/** Replace memory via memcpy.
 * \param T  Type.
 * \param a  Destination block.
 * \param b  Source block.
 * \param n  Number of elements to copy.
 **/
#define RepliT( T, a, b, n )  do \
{ \
    if ((n) > 0)  memcpy (a, b, (n) * sizeof (T)); \
} while (0)

/** Duplicate memory.
 * \param T  Type.
 * \param a  Source block.
 * \param n  Number of elements to duplicate.
 * \return  NULL when the number of elements is zero.
 **/
#define DupliT( T, a, n ) \
    ((n) == 0 ? (T*) 0 : \
     (T*) memcpy (malloc ((n) * sizeof (T)), a, (n) * sizeof (T)))

/** Declare a variable pointing to heap-allocated memory.
 * \param T  Type.
 * \param a  Variable name.
 * \param n  Number of elements to duplicate.
 * \sa AllocT()
 **/
#define DeclAlloc( T, a, n ) \
    T* const restrict a = AllocT( T, n )


/** Implemented in sys-cx.c **/
void
dbglog_printf3 (const char* file,
                const char* func,
                uint line,
                const char* fmt,
                ...);

#define DBog0(s)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s)
#define DBog1(s,a)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s,a)
#define DBog2(s,a,b)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s,a,b)
#define DBog3(s,a,b,c)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s,a,b,c)
#define DBog4(s,a,b,c,d)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s,a,b,c,d)
#define DBog5(s,a,b,c,d,e)  dbglog_printf3 (__FILE__,__FUNC__,__LINE__,s,a,b,c,d,e)
#define DBog_ujint(x)  DBog2( "%s:%lu", #x, (ujint)(x) )

void
failout_sysCx (const char* msg);
#ifndef NDEBUG
#define Claim( x )  assert(x)
#else
#define Claim( x )  do \
{ \
    if (!(x)) \
    { \
        DBog1( "%s failed.", #x ); \
        failout_sysCx (""); \
    } \
} while (0)
#endif
#define Claim2( a ,op, b )  Claim((a) op (b))

#define Claim2_uint( a, op, b ) \
do { \
  if (!((a) op (b))) { \
    DBog5( "FAILED: (%s) where (%s == %u) and (%s == %u)", Stringify((a) op (b)), #a, (uint) (a), #b, (uint) (b) ); \
  } \
} while (0)

/** Wrap this in an if statement.
 * if (LegitCk( status == 0, invariant, "last_call()"))
 * { ... }
 */
#define LegitCk(cond, inv, msg) \
  ((inv) && !(cond))) { \
    inv = false; \
    if (msg) { \
      DBog2( "(%s => !(%s))", msg, #cond ); \
    } \
  } \
  else if ((inv)

