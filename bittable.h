/**
 * \file bittable.h
 **/
#ifndef BitTable_H_
#define BitTable_H_

#include "table.h"

#include <assert.h>

DeclTableT( Bit, unsigned int );

typedef TableT(Bit) BitTable;
typedef TableElT(Bit) BitTableEl;
#define NBits_BitTableEl  (sizeof (BitTableEl) * NBits_byte)

#define FixDeclBitTable( bt, n, val ) \
    BitTableEl DeclBitTable_##bt[CeilQuot( n, NBits_BitTableEl )]; \
    BitTable bt = dflt3_BitTable( n, DeclBitTable_##bt, val )

#define LowBits( x, nbits ) \
    ((x) & ~(((x) | ~(x)) << (nbits)))


enum BitOp {
    /*         a = 0011
     *         b = 0101
     */
    BitOp_NIL,  /* 0000 */
    BitOp_AND,  /* 0001 */
    BitOp_NIMP, /* 0010 */
    BitOp_IDEN, /* 0101 */
    BitOp_XOR,  /* 0110 */
    BitOp_OR,   /* 0111 */
    BitOp_NOR,  /* 1000 */
    BitOp_EQL,  /* 1001 */
    BitOp_NOT,  /* 1010 */
    BitOp_IMP,  /* 1101 */
    BitOp_NAND, /* 1110 */
    BitOp_YES,  /* 1111 */
    NBitOps
};
typedef enum BitOp BitOp;


/** Most significant 1 bit.**/
qual_inline
    ujint
msb_ujint (ujint x)
{
    ujintlg i;
    for (i = 1; i < NBits_ujint; i *= 2)
        x |= (x >> i);
    return (x & ~(x >> 1));
}

/** Least significant 1 bit.**/
qual_inline
    ujint
lsb_ujint (ujint x)
{
    return (x & (~x + 1));
}

/** Floor of the lg (log base 2) of some integer.
 * - 0..1 -> 0
 * - 2..3 -> 1
 * - 4..7 -> 2
 * - 8..15 -> 3
 * - 16..31 -> 4
 * - 32..63 -> 5
 * - 64..127 -> 6
 **/
qual_inline
    ujintlg
lg_ujint (ujint x)
{
    ujintlg i;
    ujintlg n = 0;
    for (i = msb_ujint (NBits_ujint-1); i > 0; i /= 2)
    {
        ujint y = (x >> i);
        n *= 2;
        if (y != 0)
        {
            n += 1;
            x = y;
        }
    }
    return n;
}


#define DeclBitTableIdcs( p, q, i ) \
    const ujint p = (i) / NBits_BitTableEl; \
    const uint    q = (i) % NBits_BitTableEl

qual_inline
    void
wipe_BitTable (BitTable bt, Bit val)
{
    const ujint n = CeilQuot( bt.sz, NBits_BitTableEl );
    memset (bt.s, 
            (val == 0) ? 0x00 : 0xFF,
            n * sizeof (BitTableEl));
}

qual_inline
    BitTable
dflt_BitTable ()
{
    DeclTable( Bit, bt );
    return bt;
}

qual_inline
    BitTable
dflt2_BitTable (ujint nbits, BitTableEl* s)
{
    BitTable bt = dflt_BitTable ();
    bt.s = s;
    bt.sz = nbits;
    return bt;
}

qual_inline
    BitTable
dflt3_BitTable (ujint nbits, BitTableEl* s, Bit val)
{
    BitTable bt = dflt2_BitTable (nbits, s);
    wipe_BitTable (bt, val);
    return bt;
}

qual_inline
    BitTable
cons1_BitTable (ujint n)
{
    const ujint nblocks = CeilQuot( n, NBits_BitTableEl );
    BitTable bt = dflt_BitTable ();
    GrowTable( bt, nblocks );
    bt.sz = n;
    return bt;
}

qual_inline
    BitTable
cons2_BitTable (ujint n, Bit val)
{
    BitTable bt = cons1_BitTable (n);

    if (bt.s)
        wipe_BitTable (bt, val);

    return bt;
}

qual_inline
    void
lose_BitTable (BitTable* bt)
{
    LoseTable( *bt );
}

qual_inline
    void
size_BitTable (BitTable* bt, ujint n)
{
    const ujint sz = bt->sz;
    const ujint new_sz = n;
    const ujint nelems = CeilQuot( sz, NBits_BitTableEl );
    const ujint new_nelems = CeilQuot( new_sz, NBits_BitTableEl );
    bt->sz = nelems;
    SizeTable( *bt, new_nelems );
    if (new_nelems > nelems)
        memset (&bt->s[nelems], 0,
                (new_nelems - nelems) * sizeof (BitTableEl));
    bt->sz = new_sz;
}

qual_inline
    void
grow_BitTable (BitTable* bt, ujint n)
{
    size_BitTable (bt, bt->sz+n);
}

qual_inline
    void
mpop_BitTable (BitTable* bt, ujint n)
{
    size_BitTable (bt, bt->sz - n);
}


/** Test if a bit is set (to one).**/
qual_inline
    Bit
test_BitTable (const BitTable bt, ujint i)
{
    DeclBitTableIdcs( p, q, i );
    return (0 != (bt.s[p] & (1 << q)));
}

/** Set a bit to one.**/
qual_inline
    Bit
set1_BitTable (BitTable bt, ujint i)
{
    DeclBitTableIdcs( p, q, i );
    const BitTableEl x = bt.s[p];
    const BitTableEl y = 1 << q;

    if (0 != (x & y))
    {
        return 1;
    }
    else
    {
        bt.s[p] = x | y;
        return 0;
    }
}

/** Set a bit to zero.**/
qual_inline
    Bit
set0_BitTable (BitTable bt, ujint i)
{
    DeclBitTableIdcs( p, q, i );
    const BitTableEl x = bt.s[p];
    const BitTableEl y = 1 << q;

    if (0 == (x & y))
    {
        return 0;
    }
    else
    {
        bt.s[p] = x & ~y;
        return 1;
    }
}

/** Set a bit.
 * \sa set0_BitTable()
 * \sa set1_BitTable()
 **/
qual_inline
    Bit
setb_BitTable (BitTable bt, ujint i, Bit b)
{
    return (b ? set1_BitTable (bt, i) : set0_BitTable (bt, i));
}

qual_inline
    void
op_BitTable (BitTable a, const BitTable b, BitOp op)
{
    ujint i;
    const ujint n = CeilQuot( a.sz, NBits_BitTableEl );

    Claim2( a.sz ,==, b.sz );
    switch (op)
    {
        case BitOp_NIL:
            wipe_BitTable (a, 0);
            break;
        case BitOp_AND:
            UFor( i, n )  a.s[i] = a.s[i] & b.s[i];
            break;
        case BitOp_NIMP:
            UFor( i, n )  a.s[i] = a.s[i] & ~b.s[i];
            break;
        case BitOp_IDEN:
            if (a.s != b.s)  memcpy (a.s, b.s, n * sizeof (BitTableEl));
            break;
        case BitOp_XOR:
            UFor( i, n )  a.s[i] = a.s[i] ^ b.s[i];
            break;
        case BitOp_OR:
            UFor( i, n )  a.s[i] = a.s[i] | b.s[i];
            break;
        case BitOp_NOR:
            UFor( i, n )  a.s[i] = ~(a.s[i] | b.s[i]);
            break;
        case BitOp_EQL:
            UFor( i, n )  a.s[i] = ~(a.s[i] ^ b.s[i]);
            break;
        case BitOp_NOT:
            UFor( i, n )  a.s[i] = ~b.s[i];
            break;
        case BitOp_IMP:
            UFor( i, n )  a.s[i] = ~a.s[i] | b.s[i];
            break;
        case BitOp_NAND:
            UFor( i, n )  a.s[i] = ~(a.s[i] & b.s[i]);
            break;
        case BitOp_YES:
            wipe_BitTable (a, 1);
            break;
        case NBitOps:
            Claim( 0 );
            break;
    }
}

qual_inline
    Bit
all_BitTable (const BitTable bt)
{
    DeclBitTableIdcs( p, q, bt.sz );
    ujint i;

    UFor( i, p )
        if (0 != ~ bt.s[i])
            return 0;

    UFor( i, q )
        if (!test_BitTable (bt, bt.sz - 1 - i))
            return 0;

    return 1;
}

#undef DeclBitTableIdcs

qual_inline
    ujint
next1_BitTable (const BitTable bt, ujint idx)
{
    while (++idx < bt.sz)
        if (test_BitTable (bt, idx))
            return idx;
    return Max_ujint;
}

#endif

