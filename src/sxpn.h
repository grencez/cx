/**
 * \file sxpn.h
 **/
#ifndef Sxpn_H_
#define Sxpn_H_
#include "lgtable.h"
#include "fileb.h"

typedef struct ConsAtom ConsAtom;
typedef struct Cons Cons;
typedef struct Sxpn Sxpn;

enum ConsKind {
    Cons_Cons, /* car is a Cons */
    Cons_MemLoc,
    Cons_AlphaTab,
    Cons_cstr,
    Cons_int,
    Cons_uint,
    Cons_ujint,
    Cons_real,
    Cons_NKinds
};
typedef enum ConsKind ConsKind;

#define DeclTableT_Cons
DeclTableT( Cons, Cons );

struct ConsAtom
{
    ConsKind kind;
    union ConsAtom_union
    {
        Cons* cons;
        void* memloc;
        AlphaTab alphatab;
        char* cstr;
        int i;
        uint ui;
        ujint uji;
        real re;
    } as;
};

struct Cons
{
    Cons* cdr;
    ConsAtom car;
    uint nrefs;
};

struct Sxpn
{
  LgTable cells;
};
#define DEFAULT_Sxpn { DEFAULT1_LgTable(Cons) }

void
oput_ConsAtom (OFile* of, const ConsAtom* ca);
void
oput_Cons (OFile* of, const Cons* a);


qual_inline
    ConsAtom
dflt_ConsAtom ()
{
    ConsAtom ca;
    ca.kind = Cons_NKinds;
    memset (&ca, 0, sizeof(ca));
    return ca;
}

/** Increment reference count.**/
qual_inline
    void
inc_Cons (Cons* a)
{
    if (a && ~a->nrefs != 0)
        ++ a->nrefs;
}

/** Decrement reference count.**/
qual_inline
    void
dec_Cons (Cons* a)
{
    if (a && a->nrefs > 0 && ~a->nrefs != 0)
        -- a->nrefs;
}

qual_inline
    ConsAtom
dflt_Cons_ConsAtom (Cons* c)
{
    ConsAtom ca = dflt_ConsAtom ();
    ca.kind = Cons_Cons;
    ca.as.cons = c;
    inc_Cons (c);
    return ca;
}

qual_inline
  Cons
dflt2_Cons (ConsAtom a, Cons* b)
{
  Cons c[1];
  c->car = a;
  c->nrefs = 1;
  c->cdr = b;
  inc_Cons (b);
  return *c;
}

qual_inline
Cons dflt1_Cons (Cons* b) { return dflt2_Cons (dflt_ConsAtom (), b); }

qual_inline
Cons dflt_Cons () { return dflt1_Cons (0); }


qual_inline
  Sxpn
dflt_Sxpn ()
{
  Sxpn sx = default;
  return sx;
}

qual_inline
    Cons*
take_Sxpn (Sxpn* sx)
{
    Cons* c = (Cons*) take_LgTable (&sx->cells);
    *c = dflt_Cons ();
    return c;
}

qual_inline
    Cons*
take1_Sxpn (Sxpn* sx, Cons* b)
{
    Cons* c = (Cons*) take_LgTable (&sx->cells);
    *c = dflt1_Cons (b);
    return c;
}

qual_inline
    Cons*
take2_Sxpn (Sxpn* sx, ConsAtom a, Cons* b)
{
    Cons* c = (Cons*) take_LgTable (&sx->cells);
    *c = dflt2_Cons (a, b);
    return c;
}


static void give_Sxpn (Sxpn* sx, Cons* a);

qual_inline
    void
lose_ConsAtom (ConsAtom* ca, Sxpn* sx)
{
    switch (ca->kind)
    {
    case Cons_Cons:
        give_Sxpn (sx, ca->as.cons);
        break;
    case Cons_AlphaTab:
        lose_AlphaTab (&ca->as.alphatab);
        break;
    case Cons_cstr:
        free (ca->as.cstr);
        break;
    default:
        break;
    }
}

    void
give_Sxpn (Sxpn* sx, Cons* a)
{
    while (a)
    {
        Cons* b;
        dec_Cons (a);
        if (a->nrefs > 0)  break;

        lose_ConsAtom (&a->car, sx);

        b = a;
        a = a->cdr;
        give_LgTable (&sx->cells, b);
    }
}

qual_inline
    Cons*
pop_Sxpn (Sxpn* sx, Cons* a)
{
    Cons* tmp = a->cdr;
    give_Sxpn (sx, a);
    return tmp;
}

qual_inline
    void
lose_Sxpn (Sxpn* sx)
{
    ujint i;
    for (i = begidx_LgTable (&sx->cells);
         i < Max_ujint;
         i = nextidx_LgTable (&sx->cells, i))
    {
        Cons* a = (Cons*) elt_LgTable (&sx->cells, i);
        if (a->car.kind != Cons_Cons)
            lose_ConsAtom (&a->car, 0);
    }
    lose_LgTable (&sx->cells);
}


#endif

