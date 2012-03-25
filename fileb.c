
#include "fileb.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define AssertBelow( i, bel )  assert(i < bel)
#define AssertEq( expect, result )  assert((expect) == (result))


#ifndef Table_byte
#define Table_byte Table_byte
DeclTableT( byte, byte );
#endif


    void
init_FileB (FileB* f)
{
    static char empty[1] = { 0 };
    f->f = 0;
    InitTable( char, f->buf );
    f->buf.s = empty;
    f->buf.sz = 1;
    f->off = 0;
}

    void
lose_FileB (FileB* f)
{
    if (f->f)  fclose (f->f);
    LoseTable( char, f->buf );
}

    void
close_FileB (FileB* f)
{
    lose_FileB (f);
    init_FileB (f);
}

    void
olay_FileB (FileB* olay, FileB* source)
{
    init_FileB (olay);
    olay->buf.s = source->buf.s;
    olay->buf.sz = source->off;
    olay->buf.alloc_sz = 0;
}

static
    bool
read_FILE (FILE* in, Table( byte )* buf )
{
    bool good = true;
    long ret = 0;
    size_t sz = 0;

    if (good && (good = !!in))
    {
        ret = fseek (in, 0, SEEK_END);
    }
    if (good && (good = (ret == 0)))
    {
        ret = ftell (in);
    }
    if (good && (good = (ret >= 0)))
    {
        sz = ret;
        ret = fseek (in, 0, SEEK_SET);
    }
    if (good && (good = (ret == 0)))
    {
        GrowTable( byte, *buf, sz );
        ret = fread (buf->s, 1, sz, in);
    }
    if (good)
    {
        good = (ret == (long)sz);
    }

    if (in)  fclose (in);
    return good;
}

    char*
read_FileB (FileB* in)
{
    DeclTable( byte, buf );
    read_FILE (in->f, &buf);
    in->f = 0;
    close_FileB (in);
    in->buf.s = (char*) buf.s;
    in->buf.sz = buf.sz / sizeof(char);
    in->buf.alloc_sz = buf.alloc_sz / sizeof(char);
    GrowTable( char, in->buf, 1 );
    in->buf.s[in->buf.sz-1] = 0;
    return in->buf.s;
}

static
    bool
load_chunk_FileB (FileB* in)
{
    const uint n_per_chunk = BUFSIZ;
    Table(char)* buf = &in->buf;
    size_t n;
    char* s;

    if (!in->f)  return false;

    AssertBelow( 0, buf->sz );
    n = buf->sz - 1;
    GrowTable( char, *buf, n_per_chunk );
    s = &buf->s[n];

    n = fread (s, sizeof (char), n_per_chunk, in->f);
    s[n] = 0;
    buf->sz -= (n_per_chunk - n);
    return (n != 0);
}

static
    void
flushoff_FileB (FileB* in)
{
    Table(char)* buf = &in->buf;
    AssertBelow( 0, buf->sz );
    AssertEq( 0, buf->s[buf->sz-1] );
    if (in->off == 0)  return;
    buf->sz = buf->sz - in->off;
    if (buf->sz > 0)
    {
        memmove (buf->s, &buf->s[in->off], buf->sz * sizeof (char));
    }
    else
    {
        buf->s[0] = 0;
        buf->sz = 1;
    }
    in->off = 0;
}

    char*
getline_FileB (FileB* in)
{
    char* s;

    flushoff_FileB (in);
    s = strchr (in->buf.s, '\n');

    while (!s)
    {
        uint off = in->buf.sz - 1;
        if (!load_chunk_FileB (in))  break;
        s = strchr (&in->buf.s[off], '\n');
    }

    if (s)
    {
        s[0] = '\0';
        if (s != in->buf.s && s[-1] == '\r')
            s[-1] = '\0';
        s = &s[1];
        in->off = IndexInTable( char, in->buf, s );
    }
    else
    {
        in->off = in->buf.sz;
    }

    return (in->buf.sz == 1) ? 0 : in->buf.s;
}

    char*
getlined_FileB (FileB* in, const char* delim)
{
    char* s;
    uint delim_sz = strlen (delim);

    flushoff_FileB (in);
    s = strstr (in->buf.s, delim);

    while (!s)
    {
        uint off = in->buf.sz - 1;
        if (!load_chunk_FileB (in))  break;

        if (off < delim_sz)
            s = strstr (in->buf.s, delim);
        else
            s = strstr (&in->buf.s[1+off-delim_sz], delim);
    }

    if (s)
    {
        s[0] = '\0';
        s = &s[delim_sz];
        in->off = IndexInTable( char, in->buf, s );
    }
    else
    {
        in->off = in->buf.sz;
    }

    return (in->buf.sz == 1) ? 0 : in->buf.s;
}

    void
skipws_FileB (FileB* in)
{
    char* s;
    flushoff_FileB (in);

    s = in->buf.s;
    s = &s[strspn (s, WhiteSpaceChars)];

    while (!s[0]) {
        uint off = in->buf.sz - 1;
        if (!load_chunk_FileB (in))  break;
        s = &in->buf.s[off];
        s = &s[strspn (s, WhiteSpaceChars)];
    }
    in->off = IndexInTable( char, in->buf, s );
    flushoff_FileB (in);
}

    /** Inject content from a file /src/
     * at the current read position of file /in/.
     * This allows a trivial implementation of #include.
     **/
    void
inject_FileB (FileB* in, FileB* src, const char* delim)
{
    uint delim_sz = strlen (delim);

    read_FileB (src);

    if (src->buf.sz > 0)
    {
        uint sz = in->buf.sz - in->off;
        GrowTable( char, in->buf, src->buf.sz + delim_sz );
            /* Make room for injection.*/
        memmove (&in->buf.s[in->off + src->buf.sz + delim_sz],
                 &in->buf.s[in->off],
                 sz * sizeof (char));
            /* Inject file contents, assume src->buf.sz is strlen!*/
        memcpy (&in->buf.s[in->off],
                src->buf.s,
                src->buf.sz * sizeof (char));
    }
        /* Add the delimiter at the end.*/
    if (delim_sz > 0)
        memcpy (&in->buf.s[in->off + src->buf.sz],
                delim,
                delim_sz * sizeof (char));

    close_FileB (src);
}

