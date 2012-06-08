
#ifndef BitTable_H_
#define BitTable_H_

#include "table.h"

#include <assert.h>

DeclTableT( Bit, unsigned int );

typedef TableT(Bit) BitTable;
typedef TableElT(Bit) BitTableEl;
typedef TableSzT(Bit) BitTableSz;
#define NBits_BitTableEl  (sizeof (BitTableEl) * NBits_byte)

#define FixDeclBitTable( bt, n, val ) \
    BitTableEl DeclBitTable_##bt[CeilQuot( n, NBits_BitTableEl )]; \
    BitTable bt = dflt3_BitTable( n, DeclBitTable_##bt, val )

#define LowBits( x, nbits ) \
    ((x) & ~(((x) | ~(x)) << (nbits)))


enum BitTable_Op {
        /*        a = 0011
         *        b = 0101
         */
    BitTable_NIL,  /* 0000 */
    BitTable_AND,  /* 0001 */
    BitTable_XOR,  /* 0110 */
    BitTable_OR,   /* 0111 */
    BitTable_NOR,  /* 1000 */
    BitTable_EQL,  /* 1001 */
    BitTable_NOT,  /* 1010 */
    BitTable_IMPL, /* 1101 */
    BitTable_NAND, /* 1110 */
    BitTable_YES,  /* 1111 */
    BitTable_NOps
};
typedef enum BitTable_Op BitTable_Op;

#define DeclBitTableIdcs( p, q, i ) \
    const TableSz p = (i) / NBits_BitTableEl; \
    const uint    q = (i) % NBits_BitTableEl

qual_inline
    void
wipe_BitTable (BitTable bt, Bit val)
{
    const TableSz n = CeilQuot( bt.sz, NBits_BitTableEl );
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
dflt2_BitTable (BitTableSz nbits, BitTableEl* s)
{
    BitTable bt = dflt_BitTable ();
    bt.s = s;
    bt.sz = nbits;
    return bt;
}

qual_inline
    BitTable
dflt3_BitTable (BitTableSz nbits, BitTableEl* s, Bit val)
{
    BitTable bt = dflt2_BitTable (nbits, s);
    wipe_BitTable (bt, val);
    return bt;
}

qual_inline
    BitTable
cons1_BitTable (BitTableSz n)
{
    const BitTableSz nblocks = CeilQuot( n, NBits_BitTableEl );
    BitTable bt = dflt_BitTable ();
    GrowTable( bt, nblocks );
    bt.sz = n;
    return bt;
}

qual_inline
    BitTable
cons2_BitTable (BitTableSz n, Bit val)
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
    Bit
test_BitTable (const BitTable bt, BitTableSz i)
{
    DeclBitTableIdcs( p, q, i );
    return (0 != (bt.s[p] & (1 << q)));
}

qual_inline
    Bit
set1_BitTable (BitTable bt, BitTableSz i)
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

qual_inline
    Bit
set0_BitTable (BitTable bt, BitTableSz i)
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

qual_inline
    void
op_BitTable (BitTable a, const BitTable b, BitTable_Op op)
{
    BitTableSz i;
    const TableSz n = CeilQuot( a.sz, NBits_BitTableEl );

    Claim2( a.sz ,==, b.sz );
    switch (op)
    {
        case BitTable_NIL:
            wipe_BitTable (a, 0);
            break;
        case BitTable_AND:
            UFor( i, n )  a.s[i] = a.s[i] & b.s[i];
            break;
        case BitTable_XOR:
            UFor( i, n )  a.s[i] = a.s[i] ^ b.s[i];
            break;
        case BitTable_OR:
            UFor( i, n )  a.s[i] = a.s[i] | b.s[i];
            break;
        case BitTable_NOR:
            UFor( i, n )  a.s[i] = ~(a.s[i] | b.s[i]);
            break;
        case BitTable_EQL:
            UFor( i, n )  a.s[i] = ~(a.s[i] ^ b.s[i]);
            break;
        case BitTable_NOT:
            UFor( i, n )  a.s[i] = ~b.s[i];
            break;
        case BitTable_IMPL:
            UFor( i, n )  a.s[i] = ~a.s[i] | b.s[i];
            break;
        case BitTable_NAND:
            UFor( i, n )  a.s[i] = ~(a.s[i] & b.s[i]);
            break;
        case BitTable_YES:
            wipe_BitTable (a, 1);
            break;
        case BitTable_NOps:
            Claim( 0 );
            break;
    }
}

qual_inline
    Bit
all_BitTable (const BitTable bt)
{
    DeclBitTableIdcs( p, q, bt.sz );
    BitTableSz i;

    UFor( i, p )
        if (0 != ~ bt.s[i])
            return 0;

    UFor( i, q )
        if (!test_BitTable (bt, bt.sz - 1 - i))
            return 0;

    return 1;
}

#undef DeclBitTableIdcs

#endif

