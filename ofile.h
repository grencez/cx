/*
 * \file ofile.h
 * TODO: Not used yet.
 */
#ifndef OFile_H_
#define OFile_H_

typedef struct OFileBase OFileBase;
typedef OFileBase* OFile;
typedef struct OFileCtx OFileCtx;
typedef struct OFileOpVT OFileOpVT;

struct OFileBase
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
  bool (*flush) (OFile);
  bool (*put_chunk_flush) (OFile);

  void (*may_flush) (OFile);

  void (*putn_char) (OFile, const char*, ujint n);
  void (*putn_byte) (OFile, const byte*, ujint n);

  void (*vprintf) (OFile, const char*, va_list);
  void (*printf) (OFile, const char*, ...);

  OFile (*make_fn) (OFileCtx*);
  void (*free_fn) (OFileCtx*, OFile);
  void (*close_fn) (OFile);
};

void
oput_int_OFile (OFile of, int x);
void
oput_uint_OFile (OFile of, uint x);
void
oput_ujint_OFile (OFile of, ujint x);
void
oput_real_OFile (OFile of, ujint x);
void
oput_char_OFile (OFile of, char c);
void
oput_cstr_OFile (OFile of, char* s);

/* Implemented in syscx.c */
OFileB stdout_OFile ();
OFileB stderr_OFile ();

#endif

