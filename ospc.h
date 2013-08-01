
#ifndef OSPc_H_
#define OSPc_H_
#include "syscx.h"
#include "fileb.h"

typedef struct OSPc OSPc;

/**
 *               _
 *              | |
 *  this-proc o x |
 *            |   |
 *     ospc   x o |
 *              |_|
 *
 * The /xf/ and /of/ in OSPc are the 'o' and 'x' of /this-proc/ respectively.
 **/
struct OSPc
{
  AlphaTab cmd;
  TableT(AlphaTab) args;
  OFile* of; /**< Write to process (its stdin).**/
  XFile* xf; /**< Read from process (its stdout).**/
  pid_t pid;
  OFileB ofb;
  XFileB xfb;
  int status;
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
  init_OFileB( &p.ofb );
  init_XFileB( &p.xfb );
  p.status = 0;
  return p;
}

bool
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

