
#include "lsp.h"
#include "alphatab.h"

static
  Trit
swapped_MemLoc (const void* a, const void* b)
{
  if ((size_t) a < (size_t) b) {
    return Nil;
  }
  if ((size_t) a == (size_t) b) {
    return May;
  }
  return Yes;
}

  LspCtx*
make_LspCtx ()
{
  LspCtx* ctx = AllocT( LspCtx, 1 );
  InitAssocia( LspVT*, LspKind*, ctx->kindmap, swapped_MemLoc );
  ctx->nil.base.kind = 0;
  ctx->nil.car = 0;
  ctx->nil.cdr = 0;
  return ctx;
}

  void
free_LspCtx (LspCtx* ctx)
{

  for (Assoc* assoc = beg_Associa (&ctx->kindmap);
       assoc;
       assoc = next_Assoc (assoc))
  {
    LspKind* kind = *(LspKind**) val_of_Assoc (assoc);
    free_LspKind (kind);
  }
  lose_Associa (&ctx->kindmap);
  free (ctx);
}

  LspCtx*
ctx_of_Lsp (const Lsp a)
{
  if (!a->kind)
    return CastUp( LspCtx, nil, a );
  return a->kind->ctx;
}

  LspKind*
ensure_kind_LspCtx (LspCtx* ctx, const LspVT* vt)
{
  bool added = false;
  Assoc* assoc =
    ensure1_Associa (&ctx->kindmap, vt, &added);
  if (added) {
    LspKind* kind = make_LspKind (vt);
    kind->ctx = ctx;
    val_fo_Assoc (assoc, &kind);
  }
  return *(LspKind**) val_of_Assoc (assoc);
}

/** Easy make function for LspKind.*/
  LspKind*
make_LspKind (const LspVT* vt)
{
  LspKind* kind = AllocT( LspKind, 1 );
  kind->cells = dflt1_LgTable (vt->size);
  kind->vt = vt;
  return kind;
}

  Lsp
take_LspKind (LspKind* kind)
{
  void* el = take_LgTable (&kind->cells);
  Lsp sp = (Lsp) ((size_t) el + kind->vt->base_offset);
  sp->kind = kind;
  return sp;
}

static const LspVT* vt_ConsLsp ();
static const LspVT* vt_StringLsp ();
static const LspVT* vt_ConstStringLsp ();
static const LspVT* vt_NatLsp ();
static const LspVT* vt_IntLsp ();

  void
free_LspKind (LspKind* kind)
{
  for (ujint i = begidx_LgTable (&kind->cells);
       i != Max_ujint;
       i = nextidx_LgTable (&kind->cells, i))
  {
    void* el = elt_LgTable (&kind->cells, i);
    Lsp sp = (Lsp) ((size_t) el + kind->vt->base_offset);
    lose_Lsp (sp);
  }
  lose_LgTable (&kind->cells);

  free (kind);
}

  ConsLsp
cons_Lsp (Lsp car, ConsLsp cdr)
{
  LspCtx* ctx = ctx_of_Lsp (car);
  LspKind* kind = ensure_kind_LspCtx (ctx, vt_ConsLsp ());
  Lsp base = take_LspKind (kind);
  ConsLsp cons = CastUp( ConsLspBase, base, base );

  Claim2( ctx ,==, ctx_of_Lsp (&cdr->base) );

  cons->car = car;
  cons->cdr = cdr;
  return cons;
}

  Lsp
list2_Lsp (Lsp a, Lsp b)
{
  LspCtx* ctx = ctx_of_Lsp (a);
  ConsLsp cons = &ctx->nil;
  cons = cons_Lsp (b, cons);
  cons = cons_Lsp (a, cons);
  return &cons->base;
}

  Lsp
list3_Lsp (Lsp a, Lsp b, Lsp c)
{
  LspCtx* ctx = ctx_of_Lsp (a);
  ConsLsp cons = &ctx->nil;
  cons = cons_Lsp (c, cons);
  cons = cons_Lsp (b, cons);
  cons = cons_Lsp (a, cons);
  return &cons->base;
}

  Lsp
list4_Lsp (Lsp a, Lsp b, Lsp c, Lsp d)
{
  LspCtx* ctx = ctx_of_Lsp (a);
  ConsLsp cons = &ctx->nil;
  cons = cons_Lsp (d, cons);
  cons = cons_Lsp (c, cons);
  cons = cons_Lsp (b, cons);
  cons = cons_Lsp (a, cons);
  return &cons->base;
}

  const LspVT*
vt_ConsLsp ()
{
  static bool vt_initialized = false;
  static LspVT vt;
  if (!vt_initialized) {
    vt_initialized = true;
    memset (&vt, 0, sizeof (vt));
    vt.base_offset = offsetof( ConsLspBase, base );
    vt.size = sizeof(ConsLspBase);
  }
  return &vt;
}

  StringLsp
make_StringLsp (LspCtx* ctx, const char* s)
{
  LspKind* kind = ensure_kind_LspCtx (ctx, vt_StringLsp ());
  StringLsp sp = to_StringLsp (take_LspKind (kind));
  sp->s = dup_cstr (s);
  return sp;
}

static void lose_StringLsp (Lsp base)
{
  StringLsp sp = to_StringLsp (base);
  free (sp->s);
}

  const LspVT*
vt_StringLsp ()
{
  static bool vt_initialized = false;
  static LspVT vt;
  if (!vt_initialized) {
    vt_initialized = true;
    memset (&vt, 0, sizeof (vt));
    vt.base_offset = offsetof( StringLspBase, base );
    vt.size = sizeof(StringLspBase);
    vt.lose_fn = lose_StringLsp;
  }
  return &vt;
}

  ConstStringLsp
make_ConstStringLsp (LspCtx* ctx, const char* s)
{
  LspKind* kind = ensure_kind_LspCtx (ctx, vt_ConstStringLsp ());
  ConstStringLsp sp = to_ConstStringLsp (take_LspKind (kind));
  sp->s = s;
  return sp;
}

  const LspVT*
vt_ConstStringLsp ()
{
  static bool vt_initialized = false;
  static LspVT vt;
  if (!vt_initialized) {
    vt_initialized = true;
    memset (&vt, 0, sizeof (vt));
    vt.base_offset = offsetof( ConstStringLspBase, base );
    vt.size = sizeof(ConstStringLspBase);
  }
  return &vt;
}

  NatLsp
make_NatLsp (LspCtx* ctx, uint u)
{
  LspKind* kind = ensure_kind_LspCtx (ctx, vt_NatLsp ());
  NatLsp sp = to_NatLsp (take_LspKind (kind));
  sp->u = u;
  return sp;
}

  const LspVT*
vt_NatLsp ()
{
  static bool vt_initialized = false;
  static LspVT vt;
  if (!vt_initialized) {
    vt_initialized = true;
    memset (&vt, 0, sizeof (vt));
    vt.base_offset = offsetof( NatLspBase, base );
    vt.size = sizeof(NatLspBase);
  }
  return &vt;
}

  IntLsp
make_IntLsp (LspCtx* ctx, int i)
{
  LspKind* kind = ensure_kind_LspCtx (ctx, vt_IntLsp ());
  IntLsp sp = to_IntLsp (take_LspKind (kind));
  sp->i = i;
  return sp;
}

  const LspVT*
vt_IntLsp ()
{
  static bool vt_initialized = false;
  static LspVT vt;
  if (!vt_initialized) {
    vt_initialized = true;
    memset (&vt, 0, sizeof (vt));
    vt.base_offset = offsetof( IntLspBase, base );
    vt.size = sizeof(IntLspBase);
  }
  return &vt;
}

