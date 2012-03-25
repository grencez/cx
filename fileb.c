
#include "fileb.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#ifndef Table_byte
#define Table_byte Table_byte
DeclTableT( byte, byte );
#endif

static const uint NPerChunk = BUFSIZ;


    void
init_FileB (FileB* f)
{
    static char empty[1] = { 0 };
    f->f = 0;
    InitTable( char, f->buf );
    f->buf.s = empty;
    f->buf.sz = 1;
    f->off = 0;
    f->sink = false;
    InitTable( char, f->pathname );
    InitTable( char, f->filename );
}

    void
close_FileB (FileB* f)
{
    if (f->sink)  flusho_FileB (f);
    if (f->f)
    {
        fclose (f->f);
        f->f = 0;
    }
}

    void
lose_FileB (FileB* f)
{
    close_FileB (f);
    LoseTable( char, f->buf );
    LoseTable( char, f->pathname );
    LoseTable( char, f->filename );
}

    bool
open_FileB (FileB* f, const char* pathname, const char* filename, bool sink)
{
    uint flen = strlen (filename);
    uint plen = (pathname ? strlen (pathname) : 0);
    char* s;
    f->sink = sink;

    SizeTable( char, f->filename, flen+1 );
    memcpy (f->filename.s, filename, (flen+1)*sizeof(char));

    SizeTable( char, f->pathname, plen+1+flen+1 );

    s = f->pathname.s;
    if (plen > 0)
    {
        memcpy (s, pathname, plen*sizeof(char));
        s = &s[plen];
        s[0] = '/';
        s = &s[1];
    }
    memcpy (s, filename, (flen+1)*sizeof(char));

    f->f = fopen (f->pathname.s, (sink ? "wb" : "rb"));
    f->pathname.s[plen] = 0;
    SizeTable( char, f->pathname, plen+1 );

    return !!f->f;
}

    void
olay_FileB (FileB* olay, FileB* source)
{
    init_FileB (olay);
    olay->buf.s = source->buf.s;
    olay->buf.sz = source->off;
    olay->buf.alloc_sz = 0;
}

    char*
load_FileB (FileB* f)
{
    bool good = true;
    long ret = 0;
    size_t sz = 0;
    char* s = 0;

    if (good && (good = !!f->f))
    {
        ret = fseek (f->f, 0, SEEK_END);
    }
    if (good && (good = (ret == 0)))
    {
        ret = ftell (f->f);
    }
    if (good && (good = (ret >= 0)))
    {
        sz = ret;
        ret = fseek (f->f, 0, SEEK_SET);
    }
    if (good && (good = (ret == 0)))
    {
        GrowTable( char, f->buf, sz/sizeof(char) );
        s = &f->buf.s[f->off];
        ret = fread (s, 1, sz, f->f);
        f->off += ret;
        s[ret] = 0;
    }

    close_FileB (f);

    if (good && (good = (ret == (long)sz)))  return s;

    return 0;
}

static
    bool
load_chunk_FileB (FileB* in)
{
    Table(char)* buf = &in->buf;
    size_t n;
    char* s;

    if (!in->f)  return false;

    Claim2( 0 ,<, buf->sz );
    n = buf->sz - 1;
    GrowTable( char, *buf, NPerChunk );
    s = &buf->s[n];

    n = fread (s, sizeof (char), NPerChunk, in->f);
    s[n] = 0;
    buf->sz -= (NPerChunk - n);
    return (n != 0);
}

static
    void
flushx_FileB (FileB* in)
{
    Table(char)* buf = &in->buf;
    Claim2( 0 ,<, buf->sz );
    Claim2( 0 ,==, buf->s[buf->sz-1] );
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

    flushx_FileB (in);
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

    flushx_FileB (in);
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
skipds_FileB (FileB* in, const char* delims)
{
    char* s;
    if (!delims)  delims = WhiteSpaceChars;
    flushx_FileB (in);

    s = in->buf.s;
    s = &s[strspn (s, delims)];

    while (!s[0]) {
        flushx_FileB (in);
        if (!load_chunk_FileB (in))  break;
        s = in->buf.s;
        s = &s[strspn (s, delims)];
    }
    in->off = IndexInTable( char, in->buf, s );
    flushx_FileB (in);
}

    char*
nextds_FileB (FileB* in, char* ret_match, const char* delims)
{
    char* s;
    if (!delims)  delims = WhiteSpaceChars;
    flushx_FileB (in);

    s = in->buf.s;
    s = &s[strcspn (s, delims)];

    while (!s[0]) {
        uint off = in->buf.sz - 1;
        if (!load_chunk_FileB (in))  break;
        s = &in->buf.s[off];
        s = &s[strcspn (s, delims)];
    }

    if (ret_match)  *ret_match = s[0];
    if (s[0])
    {
        s[0] = 0;
        ++ s;
        in->off = IndexInTable( char, in->buf, s );
    }
    else
    {
        in->off = in->buf.sz;
    }

    return (in->buf.sz == 1) ? 0 : in->buf.s;
}

    char*
nextok_FileB (FileB* in, char* ret_match, const char* delims)
{
    skipds_FileB (in, delims);
    return nextds_FileB (in, ret_match, delims);
}

    /** Inject content from a file /src/
     * at the current read position of file /in/.
     * This allows a trivial implementation of #include.
     **/
    void
inject_FileB (FileB* in, FileB* src, const char* delim)
{
    uint delim_sz = strlen (delim);

    load_FileB (src);

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
}

    bool
flusho_FileB (FileB* f)
{
    size_t n;
    if (f->off == 0)  return true;
    n = fwrite (f->buf.s, sizeof(char), f->off, f->f);
    f->buf.s[0] = 0;
    f->buf.sz = 1;
    f->off -= n;
    return (f->off == 0);
}

static inline
    bool
dump_chunk_FileB (FileB* f)
{
    if (f->off < NPerChunk)  return true;
        /* In the future, we may not want to flush all the time!*/
    return flusho_FileB (f);
}

    void
dump_uint_FileB (FileB* f, uint x)
{
    SizeUpTable( char, f->buf, f->off + 50 );
    f->off += sprintf (&f->buf.s[f->off], "%u", x);
    dump_chunk_FileB (f);
}

    void
dump_real_FileB (FileB* f, real x)
{
    SizeUpTable( char, f->buf, f->off + 50 );
    f->off += sprintf (&f->buf.s[f->off], "%g", x);
    dump_chunk_FileB (f);
}

    void
dump_char_FileB (FileB* f, char c)
{
    SizeUpTable( char, f->buf, f->off + 1 );
    f->buf.s[f->off] = c;
    f->buf.s[++f->off] = 0;
    dump_chunk_FileB (f);
}

    void
dump_cstr_FileB (FileB* f, const char* s)
{
    uint n = strlen (s);
    GrowTable( char, f->buf, n );
    memcpy (&f->buf.s[f->off], s, (n+1)*sizeof(char));
    f->off += n;
    dump_chunk_FileB (f);
}

