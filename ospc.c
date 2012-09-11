/**
 * \file ospc.c
 * Spawn and communicate with a process.
 **/
#include "ospc.h"

    bool
close_OSPc (OSPc* ospc)
{
    bool good = false;
    if (ospc->pid < 0)  return 0;
    close_FileB (&ospc->xfb);
    close_FileB (&ospc->ofb);
    /* if (ospc->pid > 0)  kill (ospc->pid, SIGKILL); */
    good = waitpid_sysCx (ospc->pid, &ospc->status);
    ospc->pid = -1;
    return good;
}

    void
lose_OSPc (OSPc* ospc)
{
    close_OSPc (ospc);
    lose_FileB (&ospc->xfb);
    lose_FileB (&ospc->ofb);
    lose_AlphaTab (&ospc->cmd);
    { BLoop( i, ospc->args.sz )
        lose_AlphaTab (&ospc->args.s[i]);
    } BLose()
    LoseTable( ospc->args );
}

/** Make a pipe to process input.
 **/
    void
stdxpipe_OSPc (OSPc* ospc)
{
    Claim( !ospc->of );
    ospc->of = &ospc->ofb.xo;
}

/** Make a pipe from process output.
 **/
    void
stdopipe_OSPc (OSPc* ospc)
{
    Claim( !ospc->xf );
    ospc->xf = &ospc->xfb.xo;
}

    bool
spawn_OSPc (OSPc* ospc)
{
    fd_t xfd[2] = { -1, -1 };
    fd_t ofd[2] = { -1, -1 };
    bool good = true;
    DeclTable( cstr, argv );
    uint nfrees = 0;

    BInit();

    if (ospc->of)  good = pipe_sysCx (xfd);
    BCasc( good, good, "pipe(xfd)" );
    if (ospc->xf)  good = pipe_sysCx (ofd);
    BCasc( good, good, "pipe(ofd)" );

    PushTable( argv, dup_cstr (exename_of_sysCx ()) );
    PushTable( argv, dup_cstr (MagicArgv1_sysCx) );
    PushTable( argv, dup_cstr ("-exec") );
    PushTable( argv, dup_cstr ("-exe") );
    PushTable( argv, dup_cstr (cstr_AlphaTab (&ospc->cmd)) );

    if (ospc->of)
    {
        cloexec_sysCx (xfd[1], true);
        PushTable( argv, dup_cstr ("-stdxfd") );
        PushTable( argv, itoa_dup_cstr (xfd[0]) );
        //PushTable( argv, dup_cstr ("-closefd") );
        //PushTable( argv, itoa_dup_cstr (xfd[1]) );
    }
    if (ospc->xf)
    {
        cloexec_sysCx (ofd[0], true);
        //PushTable( argv, dup_cstr ("-closefd") );
        //PushTable( argv, itoa_dup_cstr (ofd[0]) );
        PushTable( argv, dup_cstr ("-stdofd") );
        PushTable( argv, itoa_dup_cstr (ofd[1]) );
    }
    PushTable( argv, dup_cstr ("--") );
    nfrees = argv.sz;

    { BLoop( i, ospc->args.sz )
        PushTable( argv, cstr_AlphaTab (&ospc->args.s[i]) );
    } BLose()

    PushTable( argv, 0 );

    ospc->pid = spawnvp_sysCx (argv.s);
    BCasc( ospc->pid >= 0, good, "spawn_sysCx()" );

    /* The old switcharoo. Your input is my output and vice-versa.*/
    if (ospc->of)
    {
        closefd_sysCx (xfd[0]);
        ospc->ofb.fd = xfd[1];
        set_FILE_FileB (&ospc->ofb, fdopen_sysCx (ospc->ofb.fd, "wb"));
    }
    if (ospc->xf)
    {
        closefd_sysCx (ofd[1]);
        ospc->xfb.fd = ofd[0];
        set_FILE_FileB (&ospc->xfb, fdopen_sysCx (ospc->xfb.fd, "rb"));
    }

    BLose();

    { BLoop( i, nfrees )
        free (argv.s[i]);
    } BLose()
    LoseTable( argv );

    return good;
}

