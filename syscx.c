/**
 * \file syscx.c
 * Interact with the operating system.
 **/
#include "syscx.h"

#include "fileb.h"

#include <errno.h>
#include <signal.h>

DeclTableT( HookFn, struct { void (*f) (); void* x; } );

static DeclTable( HookFn, LoseFns );
static const char* ExeName = 0;

    const char*
exename_of_sysCx ()
{
    return ExeName;
}

static
    void
signal_hook_sysCx (int sig)
{
    DBog1( "Caught signal: %d", sig );
    fail_exit_sysCx ("");
}

#if 0
static
    int
parse_fd_uri_sysCx (const char* s)
{
    static const char fd_pfx[] = "fd://";
    int fd = -1;
    if (!s)  return -1;
    if (strlen (s) <= strlen (fd_pfx))  return -1;
    if (0 != memcmp (s, fd_pfx, strlen (fd_pfx)))  return -1;

    s += strlen (fd_pfx);
    s = load_int_cstr (&fd, s);
    if (!s)  return -1;
    return fd;
}
#endif

static
    fd_t
parse_fd_arg_sysCx (const char* s)
{
    int fd = -1;
    if (!s)  return -1;
    s = load_int_cstr (&fd, s);
    if (!s)  return -1;
    return fd;
}

static
    void
parse_args_sysCx (int* pargc, char*** pargv)
{
    bool execing = false;
    int argc = *pargc;
    char** argv = *pargv;
    int argi = 0;
    ExeName = argv[argi++];

    if (!argv[argi])  return;
    if (!eql_cstr (argv[argi++], MagicArgv1_sysCx))  return;

    while (argv[argi] && !eql_cstr (argv[argi], "--"))
    {
        char* arg = argv[argi++];
        if (eql_cstr (arg, "-stdxfd"))
        {
            fd_t fd = parse_fd_arg_sysCx (argv[argi++]);
            if (fd >= 0)  dup2_sysCx (fd, 0);
            else
            {
                DBog1( "Bad -stdxfd argument: %s", argv[argi-1] );
                fail_exit_sysCx ("");
            }
        }
        else if (eql_cstr (arg, "-stdofd"))
        {
            fd_t fd = parse_fd_arg_sysCx (argv[argi++]);
            if (fd >= 0)  dup2_sysCx (fd, 1);
            else
            {
                DBog1( "Bad -stdofd argument: %s", argv[argi-1] );
                fail_exit_sysCx ("");
            }
        }
        if (eql_cstr (arg, "-closefd"))
        {
            fd_t fd = parse_fd_arg_sysCx (argv[argi++]);
            if (fd >= 0)  close_sysCx (fd);
            else
            {
                DBog1( "Bad -closefd argument: %s", argv[argi-1] );
                fail_exit_sysCx ("");
            }
        }
        else if (eql_cstr (arg, "-exe"))
        {
            argv[0] = argv[argi++];
            ExeName = argv[0];
        }
        else if (eql_cstr (arg, "-exec"))
        {
            execing = true;
        }
    }

    argi = argc - argi;
    while (argi > 0)
    {
        argv[argi] = argv[argc];
        -- argi;
        -- argc;
    }

    *pargc = argc;
    *pargv = argv;

    if (execing)
        execvp_sysCx (argv);
}

    void
init_sysCx (int* pargc, char*** pargv)
{
    stderr_FileB ();
    parse_args_sysCx (pargc, pargv);

    stdin_FileB ();
    stdout_FileB ();
    signal (SIGSEGV, signal_hook_sysCx);
}

    void
push_losefn_sysCx (void (*f) ())
{
    DeclGrow1Table( HookFn, hook, LoseFns );
    hook->f = f;
    hook->x = 0;
}
    void
push_losefn1_sysCx (void (*f) (void*), void* x)
{
    DeclGrow1Table( HookFn, hook, LoseFns );
    hook->f = (void (*) ()) f;
    hook->x = x;
}

    void
lose_sysCx ()
{
    { BLoop( i, LoseFns.sz )
            /* Do in reverse because it's a stack.*/
        DeclEltTable( HookFn, hook, LoseFns, LoseFns.sz-i-1 );
        if (hook->x)
            ((void (*) (void*)) hook->f) (hook->x);
        else
            hook->f ();
    } BLose()
    LoseTable( LoseFns );

    lose_FileB (stdin_FileB ());
    lose_FileB (stdout_FileB ());
    lose_FileB (stderr_FileB ());
}

    void
fail_exit_sysCx (const char* msg)
{
    if (msg)
    {
        int err = errno;
        flusho_FileB (stderr_FileB ());
        /* Use literal stderr just in case we have memory problems.*/
        fprintf (stderr, "Failing out! %s\n", msg);
        if (err != 0)
            fprintf (stderr, "^^^ errno:%d %s\n", err, strerror (err));
    }
    lose_sysCx ();
    exit (1);
}

    void
dbglog_printf3 (const char* file,
                const char* func,
                uint line,
                const char* fmt,
                ...)
{
    va_list args;
    int err = errno;
    OFileB* f = stderr_OFileB ();

    printf_OFileB (f, "%s(%u) %s: ", file, line, func);

    va_start (args, fmt);
    vprintf_OFileB (f, fmt, args);
    va_end(args);

    dump_char_OFileB (f, '\n');

    if (err != 0)
    {
#if 0
            /* Why no work? */
        const uint n = 2048 * sizeof(char);
        char* s;

        printf_FileB (f, "^^^ errno:%d ", err);

        s = (char*) ensure_FileB (f, n);
        s[0] = '\0';

        strerror_r (err, s, n);

        f->off += strlen (s) * sizeof(char);
        dump_char_FileB (f, '\n');
#else
        printf_OFileB (f, "^^^ errno:%d %s\n", err, strerror (err));
#endif
        errno = 0;
    }
    flush_OFileB (f);
}


    FileB*
stdin_FileB ()
{
    static FileB f_mem;
    static FileB* f = 0;
    if (!f)
    {
        f = &f_mem;
        init_FileB (f);
        set_FILE_FileB (f, stdin);
        f->byline = true;
    }
    return f;
}

    FileB*
stdout_FileB ()
{
    static FileB f_mem;
    static FileB* f = 0;
    if (!f)
    {
        f = &f_mem;
        init_FileB (f);
        seto_FileB (f, true);
        set_FILE_FileB (f, stdout);
    }
    return f;
}

    FileB*
stderr_FileB ()
{
    static FileB f_mem;
    static FileB* f = 0;
    if (!f)
    {
        f = &f_mem;
        init_FileB (f);
        seto_FileB (f, true);
        set_FILE_FileB (f, stderr);
    }
    return f;
}

    XFileB*
stdin_XFileB ()
{
    FileB* fb = stdin_FileB ();
    return &fb->xo;
}

    OFileB*
stdout_OFileB ()
{
    FileB* fb = stdout_FileB ();
    return &fb->xo;
}

    OFileB*
stderr_OFileB ()
{
    FileB* fb = stderr_FileB ();
    return &fb->xo;
}

    int
pipe_sysCx (fd_t* fds)
{
#ifdef POSIX_SOURCE
    return pipe (fds);
#else
    return _pipe (fds, BUFSIZ, 0);
#endif
}

    int
dup2_sysCx (fd_t oldfd, fd_t newfd)
{
#ifdef POSIX_SOURCE
    return dup2 (oldfd, newfd);
#else
    return _dup2 (oldfd, newfd);
#endif
}

    int
close_sysCx (fd_t fd)
{
#ifdef POSIX_SOURCE
    return close (fd);
#else
    return _close (fd);
#endif
}

    FILE*
fdopen_sysCx (fd_t fd, const char* mode)
{
#ifdef POSIX_SOURCE
    return fdopen (fd, mode);
#else
    return _fdopen (fd, mode);
#endif
}

    pid_t
spawnvp_sysCx (char* const* argv)
{
#ifdef POSIX_SOURCE
    pid_t pid = fork ();
    if (pid > 0)  return pid;
    if (pid < 0)  return -1;
    execvp (argv[0], argv);
    fail_exit_sysCx ("execvp() failed");
    return -1;
#else
    return _spawnvp (_P_NOWAIT, argv[0], argv);
#endif
}

    void
execvp_sysCx (char* const* argv)
{
#ifdef POSIX_SOURCE
    execvp (argv[0], argv);
#else
    _execvp (argv[0], argv);
#endif
    fail_exit_sysCx ("execvp()");
}

    int
waitpid_sysCx (pid_t pid, int* status)
{
#ifdef POSIX_SOURCE
    return waitpid (pid, status, 0);
#else
    return _cwait (status, pid, 0);
#endif
}

