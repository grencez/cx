
#include "ofile.h"
#include <stdio.h>

  void
close_OFile (OFile* of)
{
  if (of->ctx && of->ctx->vt->close_fn)
    of->ctx->vt->close_fn (of);
}

  void
lose_OFile (OFile* of)
{
  LoseTable( of->buf );
}

  void
free_OFile (OFile* of)
{
  if (of->ctx && of->ctx->vt->free_fn)
    of->ctx->vt->free_fn (of);
}

  void
flush_OFile (OFile* of)
{
  /* In the future, we may not want to flush all the time!*/
  /* Also, we may not wish to flush the whole buffer.*/
  if (of->ctx && of->ctx->vt->flush_fn)
  {
    of->ctx->vt->flush_fn (of);
  }
  else
  {
    Claim2( of->flushsz ,==, 0 );
    if (of->off > 0)
    {
      of->off = 0;
      of->buf.sz = 1;
      of->buf.s[0] = 0;
    }
  }
}

  void
oput_int_OFile (OFile* f, int x)
{
  EnsizeTable( f->buf, f->off + 50 );
  f->off += sprintf (cstr_OFile (f), "%i", x);
  mayflush_OFile (f);
}

  void
oput_uint_OFile (OFile* f, uint x)
{
  EnsizeTable( f->buf, f->off + 50 );
  f->off += sprintf (cstr_OFile (f), "%u", x);
  mayflush_OFile (f);
}

  void
oput_ujint_OFile (OFile* f, ujint x)
{
  EnsizeTable( f->buf, f->off + 50 );
  f->off += sprintf (cstr_OFile (f), "%lu", x);
  mayflush_OFile (f);
}

  void
oput_real_OFile (OFile* f, real x)
{
  EnsizeTable( f->buf, f->off + 50 );
  f->off += sprintf (cstr_OFile (f), "%.16e", x);
  mayflush_OFile (f);
}

  void
oput_char_OFile (OFile* f, char c)
{
  EnsizeTable( f->buf, f->off + 2 );
  f->buf.s[f->off] = c;
  f->buf.s[++f->off] = 0;
  mayflush_OFile (f);
}

  void
oput_AlphaTab (OFile* of, const AlphaTab* t)
{
  ujint n = t->sz;
  if (n == 0)  return;
  if (!t->s[n-1])  -- n;
  GrowTable( of->buf, n*sizeof(char) );
  memcpy (&of->buf.s[of->off], t->s, n*sizeof(char));
  of->buf.s[of->buf.sz-1] = 0;
  of->off += n;
  mayflush_OFile (of);
}

  void
vprintf_OFile (OFile* f, const char* fmt, va_list args)
{
  ujint sz = 2048;  /* Not good :( */
  int iret = 0;

  EnsizeTable( f->buf, f->off + sz );
  iret = vsprintf ((char*) &f->buf.s[f->off], fmt, args);
  Claim2( iret ,>=, 0 );
  Claim2( (uint) iret ,<=, sz );
  f->off += iret;
  mayflush_OFile (f);
}

  void
printf_OFile (OFile* f, const char* fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  vprintf_OFile (f, fmt, args);
  va_end (args);
}

  void
oputn_char_OFile (OFile* of, const char* a, ujint n)
{
  GrowTable( of->buf, n );
  memcpy (&of->buf.s[of->off], a, (n+1)*sizeof(char));
  of->off += n;
  mayflush_OFile (of);
}

