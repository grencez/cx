
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
};

static const char WhiteSpaceChars[] = " \t\v\r\n";


void
init_FileB (FileB* f);
void
lose_FileB (FileB* f);
void
close_FileB (FileB* f);
void
olay_FileB (FileB* olay, FileB* source);
char*
read_FileB (FileB* in);
char*
getline_FileB (FileB* in);
char*
getlined_FileB (FileB* in, const char* delim);
void
skipws_FileB (FileB* in);
void
inject_FileB (FileB* in, FileB* src, const char* delim);

#endif

