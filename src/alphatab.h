
#ifndef AlphaTab_H_
#define AlphaTab_H_
#include "table.h"
typedef TableT(char) AlphaTab;
#define DeclTableT_AlphaTab
DeclTableT( AlphaTab, AlphaTab );

#define DeclAlphaTab( a )  DeclTable( char, a )

static const char WhiteSpaceChars[] = " \t\v\r\n";

/* dflt_AlphaTab() - Jump down to this for AlphaTab functions.*/

/** Duplicate a C string.**/
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
  bool
sfxeq_cstr (const char* s, const char* sfx)
{
  uint sz;
  uint sfx_sz;
  if (!s)  return false;
  sz = strlen (s);
  sfx_sz = strlen (sfx);
  if (sz < sfx_sz)  return false;
  return (0 == strncmp (&s[sz-sfx_sz], sfx, sfx_sz));
}

qual_inline
  AlphaTab
dflt_AlphaTab ()
{
  DeclAlphaTab( t );
  return t;
}

qual_inline
  void
init_AlphaTab (AlphaTab* ab)
{
  *ab = dflt_AlphaTab ();
}

qual_inline
  AlphaTab
dflt1_AlphaTab (const char* s)
{
  DeclAlphaTab( t );
  t.s = (char*) s;
  if (s)
    t.sz = strlen (s) + 1;
  return t;
}

qual_inline
  AlphaTab
dflt2_AlphaTab (const char* s, ujint sz)
{
  DeclAlphaTab( t );
  t.s = (char*) s;
  t.sz = sz;
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
cat_char_AlphaTab (AlphaTab* a, char c)
{
  if (a->sz > 0)  -- a->sz;
  GrowTable( *a, 2 );
  a->s[a->sz-2] = c;
  a->s[a->sz-1] = 0;
}

qual_inline
  void
cat_cstr_AlphaTab (AlphaTab* t, const char* s)
{
  const AlphaTab b = dflt1_AlphaTab (s);
  cat_AlphaTab (t, &b);
}

qual_inline
  void
cat1_cstr_AlphaTab (AlphaTab* t, const char* s, ujint sz)
{
  const AlphaTab b = dflt2_AlphaTab (s, sz);
  cat_AlphaTab (t, &b);
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
  void
tac_cstr_AlphaTab (AlphaTab* a, const char* s)
{
  const AlphaTab b = dflt1_AlphaTab (s);
  tac_AlphaTab (a, &b);
}

qual_inline
  AlphaTab
cons1_AlphaTab (const char* s)
{
  DeclAlphaTab( a );
  cat_cstr_AlphaTab (&a, s);
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
  const char*
ccstr_of_AlphaTab (const AlphaTab* ts)
{
  if (ts->sz == 0)
    return (char*) Static00;
  return ts->s;
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

qual_inline
  void
trim_end_AlphaTab (AlphaTab* a, ujint capac)
{
  bool nullt = (a->sz > 0) && (a->s[a->sz-1] == '\0');
  if (capac == 0)  return;
  if (!nullt) capac -= 1;
  Claim2( capac ,<, a->sz );
  a->sz -= capac;
  a->s[a->sz-1] = '\0';
}

qual_inline
  void
clear_AlphaTab (AlphaTab* a)
{ ClearTable( *a ); }

qual_inline
  void
flush_AlphaTab (AlphaTab* a)
{ FlushTable( *a ); }

qual_inline
  Bool
null_ck_AlphaTab (const AlphaTab* a)
{
  return !a->s;
}

qual_inline
  Bool
empty_ck_AlphaTab (const AlphaTab* a)
{
  return (a->sz == 0 || (a->s[0] == '\0'));
}

qual_inline
  void
assign2_AlphaTab (AlphaTab* dst, const AlphaTab* src, ujint beg, ujint end)
{
  const ujint sz = (end - beg) - OneIf(beg!=end && src->s[end-1]!='\0');
  if (sz == 0) {
    clear_AlphaTab (dst);
    return;
  }
  if (dst != src) {
    ResizeTable( *dst, sz );
    RepliT( char, dst->s, &src->s[beg], sz-1 );
  }
  else {
    if (beg != 0)
      memmove (dst->s, &src->s[beg], (sz-1)*sizeof(char));
    ResizeTable( *dst, sz );
  }
  dst->s[sz-1] = '\0';
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
Sign
cmp_cstr_loc (const char* const* a, const char* const* b);
void
cat_uint_AlphaTab (AlphaTab* a, uint x);
void
cat_ujint_AlphaTab (AlphaTab* a, ujint x);
void
cat_int_AlphaTab (AlphaTab* a, int x);

#endif

