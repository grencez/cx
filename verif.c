
#include "bittable.h"
#include "fileb.h"
#include "rbtree.h"
#include "sys-cx.h"
#include "table.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

    bool
eql_cstr (const char* a, const char* b)
{
    return (0 == strcmp (a, b));
}

typedef struct TNode TNode;
struct TNode
{
    RBTNode rbt;
    char* key;
    uint val;
};

    Trit
swapped_TNode (const BSTNode* lhs, const BSTNode* rhs)
{
    const TNode* a = CastUp( TNode, rbt, CastUp( RBTNode, bst, lhs ) );
    const TNode* b = CastUp( TNode, rbt, CastUp( RBTNode, bst, rhs ) );
    int ret = strcmp (a->key, b->key);

        /* fprintf (stderr, "%s   %s\n", a->key, b->key); */

    if (ret < 0)  return Nil;
    if (ret > 0)  return Yes;
    return May;
}

    void
lose_TNode (BSTNode* x)
{
    TNode* a = CastUp( TNode, rbt, CastUp( RBTNode, bst, x ) );
    free (a->key);
    free (a);
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
    Claim2( x ,==, x->joint->split[side_BSTNode (x)]);
    
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
    traverse_BSTree (t, Yes, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
    n_result = 0;
    traverse_BSTree (t, Nil, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
    n_result = 0;
    traverse_BSTree (t, May, claim_TNode, args);
    Claim2( n_expect ,==, n_result );
}

static
    void
insert_TNode (RBTree* t, const char* key, uint val, uint* n_expect)
{
    DeclAlloc( TNode, a, 1 );
    a->key = dup_cstr (key);
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
    DecloStack( FileB, out );
    OFileB* of = &out->xo;
    args[0] = of;

    init_FileB (out);
    seto_FileB (out, true);
    open_FileB (out, "", "out.dot");

    dump_cstr_OFileB (of, "digraph tree {\n");
    output_dot_fn (t->sentinel, args);
    traverse_BSTree (t, Yes, output_dot_fn, args);
    dump_cstr_OFileB (of, "}\n");
    lose_FileB (out);
}

static
    TNode*
find_TNode (RBTree* t, const char* s)
{
    TNode a;
    BSTNode* x;
    a.key = (char*) s;
    x = find_BSTree (&t->bst, &a.rbt.bst);
    if (!x)  return 0;
    return CastUp( TNode, rbt, CastUp( RBTNode, bst, x ) );
}

static
    void
remove_TNode (RBTree* t, const char* key, uint* n_expect)
{
    TNode* a = find_TNode (t, key);
    Claim( a );
    remove_RBTree (t, &a->rbt);
    lose_TNode (&a->rbt.bst);
    *n_expect -= 1;
    claim_BSTree (&t->bst, *n_expect);
}

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

    /* This mimics the dirty bit in a set associative cache,
     * but is unrealistic since it disregards any values.
     * Now, if all values fall inside [0..255], then we have a useful tool,
     * but then LowBits() would not be tested.
     */
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

    void
testfn_skipws_FileB ()
{
    const char text[] = "hello i am\n some \n text! ";
    const char* const expect_text[] = {
        "hello", "i", "am", "some", "text!"
    };
    uint idx = 0;
    DecloStack( FileB, in );
    OFileB* of = stderr_OFileB ();

    init_FileB (in);
#if 0
    open_FileB (in, "", "test");
    in->f = fopen ("test", "rb");
#else
    SizeTable (in->xo.buf, sizeof(text));
    memcpy (in->xo.buf.s, text, in->xo.buf.sz);
#endif

    while ((getline_FileB (in)))
    {
        DecloStack( FileB, olay );
        char* s;

        init_FileB (olay);
        olay_FileB (&olay->xo, in);
        while ((s = nextok_FileB (olay, 0, 0)))
        {
            Claim2(idx ,<, ArraySz( expect_text ));
            Claim2(0 ,==, strcmp(expect_text[idx], s));
            ++ idx;
            dump_cstr_OFileB (of, s);
            dump_char_OFileB (of, '\n');
        }
    }

    lose_FileB (in);
    dump_cstr_OFileB (of, "------------\n");
    flush_OFileB (of);
}

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
    DecloStack( RBTree, t );
    uint n_expect = 0;

    init_RBTree (t, &sentinel.rbt, swapped_TNode);
    sentinel.key = (char*) "sentinel";
    sentinel.val = nkeys;

    { BLoop( mi, nmuls )
        { BLoop( mj, nmuls )
            { BLoop( i, nkeys )
                const uint idx = (muls[mi] * i) % nkeys;
                insert_TNode (t, keys[idx], idx, &n_expect);
            } BLose()
#if 0
            output_dot (&t->bst);
#endif
            { BLoop( i, ArraySz( keys ) )
                const uint idx = (muls[mj] * i) % nkeys;
                remove_TNode (t, keys[idx], &n_expect);
            } BLose()
        } BLose()
    } BLose()

    lose_BSTree (&t->bst, lose_TNode);
}

static
    void
claim_allocsz_Table (Table* t)
{
    const TableSz sz = t->sz;
    const TableSz allocsz = allocsz_Table (t);

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

    { BLoop( i, n )
        DeclGrow1Table( V, x, t );
        *x = (int) i;

        tmp_table = MakeCastTable( t );
        claim_allocsz_Table (&tmp_table);
        XferCastTable( t, tmp_table );
    } BLose()

    PackTable( t );
    Claim2( t.sz - 1 ,==, AllocszTable( t ));

    { BLoop( i, n )
        t.s[t.sz-1] = val;
        MPopTable( t, 1 );

        tmp_table = MakeCastTable( t );
        claim_allocsz_Table (&tmp_table);
        XferCastTable( t, tmp_table );
    } BLose()

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

int main ()
{
    init_sys_cx ();

    testfn_BitTable ();
    testfn_cache_BitTable ();
    testfn_skipws_FileB ();
    testfn_RBTree ();
    testfn_Table ();

    lose_sys_cx ();
    return 0;
}

