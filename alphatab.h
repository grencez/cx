
#ifndef AlphaTab_H_
#define AlphaTab_H_
#include "table.h"
typedef TableT(char) AlphaTab;
#define DeclTableT_AlphaTab
DeclTableT( AlphaTab, AlphaTab );

static const char WhiteSpaceChars[] = " \t\v\r\n";


qual_inline
    char*
dup_cstr (const char* s)
{
    uint n = strlen (s) + 1;
    return DupliT( char, s, n );
}

qual_inline
    bool
eql_cstr (const char* a, const char* b)
{
    if (a == b)  return true;
    if (!a)  return false;
    if (!b)  return false;
    return (0 == strcmp (a, b));
}

qual_inline
    AlphaTab
dflt_AlphaTab ()
{
    DeclTable( char, t );
    return t;
}

qual_inline
    AlphaTab
dflt1_AlphaTab (const char* s)
{
    AlphaTab t = dflt_AlphaTab ();
    t.s = (char*) s;
    t.sz = strlen (s) + 1;
    return t;
}

qual_inline
void lose_AlphaTab (AlphaTab* ts) { LoseTable( *ts ); }

qual_inline
    void
cat_AlphaTab (AlphaTab* a, const AlphaTab* b)
{
    ujint n = b->sz;
    if (n == 0)  return;
    if (!b->s[n-1])  -- n;

    if (a->sz > 0)  -- a->sz;
    GrowTable( *a, n+1 );

    RepliT( char, &a->s[a->sz-(n+1)], b->s, n );
    a->s[a->sz-1] = 0;
}

qual_inline
    void
tac_AlphaTab (AlphaTab* a, const AlphaTab* b)
{
    ujint n = b->sz;
    if (n == 0)  return;
    if (!b->s[n-1])  -- n;
    if (n == 0)  return;

    GrowTable( *a, n );
    if (a->sz > n)
        memmove (&a->s[n], a->s, (a->sz-n)*sizeof(char));
    RepliT( char, a->s, b->s, n );
}

qual_inline
    AlphaTab
cons1_AlphaTab (const char* s)
{
    AlphaTab a = dflt_AlphaTab ();
    AlphaTab b = dflt1_AlphaTab (s);
    cat_AlphaTab (&a, &b);
    return a;
}

qual_inline
    char*
cstr_AlphaTab (AlphaTab* ts)
{
    if (ts->sz == 0 || ts->s[ts->sz-1] != '\0')
        PushTable( *ts, '\0' );
    return ts->s;
}

qual_inline
    void
cat_cstr_AlphaTab (AlphaTab* t, const char* s)
{
    DeclTable( char, b );
    b.s = (char*) s;
    b.sz = strlen (s) + 1;
    cat_AlphaTab (t, &b);
}

qual_inline
    void
copy_AlphaTab (AlphaTab* a, const AlphaTab* b)
{
    CopyTable( *a, *b );
}

#endif

