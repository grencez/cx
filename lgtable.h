/**
 * \file lgtable.h
 * A non-contiguous table which is allocated by the exponentially increasing
 * chunks.
 **/
#ifndef LgTable_H_
#define LgTable_H_
#include "bittable.h"

typedef struct LgTableIntl LgTableIntl;
typedef struct LgTableAlloc LgTableAlloc;
typedef struct LgTable LgTable;

DeclTableT( LgTableIntl, LgTableIntl );
DeclTableT( LgTableAlloc, LgTableAlloc );

struct LgTableIntl
{
    const void* mem;
    ujintlg lgsz;
};

struct LgTableAlloc
{
    void* mem;
    TableT(ujint) avails;
    BitTable bt;
};

struct LgTable
{
    TableElSz elsz;
    TableT(LgTableAlloc) allocs;
    TableT(LgTableIntl) intls;
    ujint lgavails;
    ujint sz;
};

qual_inline
    LgTableAlloc
cons2_LgTableAlloc (TableElSz elsz, ujintlg lgsz)
{
    LgTableAlloc a;
    ujint sz = 2;
    if (lgsz > 0)  sz = (ujint) 1 << lgsz;
    a.mem = malloc (elsz * sz);
    /* memset (a.mem, 0xFF, elsz * sz); */
    InitTable( a.avails );
    a.bt = cons2_BitTable (sz, 0);
    a.bt.sz = 0;
    return a;
}

qual_inline
    void
lose_LgTableAlloc (LgTableAlloc* a)
{
    free (a->mem);
    LoseTable( a->avails );
    lose_BitTable (&a->bt);
}

qual_inline
    LgTable
dflt1_LgTable (TableElSz elsz)
{
    LgTable t;
    t.elsz = elsz;
    t.lgavails = 0;
    InitTable( t.allocs );
    InitTable( t.intls );
    t.sz = 0;
    return t;
}

qual_inline
    void
lose_LgTable (LgTable* t)
{
    uint i;
    for (i = 0; i < t->allocs.sz; ++i)
        lose_LgTableAlloc (&t->allocs.s[i]);
    LoseTable( t->allocs );
    LoseTable( t->intls );
}

qual_inline
    void*
elt_LgTable (LgTable* t, ujint idx)
{
    const ujintlg lgidx = lg_ujint (idx);
    if (lgidx > 0)  idx &= ~((ujint) 1 << lgidx);
    return EltZ( t->allocs.s[lgidx].mem, idx, t->elsz );
}

qual_inline
    ujint
idxelt_LgTable (const LgTable* t, const void* el)
{
    ujintlg lo = 0;
    ujintlg hi = t->intls.sz;
    do
    {
        ujintlg oh = lo + (hi - lo) / 2;
        const LgTableIntl intl = t->intls.s[oh];

        if ((size_t) el < (size_t) intl.mem)
        {
            hi = oh;
        }
        else if (((size_t) el - (size_t) intl.mem)
                 >=
                 ((ujint) t->elsz << intl.lgsz))
        {
            lo = oh+1;
        }
        else
        {
            ujint idx = IdxEltZ( intl.mem, el, t->elsz );
            if (intl.lgsz == 1 && intl.mem == t->allocs.s[0].mem)
                return idx;
            return (idx | ((ujint) 1 << intl.lgsz));
        }
    } while (lo != hi);
    Claim( false );
    return Max_ujint;
}

qual_inline
    void
ins_LgTableIntl (TableT(LgTableIntl)* intls, const void* mem)
{
    ujintlg i;
    GrowTable( *intls, 1 );
    for (i = intls->sz-1; i > 0; --i)
    {
        if ((size_t) mem < (size_t) intls->s[i-1].mem)
            intls->s[i] = intls->s[i-1];
        else
            break;
    }
    intls->s[i].mem = mem;
    intls->s[i].lgsz = (intls->sz == 1 ? 1 : intls->sz-1);
}

qual_inline
    void
del_LgTableIntl (TableT(LgTableIntl)* intls)
{
    ujintlg i = 0, j;
    /* Claim2( intls->sz ,>, 2 ); */
    for (j = 0; j < intls->sz; ++j)
    {
        if (intls->s[j].lgsz != intls->sz-1)
        {
            intls->s[i] = intls->s[j];
            ++ i;
        }
    }
    MPopTable( *intls, 1 );
    /* Claim2( i, ==, intls->sz ); */
}


/** Take control of an element of the table.
 * Table makes any necessary allocations.
 * \sa take_LgTable()
 **/
qual_inline
    ujint
takeidx_LgTable (LgTable* t)
{
    ujint idx;
    if (t->lgavails == 0)
    {
        const ujintlg lgidx = t->allocs.sz;
        LgTableAlloc* a;

        idx = (lgidx == 0) ? 0 : ((ujint) 1 << lgidx);

        PushTable( t->allocs, cons2_LgTableAlloc (t->elsz, lgidx) );
        a = TopTable( t->allocs );

        ins_LgTableIntl (&t->intls, a->mem);

        a->bt.sz = 1;
        if (set1_BitTable (a->bt, 0))
            Claim( false );
        t->lgavails |= ((ujint) 1 << lgidx);
    }
    else
    {
        const ujintlg lgidx = lg_ujint (lsb_ujint (t->lgavails));
        const ujint hibit = ((ujint) 1 << lgidx);
        LgTableAlloc* a = &t->allocs.s[lgidx];

        if (a->avails.sz > 0)
        {
            do
            {
                idx = *TopTable( a->avails );
                MPopTable( a->avails, 1 );
            } while (idx >= a->bt.sz);
        }
        else
        {
            idx = a->bt.sz;
            ++ a->bt.sz;
        }

        if (set1_BitTable (a->bt, idx))
            Claim( false );

        if (lgidx == 0)
        {
            if (a->avails.sz == 0 && a->bt.sz == 2)
                t->lgavails ^= hibit;
        }
        else
        {
            idx |= hibit;
            if (a->avails.sz == 0 && a->bt.sz == hibit)
                t->lgavails ^= hibit;
        }
    }
    ++ t->sz;
    return idx;
}


/** Take control of an element of the table.
 * Table makes any necessary allocations.
 * \sa takeidx_LgTable()
 **/
qual_inline
    void*
take_LgTable (LgTable* t)
{
    return elt_LgTable (t, takeidx_LgTable (t));
}


/** Give control of an element back to the table.
 * \sa give_LgTable()
 **/
qual_inline
    void
giveidx_LgTable (LgTable* t, ujint idx)
{
    const ujintlg lgidx = lg_ujint (idx);
    LgTableAlloc* a = &t->allocs.s[lgidx];

    if (lgidx > 0)
        idx ^= ((ujint) 1 << lgidx);

    if (!set0_BitTable (a->bt, idx))
        Claim( false );

    t->lgavails |= ((ujint) 1 << lgidx);

    if (idx + 1 < a->bt.sz)
    {
        PushTable( a->avails, idx );
    }
    else
    {
        do {
            -- a->bt.sz;
        } while (a->bt.sz > 0 && !test_BitTable (a->bt, a->bt.sz-1));

        if (a->bt.sz == 0)
            SizeTable( a->avails, 0 );
    }

    -- t->sz;
    while (t->allocs.sz > 2 &&
           t->allocs.s[t->allocs.sz-1].bt.sz == 0)
    {
        a = TopTable( t->allocs );
        lose_LgTableAlloc (a);
        del_LgTableIntl (&t->intls);
        MPopTable( t->allocs, 1 );
        t->lgavails ^= ((ujint) 1 << (t->allocs.sz));
    }
}

/** Give control of an element back to the table.
 * \sa giveidx_LgTable()
 **/
qual_inline
    void
give_LgTable (LgTable* t, void* el)
{
    giveidx_LgTable (t, idxelt_LgTable (t, el));
}

qual_inline
    ujint
nextidx_LgTable (const LgTable* t, ujint idx)
{
    ujintlg lgidx = lg_ujint (idx);
    if (lgidx >= t->allocs.sz)  return Max_ujint;
    if (lgidx > 0)  idx ^= ((ujint) 1 << lgidx);
    idx = next_BitTable (t->allocs.s[lgidx].bt, idx);

    while (idx == Max_ujint)
    {
        ++lgidx;
        if (lgidx == t->allocs.sz)  break;
        idx = beg_BitTable (t->allocs.s[lgidx].bt);
    }

    if (idx == Max_ujint)  return idx;
    if (lgidx == 0)  return idx;
    return (idx ^ ((ujint) 1 << lgidx));
}

qual_inline
    ujint
begidx_LgTable (const LgTable* t)
{
    if (t->allocs.sz > 0)
    {
        const LgTableAlloc* a = &t->allocs.s[0];
        if (a->bt.sz > 0 && test_BitTable (a->bt, 0))
            return 0;
        return nextidx_LgTable (t, 0);
    }
    return Max_ujint;
}

#endif

