
#ifndef AlphaTab_H_
#define AlphaTab_H_
#include "table.h"
typedef TableT(char) AlphaTab;
#define DeclTableT_AlphaTab
DeclTableT( AlphaTab, AlphaTab );

static const char WhiteSpaceChars[] = " \t\v\r\n";

char*
get_empty_cstr();

qual_inline
    char*
dup_cstr (const char* s)
{
    uint n = strlen (s) + 1;
    return DupliT( char, s, n );
}

qual_inline
  Sign
cmp_cstr (const char* a, const char* b)
{
  int ret;
  if (a == b)  return 0;
  if (!a)  return -1;
  if (!b)  return 1;
  ret = strcmp (a, b);
  return sign_of (ret);
}

qual_inline
    bool
eq_cstr (const char* a, const char* b)
{
    if (a == b)  return true;
    if (!a)  return false;
    if (!b)  return false;
    return (0 == strcmp (a, b));
}

qual_inline bool
eql_cstr (const char* a, const char* b)
{ return eq_cstr (a, b); }

qual_inline
  bool
pfxeq_cstr (const char* pfx, const char* s)
{
  if (!s)  return false;
  return (0 == strncmp (pfx, s, strlen (pfx)));
}

qual_inline
    AlphaTab
dflt_AlphaTab ()
{
    DeclTable( char, t );
    return t;
}

qual_inline
  void
init_AlphaTab (AlphaTab* ab)
{
  InitTable( *ab );
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
cstr_of_AlphaTab (AlphaTab* ts)
{
  if (ts->sz == 0 || ts->s[ts->sz-1] != '\0')
    PushTable( *ts, '\0' );
  return ts->s;
}

qual_inline
  char*
cstr_AlphaTab (AlphaTab* ts)
{
  return cstr_of_AlphaTab (ts);
}

qual_inline
    void
cat_cstr_AlphaTab (AlphaTab* t, const char* s)
{
    DecloStack1( AlphaTab, b, dflt1_AlphaTab (s) );
    cat_AlphaTab (t, b);
}

qual_inline
    void
tac_cstr_AlphaTab (AlphaTab* a, const char* s)
{
    DecloStack1( AlphaTab, b, dflt1_AlphaTab (s) );
    tac_AlphaTab (a, b);
}

qual_inline
    void
copy_AlphaTab (AlphaTab* a, const AlphaTab* b)
{
    CopyTable( *a, *b );
}

qual_inline
    void
copy_cstr_AlphaTab (AlphaTab* a, const char* s)
{
    AlphaTab b = dflt1_AlphaTab (s);
    CopyTable( *a, b );
}

qual_inline
    bool
endc_ck_AlphaTab (AlphaTab* a, char c)
{
    const char* s = cstr_AlphaTab (a);
    if (!s)  return false;
    s = strrchr (s, c);
    return (s && !s[1]);
}

char*
itoa_dup_cstr (int x);
char*
xget_uint_cstr (uint* ret, const char* in);
char*
xget_int_cstr (int* ret, const char* in);
char*
xget_ujint_cstr (ujint* ret, const char* in);
char*
xget_real_cstr (real* ret, const char* in);
Sign
cmp_AlphaTab (const AlphaTab* a, const AlphaTab* b);
void
cat_uint_AlphaTab (AlphaTab* a, uint x);
void
cat_ujint_AlphaTab (AlphaTab* a, ujint x);
void
cat_int_AlphaTab (AlphaTab* a, int x);

#endif

