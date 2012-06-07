
#ifndef BitTable_H_
#define BitTable_H_

#include "table.h"

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
cons2_BitTable (BitTableSz n, Bit val)
{
    const BitTableSz nblocks = CeilQuot( n, NBits_BitTableEl );
    BitTable bt = dflt_BitTable ();

    GrowTable( bt, nblocks );
    bt.sz = n;

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

