/*
 * \file xfile.h
 * TODO: Not used yet.
 */
#ifndef XFile_H_
#define XFile_H_

typedef struct XFileBase XFileBase;
typedef XFileBase* XFile;
typedef struct XFileCtx XFileCtx;
typedef struct XFileOpVT XFileOpVT;

struct XFileBase
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
  bool (*get_chunk) (XFile);
  void (*flush) (XFile);
  void (*may_flush) (XFile);

  char* (*getline) (XFile);
  char* (*getlined) (XFile, const char*);
  void (*skipds) (XFile, const char*);
  char* (*nextds) (XFile, char*, const char*);
  char* (*tods) (XFile, const char*);
  char* (*nextok) (XFile, char*, const char*);
  void (*inject) (XFile, XFile, const char*);
  void (*skipto) (XFile, const char*);

  XFile (*make_fn) (XFileCtx*);
  void (*free_fn) (XFileCtx*, XFile);
  void (*close_fn) (XFileCtx*, XFile);
};

bool xget_int_XFile (XFile xf, int*);
bool xget_uint_XFile (XFile xf, uint*);
bool xget_ujint_XFile (XFile xf, ujint*);
bool xget_real_XFile (XFile xf, ujint*);
bool xget_char_XFile (XFile xf, char*);
bool xgetn_byte_XFile (XFileB* xf, byte*, ujint);


/* Implemented in syscx.c */
XFileB stdin_XFile ();

#endif

