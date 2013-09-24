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
xget_chunk_XFileB (XFileB* xfb);
static bool
xget_chunk_fn_XFileB (XFile* xf);
static bool
oput_chunk_OFileB (OFileB* ofb);
static void
oputn_raw_byte_OFileB (OFileB* ofb, const byte* a, ujint n);

static void
close_fn_XFileB (XFile* xf);
static void
close_fn_OFileB (OFile* of);
static void
free_fn_XFileB (XFile* xf);
static void
free_fn_OFileB (OFile* of);
static bool
flush_fn_OFileB (OFile* of);

static
  void
init_FileB (FileB* fb, bool sink)
{
  fb->f = 0;
  fb->fd = -1;
  fb->good = true;
  fb->sink = sink;
  fb->byline = false;
  fb->fmt = FileB_Ascii;
  fb->pathname = dflt_AlphaTab ();
  fb->filename = dflt_AlphaTab ();
}

  void
init_XFileB (XFileB* xfb)
{
  static bool vt_initialized = false;
  static XFileCtx ctx;
  static XFileVT vt;
  init_XFile (&xfb->xf);
  xfb->xf.flushsz = BUFSIZ;
  init_FileB (&xfb->fb, false);
  xfb->xf.vt = &vt;
  xfb->xf.ctx = &ctx;
  if (!vt_initialized) {
    memset (&vt, 0, sizeof (vt));
    vt.xget_chunk_fn = xget_chunk_fn_XFileB;
    vt.close_fn = close_fn_XFileB;
    vt.free_fn = free_fn_XFileB;
    vt_initialized = true;
  }
}

  void
init_OFileB (OFileB* ofb)
{
  static bool vt_initialized = false;
  static OFileCtx ctx;
  static OFileVT vt;
  init_OFile (&ofb->of);
  ofb->of.flushsz = BUFSIZ;
  init_FileB (&ofb->fb, true);
  ofb->of.vt = &vt;
  ofb->of.ctx = &ctx;
  if (!vt_initialized) {
    memset (&vt, 0, sizeof (vt));
    vt.flush_fn = flush_fn_OFileB;
    vt.close_fn = close_fn_OFileB;
    vt.free_fn = free_fn_OFileB;
    vt_initialized = true;
  }
}

static
  void
close_FileB (FileB* f)
{
  if (f->f)
  {
    fclose (f->f);
    f->f = 0;
    f->fd = -1;
  }
}

  void
close_XFileB (XFileB* f)
{
  close_FileB (&f->fb);
  f->xf.off = 0;
  f->xf.buf.s[0] = 0;
  f->xf.buf.sz = 1;
}

  void
close_OFileB (OFileB* f)
{
  flush_OFileB (f);
  close_FileB (&f->fb);
  f->of.off = 0;
  f->of.buf.s[0] = 0;
  f->of.buf.sz = 1;
}

  void
close_fn_XFileB (XFile* xf)
{
  close_XFileB (CastUp( XFileB, xf, xf ));
}

  void
close_fn_OFileB (OFile* of)
{
  close_OFileB (CastUp( OFileB, of, of ));
}

  void
lose_XFileB (XFileB* xfb)
{
  close_XFileB (xfb);
  LoseTable( xfb->xf.buf );
  lose_AlphaTab (&xfb->fb.pathname);
  lose_AlphaTab (&xfb->fb.filename);
}

  void
lose_OFileB (OFileB* ofb)
{
  close_OFileB (ofb);
  LoseTable( ofb->of.buf );
  lose_AlphaTab (&ofb->fb.pathname);
  lose_AlphaTab (&ofb->fb.filename);
}

  void
free_fn_XFileB (XFile* xf)
{
  XFileB* xfb = CastUp( XFileB, xf, xf );
  lose_XFileB (xfb);
  free (xfb);
}

  void
free_fn_OFileB (OFile* of)
{
  OFileB* ofb = CastUp( OFileB, of, of );
  lose_OFileB (ofb);
  free (ofb);
}

static inline
  ujint
chunksz_OFileB (OFileB* ofb)
{
  (void) ofb;
  return BUFSIZ;
}

static inline
  ujint
chunksz_XFileB (XFileB* xfb)
{
  (void) xfb;
  return BUFSIZ;
}

  byte*
ensure_XFileB (XFileB* xfb, ujint n)
{
  XFile* const xf = &xfb->xf;
  ujint sz = xf->buf.sz;
  if (nullt_FileB (&xfb->fb))
  {
    Claim2( 0 ,<, sz );
    sz -= 1;
  }
  GrowTable( xf->buf, n );
  return &xf->buf.s[sz];
}

  byte*
ensure_OFileB (OFileB* ofb, ujint n)
{
  OFile* const of = &ofb->of;
  oput_chunk_OFileB (ofb);
  EnsizeTable( of->buf, of->off+n );
  return &of->buf.s[of->off];
}

  void
setfmt_XFileB (XFileB* xfb, FileB_Format fmt)
{
  XFile* const xf = &xfb->xf;
  bool nullt0, nullt1;

  nullt0 = nullt_FileB (&xfb->fb);
  xfb->fb.fmt = fmt;
  nullt1 = nullt_FileB (&xfb->fb);
  if (nullt0 != nullt1)
  {
    if (nullt0)  xf->off += 1;
    else         xf->buf.sz -= 1;
  }
}

  void
setfmt_OFileB (OFileB* ofb, FileB_Format fmt)
{
  ofb->fb.fmt = fmt;
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
set_FILE_FileB (FileB* fb, FILE* file)
{
  Claim( !fb->f );
  fb->f = file;
}

  char*
xget_XFileB (XFileB* xfb)
{
  XFile* const xf = &xfb->xf;
  bool good = true;
  long ret = -1;

  good = !!xfb->fb.f;
#ifndef _MSC_VER
  if (good)
    ret = fseek (xfb->fb.f, 0, SEEK_END);
#endif

  /* Some streams cannot be seeked.*/
  if (good && ret != 0)
  {
    errno = 0; /* Not an error.*/
    xget_XFile (xf);
  }
  else
  {
    size_t sz = 0;

    if (LegitCk( ret == 0, good, "fseek()" ))
      ret = ftell (xfb->fb.f);
    if (LegitCk( ret >= 0, good, "ftell()" ))
    {
      sz = ret;
      ret = fseek (xfb->fb.f, 0, SEEK_SET);
    }
    if (LegitCk(ret == 0, good, "fseek()" ))
    {
      GrowTable( xf->buf, sz );

      /* Note this relation!*/
      Claim2( xf->off + sz ,==, xf->buf.sz-1 );

      ret = fread (&xf->buf.s[xf->off], 1, sz, xfb->fb.f);
      if (ret >= 0)
        xf->buf.s[xf->off + ret] = '\0';
    }
    if (LegitCk( ret == (long)sz, good, "fread()" ))
    {}
  }

  if (good)
  {
    char* s = cstr_XFile (xf);
    xf->off = xf->buf.sz-1;
    return s;
  }
  return NULL;
}

  bool
xget_chunk_XFileB (XFileB* xfb)
{
  const ujint chunksz = chunksz_XFileB (xfb);
  TableT(byte)* buf = &xfb->xf.buf;
  size_t n;
  byte* s;

  if (!xfb->fb.f)  return false;

  s = ensure_XFileB (xfb, chunksz);

  if (byline_FileB (&xfb->fb))
  {
    char* line = (char*) s;
    Claim( nullt_FileB (&xfb->fb) );
    /* Don't worry about actually reading a full line here,
     * that's at a higher level.
     * We just want to avoid deadlock by stopping at a newline.
     */
    line = fgets (line, chunksz, xfb->fb.f);
    n = (line ? strlen (line) : 0);
  }
  else
  {
    n = fread (s, 1, chunksz, xfb->fb.f);
  }
  if (nullt_FileB (&xfb->fb))
    s[n] = 0;
  buf->sz -= (chunksz - n);
  return (n != 0);
}

  bool
xget_chunk_fn_XFileB (XFile* xf)
{
  return xget_chunk_XFileB (CastUp( XFileB, xf, xf ));
}

  void
flush_XFileB (XFileB* xfb)
{
  XFile* const f = &xfb->xf;
  TableT(byte)* buf = &f->buf;
  if (nullt_FileB (&xfb->fb))
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
  else if (nullt_FileB (&xfb->fb))
  {
    Claim(0);
    /* TODO - When does this even happen?*/
    buf->s[0] = 0;
    buf->sz = 1;
  }
  f->off = 0;
}


static
  bool
foput_OFileB (OFileB* ofb, const byte* a, uint n)
{
  size_t nout;
  nout = fwrite (a, 1, n, ofb->fb.f);
  return (nout == n);
}

static inline
  bool
selfcont_OFileB (OFileB* ofb)
{
  return (!ofb->fb.f);
}

static
  bool
flush1_OFileB (OFileB* ofb, const byte* a, uint n)
{
  OFile* const of = &ofb->of;
  bool good = true;
  if (selfcont_OFileB (ofb))
  {
    if (n == 0)  return true;
    GrowTable( of->buf, n );
    memcpy (&of->buf.s[of->off], a, n);
    of->off += n;
  }
  else
  {
    if (of->off > 0)
    {
      good = foput_OFileB (ofb, of->buf.s, of->off);
      if (!good)  return false;
      of->buf.sz = 1;
      of->off = 0;
    }
    if (n > 0)
    {
      good = foput_OFileB (ofb, a, n);
      if (!good)  return false;
    }
    fflush (ofb->fb.f);
  }


  if (nullt_FileB (&ofb->fb))
  {
    /* Not sure why...*/
    of->buf.s[of->off] = 0;
  }
  return true;
}

  bool
flush_OFileB (OFileB* ofb)
{
  return flush1_OFileB (ofb, 0, 0);
}

  bool
flush_fn_OFileB (OFile* of)
{
  return flush_OFileB (CastUp( OFileB, of, of ));
}

  bool
oput_chunk_OFileB (OFileB* f)
{
  if (f->of.off < f->of.flushsz)  return true;
  /* In the future, we may not want to flush all the time!*/
  /* Also, we may not wish to flush the whole buffer.*/
  return flush_OFileB (f);
}

#if 0
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
#endif

  void
oputn_raw_byte_OFileB (OFileB* ofb, const byte* a, ujint n)
{
  OFile* const of = &ofb->of;
  const ujint ntotal = of->off + n;
  if (ntotal <= allocsz_Table ((Table*) &of->buf))
  {
    memcpy (&of->buf.s[of->off], a, n);
    of->off = ntotal;

    if (of->off > of->buf.sz)
      of->buf.sz = of->off;
  }
  else if (ntotal <= 2*chunksz_OFileB (ofb))
  {
    EnsizeTable( of->buf, 2*chunksz_OFileB (ofb) );
    memcpy (&of->buf.s[of->off], a, n);
    of->off = ntotal;
  }
  else
  {
    flush1_OFileB (ofb, a, n);
  }
}

    void
oputn_byte_OFileB (OFileB* ofb, const byte* a, ujint n)
{
    if (ofb->fb.fmt == FileB_Raw)
    {
        oputn_raw_byte_OFileB (ofb, a, n);
        return;
    }
    {:for (i ; n)
        oput_uint_OFile (&ofb->of, a[i]);
        if (i+1 < n)
            oput_char_OFile (&ofb->of, ' ');
    }
}

    bool
xget_uint_XFileB (XFileB* xfb, uint* x)
{
  XFile* const xf = &xfb->xf;
  const char* s;
  if (!xfb->fb.good)  return false;
  if (nullt_FileB (&xfb->fb))
  {
    skipds_XFile (xf, 0);
    if (xf->buf.sz - xf->off < 50)
      xget_chunk_XFileB (xfb);
    s = xget_uint_cstr (x, cstr_XFile (xf));
    xfb->fb.good = !!s;
    if (!xfb->fb.good)  return false;
    xf->off = IdxElt( xf->buf.s, s );
  }
  else
  {
    union Castless {
      uint x;
      byte b[sizeof(uint)];
    } y;
    xfb->fb.good = xgetn_byte_XFileB (xfb, y.b, sizeof(uint));
    if (!xfb->fb.good)  return false;
    *x = y.x;
  }
  return true;
}

static
  bool
xgetn_raw_byte_XFileB (XFileB* xfb, byte* a, ujint n)
{
  XFile* const xf = &xfb->xf;
  Claim2( xfb->fb.fmt ,==, FileB_Raw );
  flush_XFileB (xfb);
  while (n > 0)
  {
    ujint m;
    if (xf->buf.sz == 0)
      xfb->fb.good = xget_chunk_XFileB (xfb);
    if (!xfb->fb.good)  return false;
    m = (n < xf->buf.sz ? n : xf->buf.sz);
    memcpy (a, xf->buf.s, m);
    xf->off = m;
    flush_XFileB (xfb);
    a = &a[m];
    n -= m;
  }
  return true;
}

  bool
xgetn_byte_XFileB (XFileB* xfb, byte* a, ujint n)
{
  if (xfb->fb.fmt == FileB_Raw)
    return xgetn_raw_byte_XFileB (xfb, a, n);

  while (n > 0)
  {
    uint y;
    xfb->fb.good = xget_uint_XFileB (xfb, &y);
    if (!xfb->fb.good)  return false;
    a[0] = (byte) y;
    a = &a[1];
    n -= 1;
  }
  return true;
}


