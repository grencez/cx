
#ifndef Cons_H_
#define Cons_H_
#include "fileb.h"

typedef struct ConsAtom ConsAtom;
typedef struct Cons Cons;

enum  ConsKind {
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

struct ConsAtom
{
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
    ConsKind kind;
};

struct Cons
{
    Cons* cdr;
    ConsAtom car;
    unsigned short nrefs;
};

qual_inline
    ConsAtom
dflt_ConsAtom ()
{
    ConsAtom ca;
    memset (&ca, 0, sizeof(ca));
    ca.kind = Cons_NKinds;
    return ca;
}

qual_inline
    ConsAtom
dflt_Cons_ConsAtom (Cons* c)
{
    ConsAtom ca = dflt_ConsAtom ();
    ca.as.cons = c;
    ca.kind = Cons_Cons;
    if (c)  ++ c->nrefs;
    return ca;
}

qual_inline
    Cons*
make2_Cons (ConsAtom a, Cons* b)
{
    DeclAlloc( Cons, c, 1 );
    c->car = a;
    c->nrefs = 1;
    c->cdr = b;
    if (b)  ++ b->nrefs;
    return c;
}

qual_inline
    Cons*
make1_Cons (Cons* b)
{ return make2_Cons (dflt_ConsAtom (), b); }

qual_inline
Cons* make_Cons () { return make1_Cons (0); }

static void lose_Cons (Cons* a);

qual_inline
    void
lose_ConsAtom (ConsAtom* ca)
{
    switch (ca->kind)
    {
    case Cons_Cons:
        lose_Cons (ca->as.cons);
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
lose_Cons (Cons* a)
{
    while (a)
    {
        Cons* b;
        -- a->nrefs;
        if (a->nrefs > 0)  break;
        lose_ConsAtom (&a->car);

        b = a;
        a = a->cdr;
        free (b);
    }
}

static void dump_Cons (OFileB* of, const Cons* a);

qual_inline
    void
dump_ConsAtom (OFileB* of, const ConsAtom* ca)
{
    switch (ca->kind)
    {
    case Cons_Cons:
        dump_Cons (of, ca->as.cons);
        break;
    case Cons_AlphaTab:
        dump_AlphaTab (of, &ca->as.alphatab);
        break;
    case Cons_cstr:
        dump_cstr_OFileB (of, ca->as.cstr);
        break;
    case Cons_int:
        dump_int_OFileB (of, ca->as.i);
        break;
    case Cons_uint:
        dump_uint_OFileB (of, ca->as.ui);
        break;
    case Cons_ujint:
        dump_ujint_OFileB (of, ca->as.uji);
        break;
    default:
        break;
    }
}

    void
dump_Cons (OFileB* of, const Cons* a)
{
    dump_char_OFileB (of, '(');
    while (a)
    {
        dump_ConsAtom (of, &a->car);
        a = a->cdr;
        if (a)
            dump_char_OFileB (of, ' ');
    }
    dump_char_OFileB (of, ')');
}

#endif

