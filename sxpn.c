/**
 * \file sxpn.c
 **/
#include "sxpn.h"

    void
oput_ConsAtom (OFileB* of, const ConsAtom* ca)
{
    switch (ca->kind)
    {
    case Cons_Cons:
        oput_Cons (of, ca->as.cons);
        break;
    case Cons_AlphaTab:
        oput_AlphaTab (of, &ca->as.alphatab);
        break;
    case Cons_cstr:
        oput_cstr_OFileB (of, ca->as.cstr);
        break;
    case Cons_int:
        oput_int_OFileB (of, ca->as.i);
        break;
    case Cons_uint:
        oput_uint_OFileB (of, ca->as.ui);
        break;
    case Cons_ujint:
        oput_ujint_OFileB (of, ca->as.uji);
        break;
    case Cons_real:
        oput_real_OFileB (of, ca->as.re);
        break;
    default:
        break;
    }
}

    void
oput_Cons (OFileB* of, const Cons* a)
{
    oput_char_OFileB (of, '(');
    while (a)
    {
        oput_ConsAtom (of, &a->car);
        a = a->cdr;
        if (a)
            oput_char_OFileB (of, ' ');
    }
    oput_char_OFileB (of, ')');
}

