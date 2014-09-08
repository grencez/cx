
#ifndef FileB_H_
#define FileB_H_

#include "syscx.h"
#include "xfile.h"
#include "ofile.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct FileB FileB;
typedef struct XFileB XFileB;
typedef struct OFileB OFileB;

enum FileB_Format {
    FileB_Ascii,
    FileB_Raw,
    FileB_NFormats
};
typedef enum FileB_Format FileB_Format;

struct FileB {
  FILE* f;
  fd_t fd;
  bool good;
  bool sink;
  bool byline;
  FileB_Format fmt;
  AlphaTab pathname;
  AlphaTab filename;
};

struct XFileB
{
  XFile xf;
  FileB fb;
};

struct OFileB
{
  OFile of;
  FileB fb;
};

void
init_XFileB (XFileB* xfb);
void
init_OFileB (OFileB* ofb);

void
close_XFileB (XFileB* f);
void
close_OFileB (OFileB* f);
void
lose_XFileB (XFileB* xfb);
void
lose_OFileB (OFileB* ofb);

byte*
ensure_XFileB (XFileB* xfb, ujint n);
byte*
ensure_OFileB (OFileB* ofb, ujint n);
void
setfmt_XFileB (XFileB* xfb, FileB_Format fmt);
void
setfmt_OFileB (OFileB* ofb, FileB_Format fmt);
bool
open_FileB (FileB* f, const char* pathname, const char* filename);
bool
openfd_FileB (FileB* fb, fd_t fd);
void
set_FILE_FileB (FileB* fb, FILE* file);
char*
xget_XFileB (XFileB* xfb);

void
flush_XFileB (XFileB* xfb);
bool
flush_OFileB (OFileB* ofb);

void
oputn_byte_OFileB (OFileB* f, const byte* a, ujint n);

bool
xgetn_byte_XFileB (XFileB* xfb, byte* a, ujint n);

AlphaTab
textfile_AlphaTab (const char* pathname, const char* filename);

qual_inline
  bool
nullt_FileB (const FileB* f)
{
  return (f->fmt < FileB_Raw);
}

qual_inline
  bool
byline_FileB (const FileB* f)
{
  return f->byline;
}


/* Implemented in syscx.c */
XFileB* stdin_XFileB ();
OFileB* stdout_OFileB ();
OFileB* stderr_OFileB ();
void
mktmppath_sysCx (AlphaTab* path);

qual_inline
  void
olay_OFile (XFile* olay, OFile* of, uint off)
{
  init_XFile (olay);
  olay->buf.s = &of->buf.s[off];
  olay->buf.sz = of->off - off;
}

qual_inline
  void
init_XFile_olay_OFile (XFile* olay, OFile* of)
{
  olay_OFile (olay, of, 0);
}

#endif

