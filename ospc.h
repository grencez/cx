
#ifndef OSPc_H_
#define OSPc_H_
#include "syscx.h"
#include "fileb.h"

typedef struct OSPc OSPc;

struct OSPc
{
    AlphaTab cmd;
    TableT(AlphaTab) args;
    OFileB* of; /**< Write to process (its stdin).**/
    XFileB* xf; /**< Read from process (its stdout).**/
    pid_t pid;
    FileB ofb;
    FileB xfb;
};

qual_inline
    OSPc
dflt_OSPc ()
{
    OSPc p;
    p.cmd = dflt_AlphaTab ();
    InitTable( p.args );
    p.of = 0;
    p.xf = 0;
    p.pid = -1;
    init_FileB( &p.ofb );
    seto_FileB (&p.ofb, true);
    init_FileB( &p.xfb );
    return p;
}

int
close_OSPc (OSPc* ospc);
void
lose_OSPc (OSPc* ospc);
void
stdxpipe_OSPc (OSPc* ospc);
void
stdopipe_OSPc (OSPc* ospc);
bool
spawn_OSPc (OSPc* ospc);

#endif

