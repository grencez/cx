/**
 * \file lsp.h
 * Base for S-expressions.
 **/
#ifndef Lsp_H_
#define Lsp_H_
#include "associa.h"

typedef struct LspBase LspBase;
typedef LspBase* Lsp;
typedef struct LspKind LspKind;
typedef struct LspCtx LspCtx;
typedef struct LspVT LspVT;
typedef struct ConsLspBase ConsLspBase;
typedef struct StringLspBase StringLspBase;
typedef struct ConstStringLspBase ConstStringLspBase;
typedef struct NatLspBase NatLspBase;
typedef struct IntLspBase IntLspBase;
typedef ConsLspBase* ConsLsp;
typedef StringLspBase* StringLsp;
typedef ConstStringLspBase* ConstStringLsp;
typedef NatLspBase* NatLsp;
typedef IntLspBase* IntLsp;

struct LspBase
{
  LspKind* kind;
};

struct LspKind
{
  LgTable cells;
  const LspVT* vt;
  LspCtx* ctx;
};

struct ConsLspBase
{
  LspBase base;
  Lsp car;
  ConsLsp cdr;
};

struct LspCtx
{
  ConsLspBase nil;
  // LspVT -> LspKind*
  Associa kindmap;
};

struct ConstStringLspBase
{
  LspBase base;
  const char* s;
};

struct StringLspBase
{
  LspBase base;
  char* s;
};

struct NatLspBase
{
  LspBase base;
  uint u;
};

struct IntLspBase
{
  LspBase base;
  int i;
};

struct LspVT
{
  size_t base_offset;
  size_t size;

  void (*lose_fn) (Lsp);

  Lsp (*car_fn) (LspCtx*, const Lsp);
  Lsp (*cdr_fn) (LspCtx*, const Lsp);
};

LspCtx*
make_LspCtx ();
void
free_LspCtx (LspCtx* ctx);
LspCtx*
ctx_of_Lsp (const Lsp a);

LspKind*
ensure_kind_LspCtx (LspCtx* ctx, const LspVT* vt);

LspKind*
make_LspKind (const LspVT* vt);
void
free_LspKind (LspKind* kind);

ConsLsp
cons_Lsp (Lsp a, ConsLsp b);
Lsp
list2_Lsp (Lsp a, Lsp b);
Lsp
list3_Lsp (Lsp a, Lsp b, Lsp c);
Lsp
list4_Lsp (Lsp a, Lsp b, Lsp c, Lsp d);

qual_inline
  void
lose_Lsp (Lsp sp)
{
  if (sp->kind) {
    if (sp->kind->vt->lose_fn) {
      sp->kind->vt->lose_fn (sp);
    }
  }
}

StringLsp
make_StringLsp (LspCtx* ctx, const char* s);
ConstStringLsp
make_ConstStringLsp (LspCtx* ctx, const char* s);
NatLsp
make_NatLsp (LspCtx* ctx, uint u);
IntLsp
make_IntLsp (LspCtx* ctx, int i);

qual_inline
  StringLsp
to_StringLsp (Lsp sp)
{
  return CastUp( StringLspBase, base, sp );
}

qual_inline
  ConstStringLsp
to_ConstStringLsp (Lsp sp)
{
  return CastUp( ConstStringLspBase, base, sp );
}

qual_inline
  NatLsp
to_NatLsp (Lsp sp)
{
  return CastUp( NatLspBase, base, sp );
}

qual_inline
  IntLsp
to_IntLsp (Lsp sp)
{
  return CastUp( IntLspBase, base, sp );
}

qual_inline
  Lsp
make_String_Lsp (LspCtx* ctx, const char* s)
{
  return &make_StringLsp (ctx, s)->base;
}

qual_inline
  Lsp
make_ConstString_Lsp (LspCtx* ctx, const char* s)
{
  return &make_ConstStringLsp (ctx, s)->base;
}

qual_inline
  Lsp
make_Nat_Lsp (LspCtx* ctx, uint u)
{
  return &make_NatLsp (ctx, u)->base;
}

qual_inline
  Lsp
make_Int_Lsp (LspCtx* ctx, int i)
{
  return &make_IntLsp (ctx, i)->base;
}

qual_inline
  Lsp
list2_ccstr_Lsp (const char* a, Lsp b)
{
  return list2_Lsp (make_ConstString_Lsp (ctx_of_Lsp (b), a), b);
}

qual_inline
  Lsp
list3_ccstr_Lsp (const char* a, Lsp b, Lsp c)
{
  return list3_Lsp (make_ConstString_Lsp (ctx_of_Lsp (b), a), b, c);
}

qual_inline
  Lsp
list4_ccstr_Lsp (const char* a, Lsp b, Lsp c, Lsp d)
{
  return list4_Lsp (make_ConstString_Lsp (ctx_of_Lsp (b), a), b, c, d);
}

#endif

