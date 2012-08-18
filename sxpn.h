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
    } as;
};

struct Cons
{
    Cons* cdr;
    ConsAtom car;
    unsigned short nrefs;
};

struct Sxpn
{
    LgTable cells;
};

void
dump_ConsAtom (OFileB* of, const ConsAtom* ca);
void
dump_Cons (OFileB* of, const Cons* a);


qual_inline
    ConsAtom
dflt_ConsAtom ()
{
    ConsAtom ca;
    ca.kind = Cons_NKinds;
    memset (&ca, 0, sizeof(ca));
    return ca;
}

qual_inline
    ConsAtom
dflt_Cons_ConsAtom (Cons* c)
{
    ConsAtom ca = dflt_ConsAtom ();
    ca.kind = Cons_Cons;
    ca.as.cons = c;
    if (c)  ++ c->nrefs;
    return ca;
}

qual_inline
    Cons
dflt2_Cons (ConsAtom a, Cons* b)
{
    DecloStack( Cons, c );
    c->car = a;
    c->nrefs = 1;
    c->cdr = b;
    if (b)  ++ b->nrefs;
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
    DecloStack( Sxpn, sx );
    sx->cells = dflt1_LgTable (sizeof (Cons));
    return *sx;
}

qual_inline
    Cons*
req_Sxpn (Sxpn* sx)
{
    Cons* c = (Cons*) req_LgTable (&sx->cells);
    *c = dflt_Cons ();
    return c;
}

qual_inline
    Cons*
req1_Sxpn (Sxpn* sx, Cons* b)
{
    Cons* c = (Cons*) req_LgTable (&sx->cells);
    *c = dflt1_Cons (b);
    return c;
}

qual_inline
    Cons*
req2_Sxpn (Sxpn* sx, ConsAtom a, Cons* b)
{
    Cons* c = (Cons*) req_LgTable (&sx->cells);
    *c = dflt2_Cons (a, b);
    return c;
}


static void giv_Sxpn (Sxpn* sx, Cons* a);

qual_inline
    void
lose_ConsAtom (ConsAtom* ca, Sxpn* sx)
{
    switch (ca->kind)
    {
    case Cons_Cons:
        giv_Sxpn (sx, ca->as.cons);
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
giv_Sxpn (Sxpn* sx, Cons* a)
{
    while (a)
    {
        Cons* b;
        -- a->nrefs;
        if (a->nrefs > 0)  break;

        lose_ConsAtom (&a->car, sx);

        b = a;
        a = a->cdr;
        giv_LgTable (&sx->cells, b);
    }
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

