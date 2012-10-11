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
    failout_sysCx ("");
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
    s = xget_int_cstr (&fd, s);
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
    s = xget_int_cstr (&fd, s);
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
                failout_sysCx ("");
            }
        }
        else if (eql_cstr (arg, "-stdofd"))
        {
            fd_t fd = parse_fd_arg_sysCx (argv[argi++]);
            if (fd >= 0)  dup2_sysCx (fd, 1);
            else
            {
                DBog1( "Bad -stdofd argument: %s", argv[argi-1] );
                failout_sysCx ("");
            }
        }
        if (eql_cstr (arg, "-closefd"))
        {
            fd_t fd = parse_fd_arg_sysCx (argv[argi++]);
            if (fd >= 0)  closefd_sysCx (fd);
            else
            {
                DBog1( "Bad -closefd argument: %s", argv[argi-1] );
                failout_sysCx ("");
            }
        }
        else if (eql_cstr (arg, "-exe"))
        {
            argv[0] = argv[argi++];
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
    {:for (i ; LoseFns.sz)
            /* Do in reverse because it's a stack.*/
        DeclEltTable( HookFn, hook, LoseFns, LoseFns.sz-i-1 );
        if (hook->x)
            ((void (*) (void*)) hook->f) (hook->x);
        else
            hook->f ();
    }
    LoseTable( LoseFns );

    lose_FileB (stdin_FileB ());
    lose_FileB (stdout_FileB ());
    lose_FileB (stderr_FileB ());
}

    void
failout_sysCx (const char* msg)
{
    if (msg)
    {
        int err = errno;
        /* Use literal stderr just in case we have memory problems.*/
        FILE* f = stderr;

        /* Flush these so the next message is last.*/
        flusho_FileB (stdout_FileB ());
        flusho_FileB (stderr_FileB ());

        fprintf (f, "FAILOUT: %s\n", exename_of_sysCx ());
        if (msg[0])
            fprintf (f, "^^^ Reason: %s\n", msg);
        if (err != 0)
            fprintf (f, "^^^ errno:%d %s\n", err, strerror (err));
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

    oput_char_OFileB (f, '\n');

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
        oput_char_FileB (f, '\n');
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

    bool
pipe_sysCx (fd_t* fds)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = pipe (fds);
#else
    ret = _pipe (fds, BUFSIZ, 0);
#endif
    return (ret == 0);
}

    bool
dup2_sysCx (fd_t oldfd, fd_t newfd)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = dup2 (oldfd, newfd);
#else
    ret = _dup2 (oldfd, newfd);
#endif
    return (ret == 0);
}

    jint
read_sysCx (fd_t fd, void* buf, jint sz)
{
#ifdef POSIX_SOURCE
    return read (fd, buf, sz);
#else
    return _read (fd, buf, sz);
#endif
}

    bool
closefd_sysCx (fd_t fd)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = close (fd);
#else
    ret = _close (fd);
#endif
    return (ret == 0);
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

static void
oput_execvp_args (const char* fn, char* const* argv)
{
    OFileB* of = stderr_OFileB ();
    oput_cstr_OFileB (of, fn);
    oput_char_OFileB (of, ':');
    for (uint i = 0; argv[i]; ++i)
    {
        oput_char_OFileB (of, ' ');
        oput_cstr_OFileB (of, argv[i]);
    }
    oput_char_OFileB (of, '\n');
    flush_OFileB (of);
}

    pid_t
spawnvp_sysCx (char* const* argv)
{
    pid_t pid;
#ifdef POSIX_SOURCE
    pid = fork ();
    if (pid == 0)
        execvp_sysCx (argv);
#else
    pid = _spawnvp (_P_NOWAIT, argv[0], argv);
#endif
    if (pid < 0)
    {
        DBog0( "spawn() failed!" );
        oput_execvp_args ("spawnvp_sysCx()", argv);
    }
    return pid;
}

    void
execvp_sysCx (char* const* argv)
{
#ifdef POSIX_SOURCE
    execvp (argv[0], argv);
#else
    pid_t pid = -1;
    pid = spawnvp_sysCx (argv);
    if (pid >= 0)
    {
        int status = 1;
        if (!waitpid_sysCx (pid, &status))
            failout_sysCx ("Failed to wait for process.");
        exit (status);
    }
#endif
    DBog0( "execvp() failed!" );
    //DBog1( "PATH=%s", getenv ("PATH") );
    oput_execvp_args ("execvp_sysCx()", argv);
    failout_sysCx ("execvp() failed!");
}

    bool
waitpid_sysCx (pid_t pid, int* status)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = waitpid (pid, status, 0);
    if (status)
        *status = WEXITSTATUS( *status );
#else
    ret = _cwait (status, pid, 0);
#endif
    return (ret >= 0);
}

/**
 * \param path  Return value. Can come in as a hint for the path name.
 **/
    void
mktmppath_sysCx (AlphaTab* path)
{
    const char* v = 0;
#ifdef POSIX_SOURCE
    pid_t pid = getpid ();
#else
    pid_t pid = _getpid ();
#endif
    DecloStack1( OFileB, of, dflt_OFileB () );

#ifdef POSIX_SOURCE
    v = getenv ("TMPDIR");
    if (!v)  v = "/tmp";
#else
    v = getenv ("TEMP");
#endif

    if (!v)
    {
        path->sz = 0;
        return;
    }
    oput_cstr_OFileB (of, v);
    oput_char_OFileB (of, '/');
    oput_AlphaTab (of, path);
    oput_char_OFileB (of, '-');
    oput_ujint_OFileB (of, pid);
    oput_char_OFileB (of, '-');

    path->sz = 0;
    for (ujint i = 0; i < Max_ujint; ++i)
    {
        ujint off = of->off;
        oput_ujint_OFileB (of, i);

        if (mkdir_sysCx (cstr1_OFileB (of, 0)))
        {
            copy_AlphaTab_OFileB (path, of);
            break;
        }
        
        of->off = off;
    }
    lose_OFileB (of);
}

    void
setenv_sysCx (const char* key, const char* val)
{
#ifdef POSIX_SOURCE
    setenv (key, val, 1);
#else
    SetEnvironmentVariable (key, val);
    //DBog2( "key:%s val:%s", key, val );
#endif
}

    void
tacenv_sysCx (const char* key, const char* val)
{
#ifdef POSIX_SOURCE
    const char* sep = ":";
#else
    const char* sep = ";";
#endif
    char* v;
    DecloStack1( AlphaTab, dec, cons1_AlphaTab (val) );

    v = getenv (key);
    if (v)
    {
        cat_cstr_AlphaTab (dec, sep);
        cat_cstr_AlphaTab (dec, v);
    }

    setenv_sysCx (key, cstr_AlphaTab (dec));
    lose_AlphaTab (dec);
}

    void
cloexec_sysCx (fd_t fd, bool b)
{
#ifdef POSIX_SOURCE
    int flags = fcntl (fd, F_GETFD);

    if (flags == -1)
    {
        DBog0( "fcntl() GET failed." );
        return;
    }

    if (b == (0 != (flags & FD_CLOEXEC)))  return;

    if (b)  flags |= FD_CLOEXEC;
    else    flags ^= FD_CLOEXEC;

    if (fcntl (fd, F_SETFD, flags) == -1)
        DBog0( "fcntl() SET failed." );
#else
    SetHandleInformation ((HANDLE) _get_osfhandle (fd),
                          HANDLE_FLAG_INHERIT,
                          b ?  HANDLE_FLAG_INHERIT : 0);
#endif
}

    bool
chmodu_sysCx (const char* pathname, bool r, bool w, bool x)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    chmod (pathname, (r ? S_IRUSR : 0) | (w ? S_IWUSR : 0) | (x ? S_IXUSR : 0));
#else
    (void) x;
    _chmod (pathname, (r ? _S_IREAD : 0) | (w ? _S_IWRITE : 0));
#endif
    return (ret == 0);
}

    bool
mkdir_sysCx (const char* pathname)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = mkdir (pathname, 0700);
#else
    ret = _mkdir (pathname);
#endif
    return (ret == 0);
}

    bool
rmdir_sysCx (const char* pathname)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = rmdir (pathname);
#else
    ret = _rmdir (pathname);
#endif
    return (ret == 0);
}

    bool
chdir_sysCx (const char* pathname)
{
    int ret = -1;
#ifdef POSIX_SOURCE
    ret = chdir (pathname);
#else
    ret = _chdir (pathname);
#endif
    return (ret == 0);
}

