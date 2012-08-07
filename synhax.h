
    /* Included from def.h */
#include <assert.h>

#define Concatify(a,b) a ## b
#define ConcatifyDef(a,b)  Concatify(a,b)

#define ArraySz( a )  (sizeof(a) / sizeof(*a))

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

#define CeilQuot( a, b ) \
    (((a) + (b) - 1) / (b))

#define DomMax( a )  (~((a) * (byte) 0))
#define InitDomMax( a )  do { a = 0; a = ~(a); } while (0)

#define BSfx( a, op, b, sfx )  (a)sfx op (b)sfx

#define UFor( i, bel )  for (i = 0; i < (bel); ++i)

#define BInit() {
#define BLose() }
#define BLoopT( T, i, bel )  T i; for (i = 0; i < (bel); ++i) BInit()
#define BLoop( i, bel )  BLoopT( uint, i, bel )
#define BUjFor( i, bel )  BLoopT( ujint, i, bel )

#define AccepTok( line, tok ) \
    ((0 == strncmp ((line), (tok), strlen(tok))) \
     ? ((line) = &(line)[strlen(tok)]) \
     : 0)

#define DecloStack( T, x )  T onstack_##x; T* const restrict x = &onstack_##x
#define DecloStack1( T, x, v ) \
    T onstack_##x = (v); T* const restrict x = &onstack_##x

#define AllocT( T, n ) \
    ((n) == 0 ? (T*) 0 : \
     (T*) malloc ((n) * sizeof (T)))

#define RepliT( T, a, b, n )  do \
{ \
    if ((n) > 0)  memcpy (a, b, (n) * sizeof (T)); \
} while (0)

#define DupliT( T, a, n ) \
    ((n) == 0 ? (T*) 0 : \
     (T*) memcpy (malloc ((n) * sizeof (T)), a, (n) * sizeof (T)))

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
#define DBog_ujint(x)  DBog2( "%s:%lu", #x, (ujint)(x) )

void fail_exit_sysCx (const char* msg);
#ifndef NDEBUG
#define Claim( x )  assert(x)
#else
#define Claim( x )  do \
{ \
    if (!(x)) \
    { \
        DBog1( "%s failed.", #x ); \
        fail_exit_sysCx (""); \
    } \
} while (0)
#endif
#define Claim2( a ,op, b )  Claim((a) op (b))


    /** Cascading if statement.**/
#define BCasc(cond, inv, msg) \
    if (!(inv) || !(cond)) \
    { \
        inv = false; \
        if (msg) \
        { \
            DBog2( "(%s => !(%s))", msg, #cond ); \
        } \
    } \
    BLose() if (inv) BInit()

