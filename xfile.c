
#include "xfile.h"
#include "alphatab.h"

  void
close_XFile (XFile* xf)
{
  if (xf->ctx && xf->ctx->vt->close_fn)
    xf->ctx->vt->close_fn (xf);
}

  void
lose_XFile (XFile* xf)
{
  LoseTable( xf->buf );
}

  void
free_XFile (XFile* xf)
{
  if (xf->ctx && xf->ctx->vt->free_fn)
    xf->ctx->vt->free_fn (xf);
}

  bool
xget_chunk_XFile (XFile* xf)
{
  uint sz = xf->buf.sz;
  if (xf->ctx && xf->ctx->vt->xget_chunk_fn)
    xf->ctx->vt->xget_chunk_fn (xf);
  return (sz < xf->buf.sz);
}

    void
flush_XFile (XFile* f)
{
    TableT(byte)* buf = &f->buf;
    Claim2( f->off ,<=, buf->sz );

    if (f->off == 0)  return;
    buf->sz = buf->sz - f->off;
    if (buf->sz > 0)
        memmove (buf->s, &buf->s[f->off], buf->sz);
    f->off = 0;
}

  void
xget_XFile (XFile* xf)
{
  bool more = true;
  while (more)
    more = xget_chunk_XFile (xf);
}

  char*
getline_XFile (XFile* in)
{
  uint ret_off;
  char* s;

  mayflush_XFile (in, May);
  ret_off = in->off;
  s = strchr (cstr_XFile (in), '\n');

  while (!s)
  {
    in->off = in->buf.sz - 1;
    if (!xget_chunk_XFile (in))  break;
    s = strchr (cstr_XFile (in), '\n');
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
getlined_XFile (XFile* xf, const char* delim)
{
  uint ret_off;
  char* s;
  uint delim_sz = strlen (delim);

  mayflush_XFile (xf, May);
  ret_off = xf->off;
  Claim2( ret_off ,<, xf->buf.sz );
  s = strstr (cstr_XFile (xf), delim);

  while (!s)
  {
    /* We only need to re-check the last /delim_sz-1/ chars,
     * which start at /buf.sz-delim_sz/ due to the NUL byte.
     */
    if (xf->off + delim_sz < xf->buf.sz)
      xf->off = xf->buf.sz - delim_sz;

    if (!xget_chunk_XFile (xf))  break;

    s = strstr (cstr_XFile (xf), delim);
  }

  if (s)
  {
    s[0] = '\0';
    s = &s[delim_sz];
    s[-1] = '\0';
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
skipds_XFile (XFile* xf, const char* delims)
{
    char* s;
    if (!delims)  delims = WhiteSpaceChars;
    mayflush_XFile (xf, May);
    s = (char*) &xf->buf.s[xf->off];
    s = &s[strspn (s, delims)];

    while (!s[0])
    {
        if (!xget_chunk_XFile (xf))  break;
        mayflush_XFile (xf, May);
        s = (char*) &xf->buf.s[xf->off];
        s = &s[strspn (s, delims)];
    }
    xf->off = IdxEltTable( xf->buf, s );
    mayflush_XFile (xf, May);
}

  char*
nextds_XFile (XFile* in, char* ret_match, const char* delims)
{
  uint ret_off;
  char* s;
  if (!delims)  delims = WhiteSpaceChars;
  mayflush_XFile (in, May);
  ret_off = in->off;
  Claim2( ret_off ,<, in->buf.sz );
  s = (char*) &in->buf.s[ret_off];
  s = &s[strcspn (s, delims)];

  while (!s[0])
  {
    uint off = in->buf.sz - 1;
    if (!xget_chunk_XFile (in))  break;
    s = (char*) &in->buf.s[off];
    s = &s[strcspn (s, delims)];
  }

  if (ret_match)  *ret_match = s[0];
  if (s[0])
  {
    //Claim2( IdxElt( in->buf.s, s ) ,<, in->buf.sz );
    s[0] = 0;
    in->off = IdxElt( in->buf.s, s );
    if (in->off + 1 < in->buf.sz)
      in->off += 1;
    Claim2( in->off ,<, in->buf.sz );
  }
  else
  {
    in->off = in->buf.sz - 1;
  }

  Claim( (ret_off + 1 != in->buf.sz) || !in->buf.s[ret_off]);
  return (ret_off + 1 == in->buf.sz) ? 0 : (char*) &in->buf.s[ret_off];
}

    char*
tods_XFile (XFile* xf, const char* delims)
{
    char c;
    ujint dsoff;
    ujint off;
    off = xf->off;
    nextds_XFile (xf, &c, delims);

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
nextok_XFile (XFile* xf, char* ret_match, const char* delims)
{
    skipds_XFile (xf, delims);
    return nextds_XFile (xf, ret_match, delims);
}

/** Inject content from a file /src/
 * at the current read position of file /in/.
 * This allows a trivial implementation of #include.
 **/
    void
inject_XFile (XFile* in, XFile* src, const char* delim)
{
    uint delim_sz = strlen (delim);
    const ujint sz = in->buf.sz - in->off;

    xget_XFile (src);
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
skipto_XFile (XFile* xf, const char* pos)
{
    xf->off = IdxElt( xf->buf.s, pos );
}

  bool
skiplined_XFile (XFile* xf, const char* delim)
{
  char* s;
  uint delim_sz = strlen (delim);

  mayflush_XFile (xf, May);
  s = strstr (cstr_XFile (xf), delim);

  while (!s)
  {
    /* We only need to re-check the last /delim_sz-1/ chars,
     * which start at /buf.sz-delim_sz/ due to the NUL byte.
     */
    if (xf->off + delim_sz < xf->buf.sz)
      xf->off = xf->buf.sz - delim_sz;

    mayflush_XFile (xf, May);
    if (!xget_chunk_XFile (xf))  break;

    s = strstr (cstr_XFile (xf), delim);
  }

  if (s)
  {
    s = &s[delim_sz];
    xf->off = IdxElt( xf->buf.s, s );
  }
  else
  {
    xf->off = xf->buf.sz - 1;
  }

  mayflush_XFile (xf, May);
  return !!s;
}

  bool
skip_cstr_XFile (XFile* xf, const char* pfx)
{
  uint n;
  if (!pfx)  return false;
  n = strlen (pfx);
  while (xf->off + n > xf->buf.sz)
  {
    if (!xget_chunk_XFile (xf))
      return false;
  }
  if (0 == strncmp (pfx, ccstr_of_XFile (xf), n))
  {
    xf->off += n;
    return true;
  }
  return false;
}

  void
olay_txt_XFile (XFile* olay, XFile* xf, uint off)
{
  init_XFile (olay);
  olay->buf.s = &xf->buf.s[off];
  if (xf->off + 1 == xf->buf.sz)
    olay->buf.sz = xf->buf.sz - off;
  else
    olay->buf.sz = xf->off - off;
}

  bool
getlined_olay_XFile (XFile* olay, XFile* xf, const char* delim)
{
  char* s = getlined_XFile (xf, delim);
  if (!s)  return false;
  olay_txt_XFile (olay, xf, IdxEltTable( xf->buf, s ));
  return true;
}

  bool
nextds_olay_XFile (XFile* olay, XFile* xf, char* ret_match, const char* delims)
{
  char* s = nextds_XFile (xf, ret_match, delims);
  if (!s)  return false;
  olay_txt_XFile (olay, xf, IdxEltTable( xf->buf, s ));
  return true;
}

  bool
xget_int_XFile (XFile* xf, int* x)
{
  const char* s;
  skipds_XFile (xf, WhiteSpaceChars);
  tods_XFile (xf, WhiteSpaceChars);
  s = xget_int_cstr (x, (char*)&xf->buf.s[xf->off]);
  if (!s)  return false;
  xf->off = IdxElt( xf->buf.s, s );
  return true;
}

  bool
xget_uint_XFile (XFile* xf, uint* x)
{
  const char* s;
  skipds_XFile (xf, WhiteSpaceChars);
  tods_XFile (xf, WhiteSpaceChars);
  s = xget_uint_cstr (x, (char*)&xf->buf.s[xf->off]);
  if (!s)  return false;
  xf->off = IdxElt( xf->buf.s, s );
  return true;
}

  bool
xget_ujint_XFile (XFile* xf, ujint* x)
{
  const char* s;
  skipds_XFile (xf, WhiteSpaceChars);
  tods_XFile (xf, WhiteSpaceChars);
  s = xget_ujint_cstr (x, (char*)&xf->buf.s[xf->off]);
  if (!s)  return false;
  xf->off = IdxElt( xf->buf.s, s );
  return true;
}

  bool
xget_real_XFile (XFile* xf, real* x)
{
  const char* s;
  skipds_XFile (xf, WhiteSpaceChars);
  tods_XFile (xf, WhiteSpaceChars);
  s = xget_real_cstr (x, (char*)&xf->buf.s[xf->off]);
  if (!s)  return false;
  xf->off = IdxElt( xf->buf.s, s );
  return true;
}

  bool
xget_char_XFile (XFile* xf, char* c)
{
  if (xf->off + 1 == xf->buf.sz)
  {
    if (!xget_chunk_XFile (xf))
      return false;
  }
  *c = *cstr_XFile (xf);
  xf->off += sizeof(char);
  return true;
}

