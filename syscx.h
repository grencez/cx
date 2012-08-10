
#ifndef sysCx_H_
#define sysCx_H_

static const char MagicArgv1_sysCx[] = "--opts://sysCx";

const char*
exename_of_sysCx ();
void
init_sysCx (int* pargc, char*** pargv);
void
push_losefn_sysCx (void (*f) ());
void
push_losefn1_sysCx (void (*f) (void*), void* x);
void
lose_sysCx ();

/* synhax.h - failout_sysCx() */
/* synhax.h - dbglog_printf3() */
/* fileb.h - stdin_FileB () */
/* fileb.h - stdout_FileB () */
/* fileb.h - stderr_FileB () */

#ifndef _WIN32
/* TODO: Figure out the correct POSIX_SOURCE to use!*/
#ifndef POSIX_SOURCE
#define POSIX_SOURCE
#endif
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif

#ifdef POSIX_SOURCE
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#else
#include <io.h>
#include <process.h>
typedef intptr_t pid_t;
#endif
typedef int fd_t;
#include <stdio.h>

int
pipe_sysCx (fd_t* fds);
int
dup2_sysCx (fd_t oldfd, fd_t newfd);
int
close_sysCx (fd_t fd);
FILE*
fdopen_sysCx (fd_t fd, const char* mode);
pid_t
spawnvp_sysCx (char* const* argv);
void
execvp_sysCx (char* const* argv);
int
waitpid_sysCx (pid_t pid, int* status);

#ifdef IncludeC
#include "syscx.c"
#endif
#endif

