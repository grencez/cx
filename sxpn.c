/**
 * \file sxpn.c
 **/
#include "sxpn.h"

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

