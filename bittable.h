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


#define Bits4LH( b00, b01, b10, b11 ) \
  ((b00 << 0) | (b01 << 1) | (b10 << 2) | (b11 << 3))

enum BitOp {
  //           a = 0011
  //           b = 0101
  BitOp_NIL   = /* 0000 */ Bits4LH(0,0,0,0),
  BitOp_NOR   = /* 1000 */ Bits4LH(1,0,0,0),
  BitOp_NOT1  = /* 1100 */ Bits4LH(1,1,0,0),
  BitOp_NIMP  = /* 0010 */ Bits4LH(0,0,1,0),
  BitOp_NOT0  = /* 1010 */ Bits4LH(1,0,1,0),
  BitOp_XOR   = /* 0110 */ Bits4LH(0,1,1,0),
  BitOp_NAND  = /* 1110 */ Bits4LH(1,1,1,0),
  BitOp_AND   = /* 0001 */ Bits4LH(0,0,0,1),
  BitOp_EQL   = /* 1001 */ Bits4LH(1,0,0,1),
  BitOp_IDEN1 = /* 0101 */ Bits4LH(0,1,0,1),
  BitOp_IMP   = /* 1101 */ Bits4LH(1,1,0,1),
  BitOp_IDEN0 = /* 0011 */ Bits4LH(0,0,1,1),
  BitOp_OR    = /* 0111 */ Bits4LH(0,1,1,1),
  BitOp_YES   = /* 1111 */ Bits4LH(1,1,1,1),
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
size_fo_BitTable (BitTable* bt, ujint n)
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
size_BitTable (BitTable* bt, ujint n)
{
  size_fo_BitTable (bt, n);
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

/** Check if a bit is set (to one).**/
qual_inline
    Bit
chk_BitTable (const BitTable bt, ujint i)
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
op2_BitTable (BitTable* c, BitOp op, const BitTable a, const BitTable b)
{
  ujint i;
  const ujint n = CeilQuot( a.sz, NBits_BitTableEl );

  Claim2( a.sz ,==, b.sz );
  size_fo_BitTable (c, a.sz);

  switch (op)
  {
  case BitOp_NIL:
    wipe_BitTable (*c, 0);
    break;
  case BitOp_NOR:
    UFor( i, n )  c->s[i] = ~(a.s[i] | b.s[i]);
    break;
  case BitOp_NOT1:
    UFor( i, n )  c->s[i] = ~b.s[i];
    break;
  case BitOp_NIMP:
    UFor( i, n )  c->s[i] = a.s[i] & ~b.s[i];
    break;
  case BitOp_NOT0:
    UFor( i, n )  c->s[i] = ~a.s[i];
    break;
  case BitOp_XOR:
    UFor( i, n )  c->s[i] = a.s[i] ^ b.s[i];
  case BitOp_NAND:
    UFor( i, n )  c->s[i] = ~(a.s[i] & b.s[i]);
    break;
  case BitOp_AND:
    UFor( i, n )  c->s[i] = a.s[i] & b.s[i];
    break;
  case BitOp_EQL:
    UFor( i, n )  c->s[i] = ~(a.s[i] ^ b.s[i]);
    break;
  case BitOp_IDEN1:
    if (c->s != b.s)  memcpy (c->s, b.s, n * sizeof (BitTableEl));
    break;
  case BitOp_IMP:
    UFor( i, n )  c->s[i] = ~a.s[i] | b.s[i];
    break;
  case BitOp_IDEN0:
    break;
  case BitOp_OR:
    UFor( i, n )  c->s[i] = a.s[i] | b.s[i];
    break;
  case BitOp_YES:
    wipe_BitTable (a, 1);
    break;
  case NBitOps:
    Claim(0);
    break;
  }
}

qual_inline
  void
op_BitTable (BitTable a, BitOp op, const BitTable b)
{
  ujint i;
  const ujint n = CeilQuot( a.sz, NBits_BitTableEl );

  Claim2( a.sz ,==, b.sz );
  switch (op)
  {
  case BitOp_NIL:
    wipe_BitTable (a, 0);
    break;
  case BitOp_NOT1:
    UFor( i, n )  a.s[i] = ~b.s[i];
    break;
  case BitOp_NOT0:
    UFor( i, n )  a.s[i] = ~a.s[i];
    break;
  case BitOp_IDEN1:
    if (a.s != b.s)  memcpy (a.s, b.s, n * sizeof (BitTableEl));
    break;
  case BitOp_IDEN0:
    break;
  case BitOp_YES:
    wipe_BitTable (a, 1);
    break;
  default:
    op2_BitTable (&a, op, a, b);
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
next_BitTable (const BitTable bt, ujint idx)
{
    while (++idx < bt.sz)
        if (test_BitTable (bt, idx))
            return idx;
    return Max_ujint;
}

qual_inline
    ujint
beg_BitTable (const BitTable bt)
{
    if (bt.sz == 0)  return Max_ujint;
    if (test_BitTable (bt, 0))  return 0;
    return next_BitTable (bt, 0);
}

qual_inline
    ujint
count_BitTable (const BitTable bt)
{
    ujint n = 0;
    for (ujint i = beg_BitTable (bt);
         i != Max_ujint;
         i = next_BitTable (bt, i))
        ++n;
    return n;
}

#endif

