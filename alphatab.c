
#include "alphatab.h"
#include <stdio.h>

  char*
get_empty_cstr()
{
  static char empty[1] = { '\0' };
  return empty;
}

  char*
itoa_dup_cstr (int x)
{
  char buf[50];
  sprintf (buf, "%i", x);
  return dup_cstr (buf);
}

  char*
xget_uint_cstr (uint* ret, const char* in)
{
  unsigned long v;
  char* out = 0;

  assert (ret);
  assert (in);
  v = strtoul (in, &out, 10);

  if (out == in)  out = 0;
  if (out)
  {
    *ret = (uint) v;
    if (*ret != v)  out = 0;
  }
  return out;
}

  char*
xget_int_cstr (int* ret, const char* in)
{
  long v;
  char* out = 0;

  assert (ret);
  assert (in);
  v = strtol (in, &out, 10);

  if (out == in)  out = 0;
  if (out)
  {
    *ret = (int) v;
    if (*ret != v)  out = 0;
  }
  return out;
}

  char*
xget_real_cstr (real* ret, const char* in)
{
  double v;
  char* out = 0;

  assert (ret);
  assert (in);
  v = strtod (in, &out);

  if (out == in)  out = 0;
  if (out)  *ret = (real) v;
  return out;
}

  Sign
cmp_AlphaTab (const AlphaTab* a, const AlphaTab* b)
{
  ujint na = a->sz;
  ujint nb = b->sz;
  int ret;
  if (na > 0 && !a->s[na-1])  --na;
  if (nb > 0 && !b->s[nb-1])  --nb;

  if (na <= nb)
  {
    ret = memcmp (a->s, b->s, na * sizeof(char));
    if (ret == 0 && na < nb)
      ret = -1;
  }
  else
  {
    ret = memcmp (a->s, b->s, nb * sizeof(char));
    if (ret == 0)
      ret = 1;
  }
  return sign_of (ret);
}

  void
cat_uint_AlphaTab (AlphaTab* a, uint x)
{
  char buf[50];
  (void) sprintf(buf, "%u", x);
  cat_cstr_AlphaTab (a, buf);
}

  void
cat_ujint_AlphaTab (AlphaTab* a, ujint x)
{
  char buf[50];
  (void) sprintf(buf, "%lu", x);
  cat_cstr_AlphaTab (a, buf);
}

  void
cat_int_AlphaTab (AlphaTab* a, int x)
{
  char buf[50];
  (void) sprintf(buf, "%d", x);
  cat_cstr_AlphaTab (a, buf);
}

