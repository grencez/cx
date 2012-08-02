
#ifndef FileB_H_
#define FileB_H_

#include "alphatab.h"

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
    FileB_LoadChunk,
    FileB_DumpChunk,
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
    bool good;
    bool sink;
    bool byline;
    FileB_Format fmt;
    AlphaTab pathname;
    AlphaTab filename;
};

static const char WhiteSpaceChars[] = " \t\v\r\n";


void
lose_XOFileB (XOFileB* xo);
void
lose_OFileB (OFileB* of);
void
lose_XFileB (OFileB* xf);

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
void
set_FILE_FileB (FileB* f, FILE* file);
char*
load_FileB (FileB* f);

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
inject_FileB (FileB* in, FileB* src, const char* delim);
void
skipto_FileB (FileB* in, const char* pos);

bool
flusho_FileB (FileB* f);
void
flush_OFileB (OFileB* f);
void
mayflush_OFileB (OFileB* of);
void
dump_int_OFileB (OFileB* f, int x);
void
dump_uint_OFileB (OFileB* f, uint x);
void
dump_ujint_OFileB (OFileB* f, ujint x);
void
dump_real_OFileB (OFileB* f, real x);
void
dump_char_OFileB (OFileB* f, char c);
void
dump_AlphaTab (OFileB* f, const AlphaTab* t);
void
vprintf_OFileB (OFileB* f, const char* fmt, va_list args);
void
printf_OFileB (OFileB* f, const char* fmt, ...);

void
dumpn_byte_FileB (FileB* f, const byte* a, ujint n);
void
dumpn_char_OFileB (OFileB* of, const char* a, ujint n);

char*
load_uint_cstr (uint* ret, const char* in);
char*
load_int_cstr (int* ret, const char* in);
char*
load_real_cstr (real* ret, const char* in);

bool
load_uint_FileB (FileB* f, uint* x);
bool
load_char_XFileB (XFileB* xf, char* c);
bool
load_int_XFileB (XFileB* xf, int* x);
bool
load_real_XFileB (XFileB* xf, real* x);

bool
loadn_byte_FileB (FileB* f, byte* a, ujint n);

Trit
swapped_AlphaTab (const AlphaTab* a, const AlphaTab* b);

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
    char*
cstr_XOFileB (XOFileB* f)
{
    return (char*) &f->buf.s[f->off];
}
qual_inline
char* cstr_XFileB (XFileB* xf) { return cstr_XOFileB (xf); }
qual_inline
char* cstr_OFileB (OFileB* of) { return cstr_XOFileB (of); }
qual_inline
char* cstr_FileB (FileB* f) { return cstr_XOFileB (&f->xo); }

qual_inline
    void
dump_cstr_OFileB (OFileB* of, const char* s)
{
    const AlphaTab t = dflt1_AlphaTab (s);
    dump_AlphaTab (of, &t);
}

    /* Implemented in sys-cx.c */
FileB* stdin_FileB ();
FileB* stdout_FileB ();
FileB* stderr_FileB ();
XFileB* stdin_XFileB ();
OFileB* stdout_OFileB ();
OFileB* stderr_OFileB ();

#ifdef IncludeC
#include "fileb.c"
#endif
#endif

