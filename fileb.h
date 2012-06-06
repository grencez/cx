
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
    void (* op) (XOFileB*, FileB_Op, FileBOpArg*);
};

static char XOFileB_empty[1] = { 0 };
qual_inline
    XOFileB
dflt_XOFileB ()
{
    XOFileB f;
    InitTable( f.buf );
    f.buf.s = (byte*) XOFileB_empty;
    f.buf.sz = 1;
    f.off = 0;
    return f;
}

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
void
olay_FileB (XOFileB* olay, FileB* source_fb);
char*
load_FileB (FileB* f);
void
flushx_FileB (FileB* f);
char*
getline_FileB (FileB* in);
char*
getlined_FileB (FileB* in, const char* delim);
void
skipds_FileB (FileB* in, const char* delims);
char*
nextds_FileB (FileB* in, char* ret_match, const char* delims);
char*
nextok_FileB (FileB* in, char* ret_match, const char* delims);
void
inject_FileB (FileB* in, FileB* src, const char* delim);
void
skipto_FileB (FileB* in, const char* pos);

bool
flusho_FileB (FileB* f);
void
dump_uint_FileB (FileB* f, uint x);
void
dump_real_FileB (FileB* f, real x);
void
dump_char_FileB (FileB* f, char c);
void
dump_cstr_FileB (FileB* f, const char* s);
void
vprintf_FileB (FileB* f, const char* fmt, va_list args);
void
printf_FileB (FileB* f, const char* fmt, ...);

void
dumpn_byte_FileB (FileB* f, const byte* a, TableSzT(byte) n);
void
dumpn_char_FileB (FileB* f, const char* a, TableSzT(byte) n);

char*
load_uint_cstr (uint* ret, const char* in);
char*
load_real_cstr (real* ret, const char* in);

bool
load_uint_FileB (FileB* f, uint* x);
bool
load_real_FileB (FileB* f, real* x);

bool
loadn_byte_FileB (FileB* f, byte* a, TableSzT(byte) n);


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
cstr_FileB (FileB* f)
{
    return (char*) f->xo.buf.s;
}

    /* Implemented in sys-cx.c */
FileB* stdin_FileB ();
FileB* stdout_FileB ();
FileB* stderr_FileB ();


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

