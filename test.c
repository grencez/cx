/**
 * \file test.c
 * Tests for cx.
 * Be sure to run this through valgrind sometimes, it should not leak!
 **/
#include "cx/syscx.h"
#include "cx/associa.h"
#include "cx/bittable.h"
#include "cx/fileb.h"
#include "cx/lgtable.h"
#include "cx/ospc.h"
#include "cx/rbtree.h"
#include "cx/sxpn.h"
#include "cx/table.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct TNode TNode;
struct TNode
{
    RBTNode rbt;
    const char* key;
    uint val;
};

    Trit
swapped_TNode (const BSTNode* lhs, const BSTNode* rhs)
{
    const TNode* a = CastUp( TNode, rbt, CastUp( RBTNode, bst, lhs ) );
    const TNode* b = CastUp( TNode, rbt, CastUp( RBTNode, bst, rhs ) );
    int ret = strcmp (a->key, b->key);

    if (ret < 0)  return Nil;
    if (ret > 0)  return Yes;
    return May;
}

    void
lose_TNode (BSTNode* x)
{
    TNode* a = CastUp( TNode, rbt, CastUp( RBTNode, bst, x ) );
    a->key = 0;
}

static
    uint
countup_black_RBTNode (const RBTNode* x)
{
    uint n = 0;
    Claim( x->bst.joint );
    do
    {
        n += (x->red ? 0 : 1);
        x = CastUp( RBTNode, bst, x->bst.joint );
    } while (x->bst.joint);
    return n;
}

static
    void
claim_TNode (BSTNode* x, void* args)
{
    RBTNode* b = CastUp( RBTNode, bst, x );
    uint* n = (uint*)((void**)args)[0];
    uint* nblack = (uint*)((void**)args)[1];
    *n += 1;

    Claim( x->joint );

    Claim( x->joint );
    Claim2( x ,==, x->joint->split[side_of_BSTNode (x)]);

    if (x->split[0])
        Claim2( x ,==, x->split[0]->joint );
    if (x->split[1])
        Claim2( x ,==, x->split[1]->joint );

    if (b->red)
    {
        b = CastUp( RBTNode, bst, x->joint );
        Claim( !b->red );
    }
    if (!x->split[0] || !x->split[1])
    {
        uint c = countup_black_RBTNode (b);
        if (*nblack == Max_uint)
            *nblack = c;
        else
            Claim2( *nblack ,==, c );
    }
}

static
    void
claim_BSTree (BSTree* t, uint n_expect)
{
    uint n_result;
    uint nblack = Max_uint;
    void* args[2];
    args[0] = &n_result;
    args[1] = &nblack;

    n_result = 0;
    walk_BSTree (t, Yes, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
    n_result = 0;
    walk_BSTree (t, Nil, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
    n_result = 0;
    walk_BSTree (t, May, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
}

static
    void
insert_TNode (RBTree* t, LgTable* lgt,
              const char* key, uint val, uint* n_expect)
{
    TNode* a = (TNode*) req_LgTable (lgt);
    a->key = key;
    a->val = val;
    insert_RBTree (t, &a->rbt);
    *n_expect += 1;
    claim_BSTree (&t->bst, *n_expect);
}

    void
output_dot_fn (BSTNode* x, void* args)
{
    TNode* a = CastUp( TNode, rbt, CastUp( RBTNode, bst, x ) );
    OFileB* of = (OFileB*) ((void**)args)[0];

    printf_OFileB (of, "q%u [label = \"%s\", color = \"%s\"];\n",
                   a->val,
                   a->key,
                   (a->rbt.red) ? "red" : "black");

    if (x->joint)
    {
        TNode* b = CastUp( TNode, rbt, CastUp( RBTNode, bst, x->joint ) );
        printf_OFileB (of, "q%u -> q%u;\n", b->val, a->val);
    }
    flush_OFileB (of);
}

    void
output_dot (BSTree* t)
{
    void* args[1];
    FileB ofb = dflt_FileB ();
    OFileB* of = &ofb.xo;
    args[0] = of;

    seto_FileB (&ofb, true);
    open_FileB (&ofb, "", "out.dot");

    dump_cstr_OFileB (of, "digraph tree {\n");
    output_dot_fn (t->sentinel, args);
    walk_BSTree (t, Yes, output_dot_fn, args);
    dump_cstr_OFileB (of, "}\n");
    lose_OFileB (of);
}

static
    TNode*
find_TNode (RBTree* t, const char* s)
{
    TNode a;
    BSTNode* x;
    a.key = s;
    x = find_BSTree (&t->bst, &a.rbt.bst);
    if (!x)  return 0;
    return CastUp( TNode, rbt, CastUp( RBTNode, bst, x ) );
}

static
    void
remove_TNode (RBTree* t, LgTable* lgt,
              const char* key, uint* n_expect)
{
    TNode* a = find_TNode (t, key);
    Claim( a );
    remove_RBTree (t, &a->rbt);
    lose_TNode (&a->rbt.bst);
    giv_LgTable (lgt, a);
    *n_expect -= 1;
    claim_BSTree (&t->bst, *n_expect);
}


/** \test
 * Test Associa structure.
 * \sa testfn_RBTree()
 **/
static
    void
testfn_Associa ()
{
    static const char* const keys[] = {
        "a", "b", "c", "d", "e", "f", "g",
        "h", "i", "j", "k", "l", "m", "n",
        "o", "p", "q", "r", "s", "t", "u",
        "v", "w", "x", "y", "z"
    };
    static const uint muls[] = {
        1, 3, 5, 7, 9, 11, 15, 17, 19, 21
    };
    const uint nkeys = ArraySz( keys );
    const uint nmuls = ArraySz( muls );
    DeclAssocia( AlphaTab, uint, map, (SwappedFn) swapped_AlphaTab );
    uint n_expect = 1; /* Sentinel node.*/

    Claim2( map->nodes.sz ,==, n_expect );
    { BLoop( mi, nmuls )
        { BLoop( mj, nmuls )
            { BLoop( i, nkeys )
                const uint idx = (muls[mi] * i) % nkeys;
                const AlphaTab key = dflt1_AlphaTab (keys[idx]);
                if (mj % 2 == 0)
                {
                    insert_Associa (map, &key, &idx);
                }
                else
                {
                    bool added = false;
                    Assoc* assoc = ensure1_Associa (map, &key, &added);
                    Claim( added );
                    val_fo_Assoc (assoc, &idx);
                }
                ++ n_expect;
                Claim2( map->nodes.sz ,==, n_expect );
            } BLose()


            { BLoop( i, nkeys )
                const uint idx = (muls[mj] * i) % nkeys;
                const AlphaTab key = dflt1_AlphaTab (keys[idx]);
                Assoc* a;
                if (mj % 2 == 0)
                {
                    a = lookup_Associa (map, &key);
                }
                else
                {
                    bool added = true;
                    a = ensure1_Associa (map, &key, &added);
                    Claim( !added );
                }
                Claim( a );
                {
                    uint val = *(uint*) val_of_Assoc (a);
                    Claim2( idx ,==, val );
                }
                lose_Assoc (a);
                -- n_expect;
                Claim2( map->nodes.sz ,==, n_expect );
            } BLose()
        } BLose()
    } BLose()

    /* Claim the sentinel still exists.*/
    Claim2( map->nodes.sz ,==, 1 );
    Claim2( n_expect ,==, 1 );
    lose_Associa (map);
}


/** \test
 * Set and test at a bunch of indices.
 **/
static
    void
testfn_BitTable ()
{
    uint n = 1000;
    uint ni = CeilQuot( n, 3 );
    BitTable bt = cons2_BitTable (n, 0);

    { BLoop( i, ni )
        Bit x;
        x = set1_BitTable (bt, 3 * i);
        Claim2( x ,==, 0 );
    } BLose()

    { BLoop( i, n )
        Bit x, y;
        x = test_BitTable (bt, i);
        y = (0 == (i % 3));
        Claim2( x ,==, y );
        x = set1_BitTable (bt, i);
        Claim2( x ,==, y );
    } BLose()

    lose_BitTable (&bt);
}

/** \test
 * This mimics the dirty bit in a set associative cache,
 * but is unrealistic since it disregards any values.
 * Now, if all values fall inside [0..255], then we have a useful tool,
 * but then LowBits() would not be tested.
 **/
static
    void
testfn_cache_BitTable ()
{
    uint i;
    Bit flag;
    FixDeclBitTable( cache, 256, 1 );
    const uint nslots = cache.sz;
    const uint nbits = 8;

    UFor( i, nslots )
        Claim( test_BitTable (cache, i) );
    set0_BitTable (cache, 100);
    wipe_BitTable (cache, 0);
    UFor( i, nslots )
        Claim( !test_BitTable (cache, i) );

    i = LowBits( nslots+1, nbits );
    Claim2( i ,==, 1 );
    flag = set1_BitTable (cache, i);
    Claim2( flag ,==, 0 );

    i = LowBits( nslots-1, nbits );
    Claim2( i ,==, nslots-1 );
    flag = set1_BitTable (cache, i);
    Claim2( flag ,==, 0 );

    i = LowBits( 3*(nslots-1), nbits );
    Claim2( i ,==, nslots-3 );
    flag = set1_BitTable (cache, i);
    Claim2( flag ,==, 0 );

    i = LowBits( 5*nslots-3, nbits );
    Claim2( i ,==, nslots-3 );
    flag = set1_BitTable (cache, i);
    Claim2( flag ,==, 1 );
}

/** \test
 * Simple test for the Cons structure.
 * Cons uses reference counting, to know when memory is safe to free.
 **/
    void
testfn_Cons ()
{
    DecloStack1( Sxpn, sx, dflt_Sxpn () );
    Cons* c = req_Sxpn (sx);
    Cons* b = req2_Sxpn (sx, dflt_Cons_ConsAtom (c), 0);
    Cons* a = req1_Sxpn (sx, b);
    OFileB* of = stderr_OFileB ();

    c->car.kind = Cons_AlphaTab;
    c->car.as.alphatab = dflt1_AlphaTab ("c");
    a->car.kind = Cons_AlphaTab;
    a->car.as.alphatab = dflt1_AlphaTab ("a");

    dump_Cons (of, a);
    dump_char_OFileB (of, '\n');
    dump_Cons (of, b);
    dump_char_OFileB (of, '\n');
    dump_Cons (of, c);
    dump_char_OFileB (of, '\n');
    dump_cstr_OFileB (of, "------------\n");

    giv_Sxpn (sx, b);
    giv_Sxpn (sx, a);
    giv_Sxpn (sx, c);

    lose_Sxpn (sx);
}

/** \test
 * Test FileB ability to skip whitespace.
 * This also shows how FileB overlays work.
 **/
    void
testfn_skipws_FileB ()
{
    const char text[] = "hello i am\n some \n text! ";
    const char* const expect_text[] = {
        "hello", "i", "am", "some", "text!"
    };
    uint idx = 0;
    FileB xfb = dflt_FileB ();
    XFileB* xf = &xfb.xo;
    OFileB* of = stderr_OFileB ();

#if 0
    open_FileB (&xfb, "", "test");
#else
    SizeTable (xf->buf, sizeof(text));
    memcpy (xf->buf.s, text, xf->buf.sz);
#endif

    for (char* s = getline_XFileB (xf);
         s;
         s = getline_XFileB (xf))
    {
        XFileB olay = olay_XFileB (xf, IdxEltTable( xf->buf, s ));

        for (s = nextok_XFileB (&olay, 0, 0);
             s;
             s = nextok_XFileB (&olay, 0, 0))
        {
            int cmp_ret;
            Claim2(idx ,<, ArraySz( expect_text ));
            cmp_ret = strcmp(expect_text[idx], s);
            Claim2( 0 ,==, cmp_ret );
            ++ idx;
            dump_cstr_OFileB (of, s);
            dump_char_OFileB (of, '\n');
        }
    }

    lose_XFileB (xf);
    dump_cstr_OFileB (of, "------------\n");
    flush_OFileB (of);
}

/** \test
 * Test the LgTable data structure.
 * Actually, \ref testfn_RBTree() and \ref testfn_Associa() test this better,
 * as they use the structure for memory allocation.
 * This test merely hopes to weed out simple problems before those tests run.
 **/
    void
testfn_LgTable ()
{
    DecloStack1( LgTable, lgt, dflt1_LgTable (sizeof (int)) );
    ujint idx;
    ujint n = 40;

    Claim2( 4 ,==, msb_ujint (4) );
    Claim2( 4 ,==, msb_ujint (5) );
    Claim2( 8 ,==, msb_ujint (13) );

    Claim2( 0 ,==, lg_ujint (0) );
    Claim2( 0 ,==, lg_ujint (1) );
    Claim2( 1 ,==, lg_ujint (2) );
    Claim2( 1 ,==, lg_ujint (3) );
    Claim2( 2 ,==, lg_ujint (4) );
    Claim2( 2 ,==, lg_ujint (4) );

    for (uint i = 0; i < n; ++i)
    {
        int* el = (int*) req_LgTable (lgt);
        *el = - (int) i;
        idx = idxelt_LgTable (lgt, el);
        Claim2( idx ,==, i );
    }

    gividx_LgTable (lgt, 1);
    idx = reqidx_LgTable (lgt);
    Claim2( idx ,==, 1 );

    gividx_LgTable (lgt, 0);
    idx = reqidx_LgTable (lgt);
    Claim2( idx ,==, 0 );

    gividx_LgTable (lgt, 5);
    idx = reqidx_LgTable (lgt);
    Claim2( idx ,==, 5 );

    gividx_LgTable (lgt, 7);
    idx = reqidx_LgTable (lgt);
    Claim2( idx ,==, 7 );

    for (uint i = 0; i < n; ++i)
    {
        ujint sz = n-i;
        Claim2( lgt->allocs.sz ,<=, (ujint) lg_ujint (sz) + 2 );
        gividx_LgTable (lgt, sz-1);
    }

    lose_LgTable (lgt);
}

    void
testfn_OSPc ()
{
    bool good = true;
    const char* s;
    DecloStack1( OSPc, ospc, dflt_OSPc () );
    /* stdxpipe_OSPc (ospc); */
    stdopipe_OSPc (ospc);
    ospc->cmd = cons1_AlphaTab (exename_of_sysCx ());
    PushTable( ospc->args, cons1_AlphaTab ("echo") );
    PushTable( ospc->args, cons1_AlphaTab ("hello") );
    PushTable( ospc->args, cons1_AlphaTab ("world") );
    good = spawn_OSPc (ospc);
    Claim( good );
    /* close_OFileB (ospc->of); */
    load_XFileB (ospc->xf);
    s = cstr_XFileB (ospc->xf);
    /* DBog1( "got: %s", s ); */
    Claim( eql_cstr (s, "hello world\n") );
    lose_OSPc (ospc);
}

/** \test
 * Rigorously test red-black tree with different combinations of insert
 * and remove operations.
 * Set up and tear down a tree of 26 strings (ascii letters) in many different
 * orders. To ensure many orders, use sequential multiples of coprimes with 26
 * to index the array of keys.
 *
 * Ex: The following sequence is generated from the first 26 multiples of 3.\n
 * 0 3 6 9 12 15 18 21 24 1 4 7 10 13 16 19 22 25 2 5 8 11 14 17 20 23
 **/
    void
testfn_RBTree ()
{
    static const char* const keys[] = {
        "a", "b", "c", "d", "e", "f", "g",
        "h", "i", "j", "k", "l", "m", "n",
        "o", "p", "q", "r", "s", "t", "u",
        "v", "w", "x", "y", "z"
    };
    static const uint muls[] = {
        1, 3, 5, 7, 9, 11, 15, 17, 19, 21
    };
    const uint nkeys = ArraySz( keys );
    const uint nmuls = ArraySz( muls );
    TNode sentinel;
    DecloStack1( RBTree, t, dflt2_RBTree (&sentinel.rbt, swapped_TNode) );
    DecloStack1( LgTable, lgt, dflt1_LgTable (sizeof(TNode)) );
    uint n_expect = 0;

    sentinel.key = "sentinel";
    sentinel.val = nkeys;

    { BLoop( mi, nmuls )
        { BLoop( mj, nmuls )
            { BLoop( i, nkeys )
                const uint idx = (muls[mi] * i) % nkeys;
                insert_TNode (t, lgt, keys[idx], idx, &n_expect);
            } BLose()
#if 0
            output_dot (&t->bst);
#endif
            { BLoop( i, nkeys )
                const uint idx = (muls[mj] * i) % nkeys;
                remove_TNode (t, lgt, keys[idx], &n_expect);
            } BLose()
        } BLose()
    } BLose()

    lose_BSTree (&t->bst, lose_TNode);
    lose_LgTable (lgt);
}

static
    void
claim_allocsz_Table (Table* t)
{
    const ujint sz = t->sz;
    const ujint allocsz = allocsz_Table (t);

    Claim2( sz ,<=, allocsz );
    Claim2( sz ,>=, allocsz / 4 );

    if (sz <= allocsz / 2)
    {
        grow_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
        mpop_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }

    if (sz >= allocsz / 2)
    {
        mpop_Table (t, allocsz / 4);
        Claim2( allocsz ,==, allocsz_Table (t) );
        grow_Table (t, allocsz / 4);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }

    if (sz < allocsz / 2 && sz > 0)
    {
        mpop_Table (t, CeilQuot( sz, 2 ));
        Claim2( allocsz / 2 ,==, allocsz_Table (t) );
        grow_Table (t, CeilQuot( sz, 2 ));
        Claim2( allocsz / 2 ,==, allocsz_Table (t) );

        /* Get allocsz back.*/
        grow_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
        mpop_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }
    else if (sz > allocsz / 2)
    {
        grow_Table (t, sz);
        Claim2( allocsz * 2 ,==, allocsz_Table (t) );
        mpop_Table (t, sz);
        Claim2( allocsz * 2 ,==, allocsz_Table (t) );

        /* Get allocsz back.*/
        mpop_Table (t, sz / 2 + 1);
        Claim2( allocsz ,==, allocsz_Table (t) );
        grow_Table (t, sz / 2 + 1);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }

    Claim2( sz ,==, t->sz );
    Claim2( allocsz ,==, allocsz_Table (t));
}

/** \test
 * Rigorously test Table structure.
 * At each push/pop step, assertions are made about when the table should
 * resize. This ensures the Table has proper a proper amortized constant
 * cost.
 **/
    void
testfn_Table ()
{
    const int val = 7;
    uint n = (1 << 12) + 1;
    DeclTableT( V, int );
    DeclTable( V, t );
    Table tmp_table;

    tmp_table = MakeCastTable( t );
    claim_allocsz_Table (&tmp_table);
    XferCastTable( t, tmp_table );

    for (uint i = 0; i < n; ++i)
    {
        DeclGrow1Table( V, x, t );
        *x = (int) i;

        tmp_table = MakeCastTable( t );
        claim_allocsz_Table (&tmp_table);
        XferCastTable( t, tmp_table );
    }

    PackTable( t );
    Claim2( t.sz - 1 ,==, AllocszTable( t ));

    for (uint i = 0; i < n; ++i)
    {
        t.s[t.sz-1] = val;
        MPopTable( t, 1 );

        tmp_table = MakeCastTable( t );
        claim_allocsz_Table (&tmp_table);
        XferCastTable( t, tmp_table );
    }

    Claim2( 0 ,==, t.sz );
    Claim2( 0 ,<, AllocszTable( t ));
    PackTable( t );
    Claim2( 0 ,==, AllocszTable( t ));
    InitTable( t );

    PushTable( t, val );
    Claim2( 1 ,==, t.sz );
    PackTable( t );
    PushTable( t, val );
    GrowTable( t, 10 );
    MPopTable( t, 12 );


    PushTable( t, val );
    PushTable( t, val );
    PushTable( t, val );
    Claim2( 3 ,==, t.sz );
    LoseTable( t );
}

int main (int argc, char** argv)
{
    int argi =
        (init_sysCx (&argc, &argv),
         1);

    // Special test as child process.
    if (eql_cstr (argv[argi], "echo"))
    {
        OFileB* of = stdout_OFileB ();
        for (argi += 1; argi < argc; ++argi)
        {
            dump_cstr_OFileB (of, argv[argi]);
            dump_char_OFileB (of, (argi + 1 < argc) ? ' ' : '\n');
        }
        lose_sysCx ();
        return 0;
    }

    testfn_Table ();
    testfn_BitTable ();
    testfn_cache_BitTable ();
    testfn_LgTable ();
    testfn_Cons ();
    testfn_skipws_FileB ();
    testfn_RBTree ();
    testfn_Associa ();
    testfn_OSPc ();

    lose_sysCx ();
    return 0;
}

