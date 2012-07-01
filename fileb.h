
#ifndef FileB_H_
#define FileB_H_

#include "table.h"

#include <stdarg.h>
#include <stdio.h>

#ifndef DeclTableT_byte
#define DeclTableT_byte
DeclTableT( byte, byte );
#endif

#ifndef DeclTableT_char
#define DeclTableT_char
DeclTableT( char, char );
#endif

#ifndef FileB
#define FileB FileB
typedef struct FileB FileB;
#endif
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
    TableSzT(byte) off;
    TableSzT(byte) flushsz;
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
        /* TODO - Flag for auto-flush so packets can be sent.*/
    FileB_Format fmt;
    TableT(char) pathname;
    TableT(char) filename;
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
ensure_FileB (FileB* f, TableSzT(byte) n);
void
setfmt_FileB (FileB* f, FileB_Format fmt);
bool
open_FileB (FileB* f, const char* pathname, const char* filename);
void
set_FILE_FileB (FileB* f, FILE* file);
char*
load_FileB (FileB* f);

void
flushx_FileB (FileB* f);
void
flush_XFileB (XFileB* f);
void
mayflush_XFileB (XFileB* xf);
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
dump_uint_OFileB (OFileB* f, uint x);
void
dump_real_OFileB (OFileB* f, real x);
void
dump_char_OFileB (OFileB* f, char c);
void
dump_cstr_OFileB (OFileB* f, const char* s);
void
vprintf_OFileB (OFileB* f, const char* fmt, va_list args);
void
printf_OFileB (OFileB* f, const char* fmt, ...);

void
dumpn_byte_FileB (FileB* f, const byte* a, TableSzT(byte) n);
void
dumpn_char_OFileB (OFileB* of, const char* a, TableSzT(byte) n);

char*
load_uint_cstr (uint* ret, const char* in);
char*
load_int_cstr (int* ret, const char* in);
char*
load_real_cstr (real* ret, const char* in);

bool
load_uint_FileB (FileB* f, uint* x);
bool
load_int_XFileB (XFileB* xf, int* x);
bool
load_real_XFileB (XFileB* xf, real* x);

bool
loadn_byte_FileB (FileB* f, byte* a, TableSzT(byte) n);

qual_inline
    void
set_mayflush_XFileB (XFileB* xf, bool may)
{
    xf->flushsz = may ? 1 : 0;
}

qual_inline
    XFileB
olay_XOFileB (XOFileB* xf, uint off)
{
    XFileB olay = dflt_XFileB ();
    olay.buf.s = &xf->buf.s[off];
    olay.buf.sz = xf->off - off;
    return olay;
}
qual_inline
XFileB olay_XFileB (OFileB* xf, uint off) { return olay_XOFileB (xf, off); }
qual_inline
XFileB olay_OFileB (OFileB* of, uint off) { return olay_XOFileB (of, off); }

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

    /* Implemented in sys-cx.c */
FileB* stdin_FileB ();
FileB* stdout_FileB ();
FileB* stderr_FileB ();
XFileB* stdin_XFileB ();
OFileB* stdout_OFileB ();
OFileB* stderr_OFileB ();


qual_inline
    char*
dup_cstr (const char* s)
{
    uint n = strlen (s) + 1;
    return DupliT( char, s, n );
}


#ifdef IncludeC
#include "fileb.c"
#endif
#endif

