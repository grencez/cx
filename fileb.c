/**
 * \file fileb.c
 * Simple and advanced file I/O and parsing.
 **/
#include "fileb.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

static bool
xget_chunk_FileB (FileB* f);
static bool
xget_chunk_XFileB (XFileB* xf);
static inline bool
oput_chunk_FileB (FileB* f);
static void
op_FileB (XOFileB* xo, FileB_Op op, FileBOpArg* arg);
static void
oputn_raw_byte_FileB (FileB* f, const byte* a, ujint n);

    void
lose_XOFileB (XOFileB* xo)
{
    LoseTable( xo->buf );
}
void lose_OFileB (OFileB* of) { lose_XOFileB (of); }
void lose_XFileB (OFileB* xf) { lose_XOFileB (xf); }

    FileB
dflt_FileB ()
{
    FileB fb;
    fb.xo = dflt_XOFileB ();
    fb.xo.flushsz = BUFSIZ;
    fb.xo.op = op_FileB;
    fb.f = 0;
    fb.fd = -1;
    fb.good = true;
    fb.sink = false;
    fb.byline = false;
    fb.fmt = FileB_Ascii;
    fb.pathname = dflt_AlphaTab ();
    fb.filename = dflt_AlphaTab ();
    return fb;
}
    void
init_FileB (FileB* f)
{
    *f = dflt_FileB ();
}

    void
close_FileB (FileB* f)
{
    if (f->sink)  flusho_FileB (f);
    if (f->f)
    {
        fclose (f->f);
        f->f = 0;
        f->fd = -1;
    }
    f->xo.off = 0;
    f->xo.buf.s[0] = 0;
    f->xo.buf.sz = 1;
}

    void
lose_FileB (FileB* f)
{
    close_FileB (f);
    LoseTable( f->xo.buf );
    lose_AlphaTab (&f->pathname);
    lose_AlphaTab (&f->filename);
}

    void
seto_FileB (FileB* f, bool sink)
{
    Claim( !f->f );
    f->sink = sink;
}

static inline
    ujint
chunksz_FileB (FileB* f)
{
    (void) f;
    return BUFSIZ;
}

    byte*
ensure_FileB (FileB* f, ujint n)
{
    XFileB* const xof = &f->xo;
    ujint sz;
    if (f->sink)
    {
        oput_chunk_FileB (f);
        EnsizeTable( xof->buf, xof->off+n );
        return &xof->buf.s[xof->off];
    }

    sz = xof->buf.sz;
    if (nullt_FileB (f))
    {
        Claim2( 0 ,<, sz );
        sz -= 1;
    }
    GrowTable( xof->buf, n );
    return &xof->buf.s[sz];
}

    void
setfmt_FileB (FileB* f, FileB_Format fmt)
{
    XOFileB* const xof = &f->xo;
    bool nullt0, nullt1;
    if (f->sink)
    {
        f->fmt = fmt;
        return;
    }
    nullt0 = nullt_FileB (f);
    f->fmt = fmt;
    nullt1 = nullt_FileB (f);
    if (nullt0 != nullt1)
    {
        if (nullt0)  xof->off += 1;
        else         xof->buf.sz -= 1;
    }
}

static
    uint
pathname_sz (const char* path)
{
    const char* s;
    s = strrchr (path, '/');
    if (!s)  return 0;
    return 1 + IdxElt( path, s );
}

static
    bool
absolute_path (const char* path)
{
    return path[0] == '/';
}

    bool
open_FileB (FileB* f, const char* pathname, const char* filename)
{
    uint pflen = pathname_sz (filename);
    uint flen = strlen (filename) - pflen;
    uint plen = (pathname ? strlen (pathname) : 0);
    char* s;

    SizeTable( f->filename, flen+1 );
    memcpy (f->filename.s, &filename[pflen], (flen+1)*sizeof(char));

    if (pflen > 0 && absolute_path (filename))
        plen = 0;

    SizeTable( f->pathname, plen+1+pflen+flen+1 );

    s = f->pathname.s;
    if (plen > 0)
    {
        memcpy (s, pathname, plen*sizeof(char));
        s = &s[plen];
        s[0] = '/';
        s = &s[1];
    }
    memcpy (s, filename, (pflen+flen+1)*sizeof(char));

    f->f = fopen (f->pathname.s, (f->sink ? "wb" : "rb"));

    plen += pflen;
    f->pathname.s[plen] = 0;
    SizeTable( f->pathname, plen+1 );

    return !!f->f;
}

    bool
openfd_FileB (FileB* fb, fd_t fd)
{
    Claim( !fb->f );
    fb->fd = fd;
    fb->f = fdopen_sysCx (fd, (fb->sink ? "wb" : "rb"));
    return !!fb->f;
}

    void
set_FILE_FileB (FileB* f, FILE* file)
{
    Claim( !f->f );
    f->f = file;
}

    char*
xget_FileB (FileB* f)
{
    XFileB* const xf = &f->xo;
    bool good = true;
    long ret = -1;

    good = !!f->f;
#ifndef _MSC_VER
    if (good)
        ret = fseek (f->f, 0, SEEK_END);
#endif

    /* Some streams cannot be seeked.*/
    if (good && ret != 0)
    {
        errno = 0; /* Not an error.*/
        xget_XFileB (xf);
    }
    else
    {
        size_t sz = 0;
        BInit();

        BCasc( ret == 0, good, "fseek()" );

        ret = ftell (f->f);
        BCasc( ret >= 0, good, "ftell()" );

        sz = ret;
        ret = fseek (f->f, 0, SEEK_SET);
        BCasc( ret == 0, good, "fseek()" );

        GrowTable( xf->buf, sz );

            /* Note this relation!*/
        Claim2( xf->off + sz ,==, xf->buf.sz-1 );

        ret = fread (&xf->buf.s[xf->off], 1, sz, f->f);
        if (ret >= 0)
            xf->buf.s[xf->off + ret] = '\0';

        BCasc( ret == (long)sz, good, "fread()" );
        BLose();
    }

    if (good)
    {
        char* s = cstr_XFileB (xf);
        xf->off = xf->buf.sz-1;
        return s;
    }
    return NULL;
}

    bool
xget_chunk_FileB (FileB* f)
{
    const ujint chunksz = chunksz_FileB (f);
    TableT(byte)* buf = &f->xo.buf;
    size_t n;
    byte* s;

    if (!f->f)  return false;

    s = ensure_FileB (f, chunksz);

    if (byline_FileB (f))
    {
        char* line = (char*) s;
        Claim( nullt_FileB (f) );
            /* Don't worry about actually reading a full line here,
             * that's at a higher level.
             * We just want to avoid deadlock by stopping at a newline.
             */
        line = fgets (line, chunksz, f->f);
        n = (line ? strlen (line) : 0);
    }
    else
    {
        n = fread (s, 1, chunksz, f->f);
    }
    if (nullt_FileB (f))
        s[n] = 0;
    buf->sz -= (chunksz - n);
    return (n != 0);
}

    bool
xget_chunk_XFileB (XFileB* xf)
{
    uint sz = xf->buf.sz;
    if (xf->op)
        xf->op (xf, FileB_XGetChunk, 0);
    return (sz < xf->buf.sz);
}

    void
close_XFileB (XFileB* xf)
{
    xf->op (xf, FileB_Close, 0);
}

    void
close_OFileB (OFileB* of)
{
    of->op (of, FileB_Close, 0);
}

    void
flushx_FileB (FileB* fb)
{
    OFileB* const f = &fb->xo;
    TableT(byte)* buf = &f->buf;
    if (nullt_FileB (fb))
    {
        Claim2( 0 ,<, buf->sz );
        Claim2( 0 ,==, buf->s[buf->sz-1] );
        Claim2( f->off ,<, buf->sz );
    }
    else
    {
        Claim2( f->off ,<=, buf->sz );
    }
    if (f->off == 0)  return;
    buf->sz = buf->sz - f->off;
    if (buf->sz > 0)
    {
        memmove (buf->s, &buf->s[f->off], buf->sz);
    }
    else if (nullt_FileB (fb))
    {
        Claim(0);
            /* TODO - When does this even happen?*/
        buf->s[0] = 0;
        buf->sz = 1;
    }
    f->off = 0;
}

    void
flush_XFileB (XFileB* f)
{
    TableT(byte)* buf = &f->buf;
    Claim2( f->off ,<=, buf->sz );

    if (f->off == 0)  return;
    buf->sz = buf->sz - f->off;
    if (buf->sz > 0)
        memmove (buf->s, &buf->s[f->off], buf->sz);
    f->off = 0;
}

    char*
getline_XFileB (XFileB* in)
{
    uint ret_off;
    char* s;

    mayflush_XFileB (in, May);
    ret_off = in->off;
    s = strchr (cstr_XFileB (in), '\n');

    while (!s)
    {
        in->off = in->buf.sz - 1;
        if (!xget_chunk_XFileB (in))  break;
        s = strchr (cstr_XFileB (in), '\n');
    }

    if (s)
    {
        s[0] = '\0';
        in->off = IdxElt( in->buf.s, s );
        if (in->off > ret_off && s[-1] == '\r')
            s[-1] = '\0';
        if (in->off + 1 < in->buf.sz)
            in->off += 1;
    }
    else
    {
        in->off = in->buf.sz - 1;
    }

    return (ret_off + 1 == in->buf.sz) ? 0 : (char*) &in->buf.s[ret_off];
}

    char*
getlined_XFileB (XFileB* xf, const char* delim)
{
    uint ret_off;
    char* s;
    uint delim_sz = strlen (delim);

    mayflush_XFileB (xf, May);
    ret_off = xf->off;
    Claim2( ret_off ,<, xf->buf.sz );
    s = strstr (cstr_XFileB (xf), delim);

    while (!s)
    {
            /* We only need to re-check the last /delim_sz-1/ chars,
             * which start at /buf.sz-delim_sz/ due to the NUL byte.
             */
        if (xf->off + delim_sz < xf->buf.sz)
            xf->off = xf->buf.sz - delim_sz;

        if (!xget_chunk_XFileB (xf))  break;

        s = strstr (cstr_XFileB (xf), delim);
    }

    if (s)
    {
        s[0] = '\0';
        s = &s[delim_sz];
        xf->off = IdxElt( xf->buf.s, s );
        Claim2( xf->off ,<, xf->buf.sz );
    }
    else
    {
        xf->off = xf->buf.sz - 1;
    }

    return (ret_off + 1 == xf->buf.sz) ? 0 : (char*) &xf->buf.s[ret_off];
}

    void
skipds_XFileB (XFileB* xf, const char* delims)
{
    char* s;
    if (!delims)  delims = WhiteSpaceChars;
    mayflush_XFileB (xf, May);
    s = (char*) &xf->buf.s[xf->off];
    s = &s[strspn (s, delims)];

    while (!s[0])
    {
        if (!xget_chunk_XFileB (xf))  break;
        mayflush_XFileB (xf, May);
        s = (char*) &xf->buf.s[xf->off];
        s = &s[strspn (s, delims)];
    }
    xf->off = IdxEltTable( xf->buf, s );
    mayflush_XFileB (xf, May);
}

    char*
nextds_XFileB (XFileB* in, char* ret_match, const char* delims)
{
    uint ret_off;
    char* s;
    if (!delims)  delims = WhiteSpaceChars;
    mayflush_XFileB (in, May);
    ret_off = in->off;
    Claim2( ret_off ,<, in->buf.sz );
    s = (char*) &in->buf.s[ret_off];
    s = &s[strcspn (s, delims)];

    while (!s[0])
    {
        uint off = in->buf.sz - 1;
        if (!xget_chunk_XFileB (in))  break;
        s = (char*) &in->buf.s[off];
        s = &s[strcspn (s, delims)];
    }

    if (ret_match)  *ret_match = s[0];
    if (s[0])
    {
        s[0] = 0;
        in->off = IdxElt( in->buf.s, s );
        if (in->off + 1 < in->buf.sz)
            in->off += 1;
    }
    else
    {
        in->off = in->buf.sz - 1;
    }

    return (ret_off + 1 == in->buf.sz) ? 0 : (char*) &in->buf.s[ret_off];
}

    char*
tods_XFileB (XFileB* xf, const char* delims)
{
    char c;
    ujint dsoff;
    ujint off;
    off = xf->off;
    nextds_XFileB (xf, &c, delims);

    dsoff = xf->off;
    if (c)
    {
        -- dsoff;
        xf->buf.s[dsoff] = c;
    }
    xf->off = off;
    return (char*) &xf->buf.s[dsoff];
}

    char*
nextok_XFileB (XFileB* xf, char* ret_match, const char* delims)
{
    skipds_XFileB (xf, delims);
    return nextds_XFileB (xf, ret_match, delims);
}

/** Inject content from a file /src/
 * at the current read position of file /in/.
 * This allows a trivial implementation of #include.
 **/
    void
inject_XFileB (XFileB* in, XFileB* src, const char* delim)
{
    uint delim_sz = strlen (delim);
    const ujint sz = in->buf.sz - in->off;

    xget_XFileB (src);
    Claim2( src->buf.sz ,>, 0 );

    GrowTable( in->buf, src->buf.sz-1 + delim_sz );
    /* Make room for injection.*/
    memmove (&in->buf.s[in->off + src->buf.sz-1 + delim_sz],
             &in->buf.s[in->off],
             sz * sizeof (char));
    /* Inject file contents, assume src->buf.sz is strlen!*/
    memcpy (&in->buf.s[in->off],
            src->buf.s,
            (src->buf.sz-1) * sizeof (char));

    /* Add the delimiter at the end.*/
    if (delim_sz > 0)
        memcpy (&in->buf.s[in->off + src->buf.sz-1],
                delim,
                delim_sz * sizeof (char));
}

    void
skipto_XFileB (XFileB* xf, const char* pos)
{
    xf->off = IdxElt( xf->buf.s, pos );
}

static
    bool
foput_FileB (FileB* f, const byte* a, uint n)
{
    size_t nout;
    nout = fwrite (a, 1, n, f->f);
    return (nout == n);
}

static inline
    bool
selfcont_FileB (FileB* f)
{
    return (!f->f);
}

static
    bool
flusho1_FileB (FileB* fb, const byte* a, uint n)
{
    OFileB* const f = &fb->xo;
    bool good = true;
    if (selfcont_FileB (fb))
    {
        if (n == 0)  return true;
        GrowTable( f->buf, n );
        memcpy (&f->buf.s[f->off], a, n);
        f->off += n;
    }
    else
    {
        if (f->off > 0)
        {
            good = foput_FileB (fb, f->buf.s, f->off);
            if (!good)  return false;
            f->buf.sz = 1;
            f->off = 0;
        }
        if (n > 0)
        {
            good = foput_FileB (fb, a, n);
            if (!good)  return false;
        }
    }


    if (nullt_FileB (fb))
    {
            /* Not sure why...*/
        f->buf.s[f->off] = 0;
    }
    return true;
}

    bool
flusho_FileB (FileB* f)
{
    return flusho1_FileB (f, 0, 0);
}

    bool
oput_chunk_FileB (FileB* f)
{
    if (f->xo.off < f->xo.flushsz)  return true;
        /* In the future, we may not want to flush all the time!*/
        /* Also, we may not wish to flush the whole buffer.*/
    return flusho_FileB (f);
}

    void
flush_OFileB (OFileB* of)
{
        /* In the future, we may not want to flush all the time!*/
        /* Also, we may not wish to flush the whole buffer.*/
    if (of->op)
    {
        of->op (of, FileB_FlushO, 0);
    }
    else
    {
        Claim2( of->flushsz ,==, 0 );
        if (of->off > 0)
        {
            of->off = 0;
            of->buf.sz = 1;
            of->buf.s[0] = 0;
        }
    }
}


    void
mayflush_OFileB (OFileB* of)
{
    if (of->flushsz > 0 && of->off > of->flushsz)
        flush_OFileB (of);
}

    void
op_FileB (XOFileB* xo, FileB_Op op, FileBOpArg* arg)
{
    FileB* fb = CastUp( FileB, xo, xo );
    (void) arg;
    switch (op)
    {
        case FileB_XGetChunk:
            xget_chunk_FileB (fb);
            break;
        case FileB_OPutChunk:
            oput_chunk_FileB (fb);
            break;
        case FileB_FlushO:
            flusho_FileB (fb);
            break;
        case FileB_Close:
            close_FileB (fb);
            break;
        case FileB_NOps:
            Claim(0);
            break;
    }
}

    void
oput_int_OFileB (OFileB* f, int x)
{
    EnsizeTable( f->buf, f->off + 50 );
    f->off += sprintf (cstr_OFileB (f), "%i", x);
    mayflush_OFileB (f);
}

    void
oput_uint_OFileB (OFileB* f, uint x)
{
    EnsizeTable( f->buf, f->off + 50 );
    f->off += sprintf (cstr_OFileB (f), "%u", x);
    mayflush_OFileB (f);
}

    void
oput_ujint_OFileB (OFileB* f, ujint x)
{
    EnsizeTable( f->buf, f->off + 50 );
    f->off += sprintf (cstr_OFileB (f), "%lu", x);
    mayflush_OFileB (f);
}

    void
oput_real_OFileB (OFileB* f, real x)
{
    EnsizeTable( f->buf, f->off + 50 );
    f->off += sprintf (cstr_OFileB (f), "%.16e", x);
    mayflush_OFileB (f);
}

    void
oput_char_OFileB (OFileB* f, char c)
{
    EnsizeTable( f->buf, f->off + 2 );
    f->buf.s[f->off] = c;
    f->buf.s[++f->off] = 0;
    mayflush_OFileB (f);
}

    void
oput_AlphaTab (OFileB* of, const AlphaTab* t)
{
    ujint n = t->sz;
    if (n == 0)  return;
    if (!t->s[n-1])  -- n;
    GrowTable( of->buf, n*sizeof(char) );
    memcpy (&of->buf.s[of->off], t->s, n*sizeof(char));
    of->buf.s[of->buf.sz-1] = 0;
    of->off += n;
    mayflush_OFileB (of);
}

    void
vprintf_OFileB (OFileB* f, const char* fmt, va_list args)
{
    ujint sz = 2048;  /* Not good :( */
    int iret = 0;

    EnsizeTable( f->buf, f->off + sz );
    iret = vsprintf ((char*) &f->buf.s[f->off], fmt, args);
    Claim2( iret ,>=, 0 );
    Claim2( (uint) iret ,<=, sz );
    f->off += iret;
    mayflush_OFileB (f);
}

    void
printf_OFileB (OFileB* f, const char* fmt, ...)
{
    va_list args;
    va_start (args, fmt);
    vprintf_OFileB (f, fmt, args);
    va_end (args);
}

    void
oputn_raw_byte_FileB (FileB* f, const byte* a, ujint n)
{
    OFileB* const of = &f->xo;
    const ujint ntotal = of->off + n;
    if (ntotal <= allocsz_Table ((Table*) &of->buf))
    {
        memcpy (&of->buf.s[of->off], a, n);
        of->off = ntotal;

        if (of->off > of->buf.sz)
            of->buf.sz = of->off;
    }
    else if (ntotal <= 2*chunksz_FileB (f))
    {
        EnsizeTable( of->buf, 2*chunksz_FileB (f) );
        memcpy (&of->buf.s[of->off], a, n);
        of->off = ntotal;
    }
    else
    {
        flusho1_FileB (f, a, n);
    }
}

    void
oputn_byte_FileB (FileB* f, const byte* a, ujint n)
{
    if (f->fmt == FileB_Raw)
    {
        oputn_raw_byte_FileB (f, a, n);
        return;
    }
    { BLoop( i, n )
        oput_uint_OFileB (&f->xo, a[i]);
        if (i+1 < n)
            oput_char_OFileB (&f->xo, ' ');
    } BLose()
}

    void
oputn_char_OFileB (OFileB* of, const char* a, ujint n)
{
    GrowTable( of->buf, n );
    memcpy (&of->buf.s[of->off], a, (n+1)*sizeof(char));
    of->off += n;
    mayflush_OFileB (of);
}

    void
xget_XFileB (XFileB* xf)
{
    bool more = true;
    while (more)
        more = xget_chunk_XFileB (xf);
}

    char*
xget_uint_cstr (uint* ret, const char* in)
{
    unsigned long v;
    char* out = 0;

    assert (ret);
    assert (in);
    v = strtoul (in, &out, 10);

    if (out == in)  out = 0;
    if (out)
    {
        *ret = (uint) v;
        if (*ret != v)  out = 0;
    }
    return out;
}

    char*
xget_int_cstr (int* ret, const char* in)
{
    long v;
    char* out = 0;

    assert (ret);
    assert (in);
    v = strtol (in, &out, 10);

    if (out == in)  out = 0;
    if (out)
    {
        *ret = (int) v;
        if (*ret != v)  out = 0;
    }
    return out;
}

    char*
xget_real_cstr (real* ret, const char* in)
{
    double v;
    char* out = 0;

    assert (ret);
    assert (in);
    v = strtod (in, &out);

    if (out == in)  out = 0;
    if (out)  *ret = (real) v;
    return out;
}

    bool
xget_uint_FileB (FileB* f, uint* x)
{
    XFileB* const xf = &f->xo;
    const char* s;
    if (!f->good)  return false;
    if (nullt_FileB (f))
    {
        skipds_XFileB (xf, 0);
        if (xf->buf.sz - xf->off < 50)
            xget_chunk_FileB (f);
        s = xget_uint_cstr (x, cstr_XFileB (xf));
        f->good = !!s;
        if (!f->good)  return false;
        xf->off = IdxElt( xf->buf.s, s );
    }
    else
    {
        union Castless {
            uint x;
            byte b[sizeof(uint)];
        } y;
        f->good = xgetn_byte_FileB (f, y.b, sizeof(uint));
        if (!f->good)  return false;
        *x = y.x;
    }
    return true;
}

    bool
xget_char_XFileB (XFileB* xf, char* c)
{
    if (xf->off + 1 == xf->buf.sz)
    {
        if (!xget_chunk_XFileB (xf))
            return false;
    }
    *c = *cstr_XFileB (xf);
    xf->off += sizeof(char);
    return true;
}

    bool
xget_int_XFileB (XFileB* xf, int* x)
{
    const char* s;
    skipds_XFileB (xf, WhiteSpaceChars);
    tods_XFileB (xf, WhiteSpaceChars);
    s = xget_int_cstr (x, (char*)&xf->buf.s[xf->off]);
    if (!s)  return false;
    xf->off = IdxElt( xf->buf.s, s );
    return true;
}

    bool
xget_real_XFileB (XFileB* xf, real* x)
{
    const char* s;
    skipds_XFileB (xf, WhiteSpaceChars);
    tods_XFileB (xf, WhiteSpaceChars);
    s = xget_real_cstr (x, (char*)&xf->buf.s[xf->off]);
    if (!s)  return false;
    xf->off = IdxElt( xf->buf.s, s );
    return true;
}

static
    bool
xgetn_raw_byte_FileB (FileB* f, byte* a, ujint n)
{
    XFileB* const xf = &f->xo;
    Claim2( f->fmt ,==, FileB_Raw );
    flushx_FileB (f);
    while (n > 0)
    {
        ujint m;
        if (xf->buf.sz == 0)
            f->good = xget_chunk_FileB (f);
        if (!f->good)  return false;
        m = (n < xf->buf.sz ? n : xf->buf.sz);
        memcpy (a, xf->buf.s, m);
        xf->off = m;
        flushx_FileB (f);
        a = &a[m];
        n -= m;
    }
    return true;
}

    bool
xgetn_byte_FileB (FileB* f, byte* a, ujint n)
{
    if (f->fmt == FileB_Raw)
        return xgetn_raw_byte_FileB (f, a, n);

    while (n > 0)
    {
        uint y;
        f->good = xget_uint_FileB (f, &y);
        if (!f->good)  return false;
        a[0] = (byte) y;
        a = &a[1];
        n -= 1;
    }
    return true;
}

    Trit
swapped_AlphaTab (const AlphaTab* a, const AlphaTab* b)
{
    ujint na = a->sz;
    ujint nb = b->sz;
    int ret;
    if (na > 0 && !a->s[na-1])  --na;
    if (nb > 0 && !b->s[nb-1])  --nb;

    if (na <= nb)
    {
        ret = memcmp (a->s, b->s, na * sizeof(char));
        if (ret == 0 && na < nb)
            ret = -1;

    }
    else
    {
        ret = memcmp (a->s, b->s, nb * sizeof(char));
        if (ret == 0)
            ret = 1;
    }
    return (ret < 0 ? Nil : (ret > 0 ? Yes : May));
}

