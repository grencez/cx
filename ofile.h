/*
 * \file ofile.h
 */
#ifndef OFile_H_
#define OFile_H_
#include "alphatab.h"
#include <stdarg.h>

typedef struct OFile OFile;
typedef struct OFileCtx OFileCtx;
typedef struct OFileVT OFileVT;

struct OFile
{
  TableT(byte) buf;
  ujint off;
  ujint flushsz;
  OFileCtx* ctx;
};

struct OFileCtx
{
  const OFileVT* vt;
};

struct OFileVT
{
  bool (*flush_fn) (OFile*);
  void (*free_fn) (OFile*);
  void (*close_fn) (OFile*);
};

void
close_OFile (OFile* of);
void
lose_OFile (OFile* of);
void
free_OFile (OFile* of);
void
flush_OFile (OFile* of);

void
oput_int_OFile (OFile* of, int x);
void
oput_uint_OFile (OFile* of, uint x);
void
oput_ujint_OFile (OFile* of, ujint x);
void
oput_real_OFile (OFile* of, real x);
void
oput_char_OFile (OFile* of, char c);
void
oput_AlphaTab (OFile* of, const AlphaTab* t);
void
vprintf_OFile (OFile* of, const char* fmt, va_list args);
void
printf_OFile (OFile* of, const char* fmt, ...);
void
oputn_char_OFile (OFile* of, const char* a, ujint n);

/* Implemented in syscx.c */
OFile* stdout_OFile ();
OFile* stderr_OFile ();

qual_inline
  void
init_OFile (OFile* of)
{
  static char empty[1] = { 0 };
  InitTable( of->buf );
  of->buf.s = (byte*) empty;
  of->buf.sz = 1;
  of->off = 0;
  of->flushsz = 0;
  of->ctx = 0;
}

qual_inline
  void
mayflush_OFile (OFile* of)
{
  if (of->flushsz > 0 && of->off > of->flushsz)
    of->ctx->vt->flush_fn (of);
}

qual_inline
  void
oput_cstr_OFile (OFile* of, const char* s)
{
  const AlphaTab t = dflt1_AlphaTab (s);
  oput_AlphaTab (of, &t);
}

qual_inline
  const char*
ccstr1_of_OFile (const OFile* of, ujint off)
{ return (char*) &of->buf.s[off]; }

qual_inline
  const char*
ccstr_of_OFile (const OFile* of)
{ return ccstr1_of_OFile (of, of->off); }

qual_inline
  char*
cstr1_OFile (OFile* f, ujint off)
{ return (char*) &f->buf.s[off]; }

qual_inline
  char*
cstr_OFile (OFile* of)
{ return cstr1_OFile (of, of->off); }

qual_inline
  AlphaTab
AlphaTab_OFile (OFile* of, ujint off)
{
  DeclTable( char, t );
  t.s = (char*) &of->buf.s[off];
  t.sz = (of->off - off) / sizeof(char);
  return t;
}

qual_inline
  void
cat_AlphaTab_OFile (AlphaTab* t, OFile* of)
{
  AlphaTab tmp = AlphaTab_OFile (of, 0);
  cat_AlphaTab (t, &tmp);
}

qual_inline
  void
copy_AlphaTab_OFile (AlphaTab* t, OFile* of)
{
  AlphaTab tmp = AlphaTab_OFile (of, 0);
  copy_AlphaTab (t, &tmp);
}

#endif

