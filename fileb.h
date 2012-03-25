
#ifndef FileB_H_
#define FileB_H_

#include "table.h"

#include <stdio.h>

#ifndef Table_char
#define Table_char Table_char
DeclTableT( char, char );
#endif

#ifndef FileB
#define FileB FileB
typedef struct FileB FileB;
#endif
struct FileB
{
    FILE* f;
    Table(char) buf;
    uint off;
    bool sink;
    Table(char) pathname;
    Table(char) filename;
};

static const char WhiteSpaceChars[] = " \t\v\r\n";


void
init_FileB (FileB* f);
void
close_FileB (FileB* f);
void
lose_FileB (FileB* f);
bool
open_FileB (FileB* f, const char* pathname, const char* filename, bool sink);
void
olay_FileB (FileB* olay, FileB* source);
char*
load_FileB (FileB* f);
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

#ifdef IncludeC
#include "fileb.c"
#endif
#endif

