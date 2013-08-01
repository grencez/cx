/*
 * \file xfile.h
 */
#ifndef XFile_H_
#define XFile_H_
#include "alphatab.h"

typedef struct XFile XFile;
typedef struct XFileCtx XFileCtx;
typedef struct XFileVT XFileVT;

struct XFile
{
  TableT(byte) buf;
  ujint off;
  ujint flushsz;
  XFileCtx* ctx;
};

struct XFileCtx
{
  const XFileVT* vt;
};


struct XFileVT
{
  bool (*xget_chunk_fn) (XFile*);
  //void (*flush_fn) (XFile);

  void (*free_fn) (XFile*);
  void (*close_fn) (XFile*);
};

void
close_XFile (XFile* xf);
void
lose_XFile (XFile* xf);
void
free_XFile (XFile* xf);
void
flush_XFile (XFile* f);

void
xget_XFile (XFile* xf);
char*
getline_XFile (XFile* in);
char*
getlined_XFile (XFile* xf, const char* delim);
void
skipds_XFile (XFile* xf, const char* delims);
char*
nextds_XFile (XFile* in, char* ret_match, const char* delims);
char*
tods_XFile (XFile* xf, const char* delims);
char*
nextok_XFile (XFile* xf, char* ret_match, const char* delims);
void
inject_XFile (XFile* in, XFile* src, const char* delim);
void
skipto_XFile (XFile* xf, const char* pos);

bool
xget_int_XFile (XFile* xf, int*);
bool
xget_uint_XFile (XFile* xf, uint*);
bool
xget_ujint_XFile (XFile* xf, ujint*);
bool
xget_real_XFile (XFile* xf, real*);
bool
xget_char_XFile (XFile* xf, char*);

/* Implemented in syscx.c */
XFile* stdin_XFile ();

qual_inline
  void
init_XFile (XFile* xf)
{
  static char empty[1] = { 0 };
  InitTable( xf->buf );
  xf->buf.s = (byte*) empty;
  xf->buf.sz = 1;
  xf->off = 0;
  xf->flushsz = 0;
  xf->ctx = 0;
}

qual_inline
  void
olay_XFile (XFile* olay, XFile* xf, uint off)
{
  init_XFile (olay);
  olay->buf.s = &xf->buf.s[off];
  olay->buf.sz = xf->off - off;
}

qual_inline
  void
mayflush_XFile (XFile* xf, Trit may)
{
  if (may == Yes)  xf->flushsz = 1;

  if (xf->flushsz > 0 && xf->off > CeilQuot( xf->buf.sz, 4 ))
    flush_XFile (xf);

  if (may == Nil)  xf->flushsz = 0;
}

qual_inline
  char*
cstr1_XFile (XFile* f, ujint off)
{ return (char*) &f->buf.s[off]; }

qual_inline
  char*
cstr_XFile (XFile* xf)
{ return cstr1_XFile (xf, xf->off); }

qual_inline
  AlphaTab
AlphaTab_XFile (XFile* xf, ujint off)
{
  DeclTable( char, t );
  t.s = (char*) &xf->buf.s[off];
  t.sz = (xf->off - off) / sizeof(char);
  return t;
}

#endif

