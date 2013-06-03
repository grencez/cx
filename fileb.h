
#ifndef FileB_H_
#define FileB_H_

#include "alphatab.h"
#include "syscx.h"

#include <stdarg.h>
#include <stdio.h>

typedef struct FileB FileB;
typedef struct XOFileB XOFileB;
typedef XOFileB XFileB;
typedef XOFileB OFileB;
typedef struct FileBOpArg FileBOpArg;

enum FileB_Format {
    FileB_Ascii,
    FileB_Raw,
    FileB_NFormats
};
typedef enum FileB_Format FileB_Format;

enum FileB_Op {
    FileB_XGetChunk,
    FileB_OPutChunk,
    FileB_FlushO,
    FileB_Close,
    FileB_NOps
};
typedef enum FileB_Op FileB_Op;


struct FileBOpArg
{
    union {
        struct {
            uint nbytes;
            const byte* bytes;
        } cdata;
    } as;
    bool good;
};
#define DeclFileBOpArg(arg) \
    FileBOpArg arg

qual_inline
    FileBOpArg
dflt_FileBOpArg ()
{
    FileBOpArg a;
    a.good = true;
    return a;
}


struct XOFileB
{
    TableT(byte) buf;
    ujint off;
    ujint flushsz;
    void (* op) (XOFileB*, FileB_Op, FileBOpArg*);
};

qual_inline
    XOFileB
dflt_XOFileB ()
{
    static char XOFileB_empty[1] = { 0 };
    XOFileB f;
    InitTable( f.buf );
    f.buf.s = (byte*) XOFileB_empty;
    f.buf.sz = 1;
    f.off = 0;
    f.op = 0;
    f.flushsz = 0;
    return f;
}
qual_inline
OFileB dflt_OFileB () { return dflt_XOFileB (); }
qual_inline
XFileB dflt_XFileB () { return dflt_XOFileB (); }

struct FileB
{
    XOFileB xo;
    FILE* f;
    fd_t fd;
    bool good;
    bool sink;
    bool byline;
    FileB_Format fmt;
    AlphaTab pathname;
    AlphaTab filename;
};

void
lose_XOFileB (XOFileB* xo);
void
lose_OFileB (OFileB* of);
void
lose_XFileB (OFileB* xf);

FileB
dflt_FileB ();
void
init_FileB (FileB* f);
void
close_FileB (FileB* f);
void
lose_FileB (FileB* f);
void
seto_FileB (FileB* f, bool sink);
byte*
ensure_FileB (FileB* f, ujint n);
void
setfmt_FileB (FileB* f, FileB_Format fmt);
bool
open_FileB (FileB* f, const char* pathname, const char* filename);
bool
openfd_FileB (FileB* fb, fd_t fd);
void
set_FILE_FileB (FileB* f, FILE* file);
char*
xget_FileB (FileB* f);

void
close_XFileB (XFileB* xf);
void
close_OFileB (OFileB* of);

void
flushx_FileB (FileB* f);
void
flush_XFileB (XFileB* f);
char*
getline_XFileB (XFileB* in);
char*
getlined_XFileB (XFileB* xf, const char* delim);
void
skipds_XFileB (XFileB* xf, const char* delims);
char*
nextds_XFileB (XFileB* in, char* ret_match, const char* delims);
char*
tods_XFileB (XFileB* xf, const char* delims);
char*
nextok_XFileB (XFileB* xf, char* ret_match, const char* delims);
void
inject_XFileB (XFileB* in, XFileB* src, const char* delim);
void
skipto_XFileB (XFileB* xf, const char* pos);

bool
flusho_FileB (FileB* f);
void
flush_OFileB (OFileB* f);
void
mayflush_OFileB (OFileB* of);
void
oput_int_OFileB (OFileB* f, int x);
void
oput_uint_OFileB (OFileB* f, uint x);
void
oput_ujint_OFileB (OFileB* f, ujint x);
void
oput_real_OFileB (OFileB* f, real x);
void
oput_char_OFileB (OFileB* f, char c);
void
oput_AlphaTab (OFileB* f, const AlphaTab* t);
void
vprintf_OFileB (OFileB* f, const char* fmt, va_list args);
void
printf_OFileB (OFileB* f, const char* fmt, ...);

void
oputn_byte_FileB (FileB* f, const byte* a, ujint n);
void
oputn_char_OFileB (OFileB* of, const char* a, ujint n);

void
xget_XFileB (XFileB* xf);

bool
xget_uint_FileB (FileB* f, uint* x);
bool
xget_char_XFileB (XFileB* xf, char* c);
bool
xget_int_XFileB (XFileB* xf, int* x);
bool
xget_real_XFileB (XFileB* xf, real* x);

bool
xgetn_byte_FileB (FileB* f, byte* a, ujint n);

qual_inline
    void
mayflush_XFileB (XFileB* xf, Trit may)
{
    if (may == Yes)  xf->flushsz = 1;

    if (xf->flushsz > 0 && xf->off > CeilQuot( xf->buf.sz, 4 ))
        flush_XFileB (xf);

    if (may == Nil)  xf->flushsz = 0;
}

qual_inline
    XFileB
olay_XFileB (XFileB* xf, uint off)
{
    XFileB olay = dflt_XFileB ();
    olay.buf.s = &xf->buf.s[off];
    olay.buf.sz = xf->off - off;
    return olay;
}
qual_inline
XFileB olay_OFileB (OFileB* of, uint off) { return olay_XFileB (of, off); }

qual_inline
    AlphaTab
AlphaTab_XFileB (XFileB* xf, ujint off)
{
    DeclTable( char, t );
    t.s = (char*) &xf->buf.s[off];
    t.sz = (xf->off - off) / sizeof(char);
    return t;
}

qual_inline
    void
cat_AlphaTab_OFileB (AlphaTab* t, OFileB* of)
{
    AlphaTab tmp = AlphaTab_XFileB (of, 0);
    cat_AlphaTab (t, &tmp);
}

qual_inline
    void
copy_AlphaTab_OFileB (AlphaTab* t, OFileB* of)
{
    AlphaTab tmp = AlphaTab_XFileB (of, 0);
    copy_AlphaTab (t, &tmp);
}

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


qual_inline
char* cstr1_XFileB (XFileB* f, ujint off) { return (char*) &f->buf.s[off]; }
qual_inline
char* cstr1_OFileB (OFileB* f, ujint off) { return (char*) &f->buf.s[off]; }
qual_inline
char* cstr_XFileB (XFileB* xf) { return cstr1_XFileB (xf, xf->off); }
qual_inline
char* cstr_OFileB (OFileB* of) { return cstr1_OFileB (of, of->off); }
qual_inline
char* cstr_FileB (FileB* f)
{
    if (f->sink)  return cstr_OFileB (&f->xo);
    else          return cstr_XFileB (&f->xo);
}

qual_inline
    void
oput_cstr_OFileB (OFileB* of, const char* s)
{
    const AlphaTab t = dflt1_AlphaTab (s);
    oput_AlphaTab (of, &t);
}

/* Implemented in syscx.c */
FileB* stdin_FileB ();
FileB* stdout_FileB ();
FileB* stderr_FileB ();
XFileB* stdin_XFileB ();
OFileB* stdout_OFileB ();
OFileB* stderr_OFileB ();
void
mktmppath_sysCx (AlphaTab* path);

#endif

