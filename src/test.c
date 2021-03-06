/**
 * \file test.c
 * Tests for cx.
 * Be sure to run this through valgrind sometimes, it should not leak!
 **/
#define TestClaim

#include "cx/syscx.h"
#include "cx/associa.h"
#include "cx/bittable.h"
#include "cx/fileb.h"
#include "cx/lgtable.h"
#include "cx/ospc.h"
#include "cx/rbtree.h"
#include "cx/sxpn.h"
#include "cx/table.h"
#include "testcxx.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static
  void
oput_testsep (const char* name)
{
  OFile* of = stderr_OFile ();
  oput_cstr_OFile (of, "-------");
  {:if (name)
    oput_char_OFile (of, ' ');
    oput_cstr_OFile (of, name);
    oput_char_OFile (of, ' ');
  }
  oput_cstr_OFile (of, "-------\n");
}

typedef struct TNode TNode;
struct TNode
{
    RBTNode rbt;
    const char* key;
    uint val;
};

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
    {:do
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

    {:if (b->red)
        b = CastUp( RBTNode, bst, x->joint );
        Claim( !b->red );
    }
    {:if (!x->split[0] || !x->split[1])
        uint c = countup_black_RBTNode (b);
        if (*nblack == UINT_MAX)
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
    uint nblack = UINT_MAX;
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
    TNode* a = (TNode*) take_LgTable (lgt);
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
  OFile* of = (OFile*) ((void**)args)[0];

  printf_OFile (of, "q%u [label = \"%s\", color = \"%s\"];\n",
                a->val,
                a->key,
                (a->rbt.red) ? "red" : "black");

  {:if (x->joint)
    TNode* b = CastUp( TNode, rbt, CastUp( RBTNode, bst, x->joint ) );
    printf_OFile (of, "q%u -> q%u;\n", b->val, a->val);
  }
  flush_OFile (of);
}

  void
output_dot (BSTree* t)
{
  void* args[1];
  OFileB ofb[] = default;
  OFile* of = &ofb->of;
  args[0] = of;

  open_FileB (&ofb->fb, "", "out.dot");

  oput_cstr_OFile (of, "digraph tree {\n");
  output_dot_fn (t->sentinel, args);
  walk_BSTree (t, Yes, output_dot_fn, args);
  oput_cstr_OFile (of, "}\n");
  lose_OFileB (ofb);
}

static
    TNode*
find_TNode (RBTree* t, const char* s)
{
    BSTNode* x = find_BSTree (&t->bst, &s);
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
    give_LgTable (lgt, a);
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
    Associa map[1];
    uint n_expect = 1; /* Sentinel node.*/

    InitAssocia( AlphaTab, uint, *map, cmp_AlphaTab );

    Claim2( map->nodes.sz ,==, n_expect );
    {:for (mi ; nmuls)
        {:for (mj ; nmuls)
            {:for (i ; nkeys)
                const uint idx = (muls[mi] * i) % nkeys;
                const AlphaTab key = dflt1_AlphaTab (keys[idx]);
                {:if (mj % 2 == 0)
                    insert_Associa (map, &key, &idx);
                }
                {:else
                    bool added = false;
                    Assoc* assoc = ensure1_Associa (map, &key, &added);
                    Claim( added );
                    val_fo_Assoc (map, assoc, &idx);
                }
                ++ n_expect;
                Claim2( map->nodes.sz ,==, n_expect );
            }


            {:for (i ; nkeys)
                const uint idx = (muls[mj] * i) % nkeys;
                const AlphaTab key = dflt1_AlphaTab (keys[idx]);
                Assoc* a;
                {:if (mj % 2 == 0)
                    a = lookup_Associa (map, &key);
                }
                {:else
                    bool added = true;
                    a = ensure1_Associa (map, &key, &added);
                    Claim( !added );
                }
                Claim( a );
                {
                    uint val = *(uint*) val_of_Assoc (map, a);
                    Claim2( idx ,==, val );
                }
                give_Associa (map, a);
                -- n_expect;
                Claim2( map->nodes.sz ,==, n_expect );
            }
        }
    }

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

  {:for (i ; ni)
    Bit x;
    x = set1_BitTable (bt, 3 * i);
    Claim2( x ,==, 0 );
  }

  {:for (i ; n)
    Bit x, y;
    x = test_BitTable (bt, i);
    y = (0 == (i % 3));
    Claim2( x ,==, y );
    x = set1_BitTable (bt, i);
    Claim2( x ,==, y );
  }

  {
    Claim2( (1<<4) | (1<<3) ,==, BitMaskT(uint, 3, 2) );
    Claim2( (1<<5) | (1<<4) | (1<<3) ,==, BitMaskT(uint, 3, 3) );
    set_uint_BitTable(bt, 3, 3, 5);
    Claim2( 5 ,==, get_uint_BitTable(bt, 3, 3) );
    Claim2( 1 ,==, get_uint_BitTable(bt, 3, 2) );
  }

  {
    const uint idx = INT_BIT - 2;
    const uint x = 100;
    set_uint_BitTable(bt, idx, 5, x);
    Claim2( x & 7        ,==, get_uint_BitTable(bt, idx  , 3) );
    Claim2( (x >> 1) & 7 ,==, get_uint_BitTable(bt, idx+1, 3) );
    Claim2( x & 15       ,==, get_uint_BitTable(bt, idx  , 4) );
    Claim2( x & 31       ,==, get_uint_BitTable(bt, idx  , 5) );
  }

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
  Sxpn sx[] = default;
  Cons* c = take_Sxpn (sx);
  Cons* b = take2_Sxpn (sx, dflt_Cons_ConsAtom (c), 0);
  Cons* a = take1_Sxpn (sx, b);
  OFile* of = stderr_OFile ();
  oput_testsep (0);

  c->car.kind = Cons_AlphaTab;
  c->car.as.alphatab = dflt1_AlphaTab ("c");
  a->car.kind = Cons_AlphaTab;
  a->car.as.alphatab = dflt1_AlphaTab ("a");

  oput_Cons (of, a);
  oput_char_OFile (of, '\n');
  oput_Cons (of, b);
  oput_char_OFile (of, '\n');
  oput_Cons (of, c);
  oput_char_OFile (of, '\n');
  oput_testsep (0);

  give_Sxpn (sx, b);
  give_Sxpn (sx, a);
  give_Sxpn (sx, c);

  Claim2( sx->cells.sz ,==, 0 );
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
  XFileB xfb[] = default;
  XFile* xf = &xfb->xf;
  OFile* of = stderr_OFile ();

#if 0
  open_FileB (&xfb, "", "test");
#else
  SizeTable (xf->buf, sizeof(text));
  memcpy (xf->buf.s, text, xf->buf.sz);
#endif

  for (char* s = getline_XFile (xf);
       s;
       s = getline_XFile (xf))
  {
    XFile olay[1];
    olay_txt_XFile (olay, xf, IdxEltTable( xf->buf, s ));

    for (s = nextok_XFile (olay, 0, 0);
         s;
         s = nextok_XFile (olay, 0, 0))
    {
      oput_cstr_OFile (of, s);
      oput_char_OFile (of, '\n');
      flush_OFile (of);
      Claim2(idx ,<, ArraySz( expect_text ));
      Claim( eq_cstr (expect_text[idx], s) );
      ++ idx;
    }
  }

  lose_XFileB (xfb);
  oput_cstr_OFile (of, "------------\n");
  flush_OFile (of);
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
  zuint idx;
  zuint n = 40;

  Claim2( 4 ,==, msb_luint (4) );
  Claim2( 4 ,==, msb_luint (5) );
  Claim2( 8 ,==, msb_luint (13) );

  Claim2( 0 ,==, lg_luint (0) );
  Claim2( 0 ,==, lg_luint (1) );
  Claim2( 1 ,==, lg_luint (2) );
  Claim2( 1 ,==, lg_luint (3) );
  Claim2( 2 ,==, lg_luint (4) );
  Claim2( 2 ,==, lg_luint (4) );

  {:for (i ; n)
    int* el = (int*) take_LgTable (lgt);
    *el = - (int) i;
    idx = idxelt_LgTable (lgt, el);
    Claim2( idx ,==, i );
  }

  giveidx_LgTable (lgt, 1);
  idx = takeidx_LgTable (lgt);
  Claim2( idx ,==, 1 );

  giveidx_LgTable (lgt, 0);
  idx = takeidx_LgTable (lgt);
  Claim2( idx ,==, 0 );

  giveidx_LgTable (lgt, 5);
  idx = takeidx_LgTable (lgt);
  Claim2( idx ,==, 5 );

  giveidx_LgTable (lgt, 7);
  idx = takeidx_LgTable (lgt);
  Claim2( idx ,==, 7 );

  {:for (i ; n)
    zuint sz = n-i;
    Claim2( lgt->allocs.sz ,<=, (zuint) lg_luint (sz) + 2 );
    giveidx_LgTable (lgt, sz-1);
  }

  lose_LgTable (lgt);
}

/** \test
 * Stress test of the LgTable data structure.
 **/
static
  void
testfn_stress_LgTable ()
{
  const uint n = 1e5;
  LgTable lgt[1];
  *lgt = dflt1_LgTable (sizeof(int));

  {:for (i ; n)
    uint nallocs;
    uint idx;
    idx = takeidx_LgTable (lgt);
    Claim2( idx ,==, i );
    nallocs = lgt->allocs.sz;
    giveidx_LgTable (lgt, i);
    Claim2( nallocs ,==, lgt->allocs.sz );
    idx = takeidx_LgTable (lgt);
    Claim2( idx ,==, i );
    Claim2( nallocs ,==, lgt->allocs.sz );
  }

  {:for (i ; n)
    giveidx_LgTable (lgt, n-i-1);
    if (lgt->sz >= 2)
      Claim2( 8 * lgt->sz / 3 ,>, allocsz_of_LgTable (lgt) );
    else
      Claim2( allocsz_of_LgTable (lgt) ,==, 4);

    if (i > n / 2) {
      Claim2( 4 * lgt->sz / 3 ,<=, allocsz_of_LgTable (lgt) );
    }
  }
  Claim2( lgt->sz ,==, 0 );
  Claim2( allocsz_of_LgTable (lgt) ,==, 4 );

  {:for (i ; n)
    takeidx_LgTable (lgt);
  }
  {:for (i ; n)
    giveidx_LgTable (lgt, i);
  }
  Claim2( lgt->sz ,==, 0 );
  Claim2( allocsz_of_LgTable (lgt) ,==, 4 );

  lose_LgTable (lgt);
}

    void
testfn_OSPc ()
{
    bool good = true;
    const char* s;
    OSPc ospc[] = default;
    /* stdxpipe_OSPc (ospc); */
    stdopipe_OSPc (ospc);
    ospc->cmd = cons1_AlphaTab (exename_of_sysCx ());
    PushTable( ospc->args, cons1_AlphaTab ("echo") );
    PushTable( ospc->args, cons1_AlphaTab ("hello") );
    PushTable( ospc->args, cons1_AlphaTab ("world") );
    good = spawn_OSPc (ospc);
    Claim( good );
    /* close_OFileB (ospc->of); */
    xget_XFile (ospc->xf);
    s = cstr_XFile (ospc->xf);
    /* DBog1( "got: %s", s ); */
    Claim( eql_cstr (s, "hello world\n") );
    good = close_OSPc (ospc);
    Claim( good );
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
    PosetCmp cmp =
      dflt3_PosetCmp (offsetof( TNode, rbt ),
                      offsetof( TNode, key ),
                      (PosetCmpFn) cmp_cstr_loc);
    DecloStack1( RBTree, t, dflt2_RBTree (&sentinel.rbt, cmp) );
    DecloStack1( LgTable, lgt, dflt1_LgTable (sizeof(TNode)) );
    uint n_expect = 0;

    sentinel.key = "sentinel";
    sentinel.val = nkeys;

    {:for (mi ; nmuls)
        {:for (mj ; nmuls)
            {:for (i ; nkeys)
                const uint idx = (muls[mi] * i) % nkeys;
                insert_TNode (t, lgt, keys[idx], idx, &n_expect);
            }
#if 0
            output_dot (&t->bst);
#endif
            {:for (i ; nkeys)
                const uint idx = (muls[mj] * i) % nkeys;
                remove_TNode (t, lgt, keys[idx], &n_expect);
            }
        }
    }

    lose_BSTree (&t->bst, lose_TNode);
    lose_LgTable (lgt);
}

static
    void
claim_allocsz_Table (Table* t)
{
    const zuint sz = t->sz;
    const zuint allocsz = allocsz_Table (t);

    Claim2( sz ,<=, allocsz );
    Claim2( sz ,>=, allocsz / 4 );

    {:if (sz <= allocsz / 2)
        grow_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
        mpop_Table (t, allocsz / 2);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }

    {:if (sz >= allocsz / 2)
        mpop_Table (t, allocsz / 4);
        Claim2( allocsz ,==, allocsz_Table (t) );
        grow_Table (t, allocsz / 4);
        Claim2( allocsz ,==, allocsz_Table (t) );
    }

    {:if (sz < allocsz / 2 && sz > 0)
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
    {:else if (sz > allocsz / 2)
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

    {:for (uint i = 0; i < n; ++i)
        DeclGrow1Table( V, x, t );
        *x = (int) i;

        tmp_table = MakeCastTable( t );
        claim_allocsz_Table (&tmp_table);
        XferCastTable( t, tmp_table );
    }

    PackTable( t );
    Claim2( t.sz - 1 ,==, AllocszTable( t ));

    {:for (uint i = 0; i < n; ++i)
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

static
    void
testfn_exec ()
{
    pid_t pid = -1;
    int status = 1;
    const char* argv[4];
    bool good = false;
    argv[0] = exename_of_sysCx ();
    argv[1] = "wait0";
    argv[2] = "5"; // Special exit code.
    argv[3] = 0;

    fputs ("V spawn() called V\n", stderr);
    fflush (stderr);
    pid = spawnvp_sysCx ((char**) argv);

    good = waitpid_sysCx (pid, &status);
    fputs ("^ wait() returned ^\n", stderr);
    fflush (stderr);
    Claim( good );
    Claim2( status ,==, atoi (argv[2]) );
}

static
  void
testfn_pathname ()
{
  typedef struct TestCase TestCase;
  struct TestCase {
    const char* opt_dir;
    const char* filename;
    const char* expect;
  };
  const TestCase tests[] =
  {  { "my/path", "/oh/no/abs/file.txt", "/oh/no/abs/file.txt" }
    ,{ "my/path", "oh/no/abs/file.txt", "my/path/oh/no/abs/file.txt" }
    ,{ 0, "path/to/file.txt", "path/to/file.txt" }
    ,{ 0, "file.txt", "file.txt" }
    ,{ "", "file.txt", "file.txt" }
    ,{ "path", "file.txt", "path/file.txt" }
    ,{ "path/", "file.txt", "path/file.txt" }
    ,{ "/", "path/to/file.txt", "/path/to/file.txt" }
    ,{ "/path", "to/file.txt", "/path/to/file.txt" }
    ,{ "/path/", "to/file.txt", "/path/to/file.txt" }
  };

  for (i ; ArraySz( tests )) {
    const TestCase testcase = tests[i];
    AlphaTab result = default;
    uint sepidx =
      pathname2_AlphaTab (&result, testcase.opt_dir, testcase.filename);

    if (!eq_cstr (result.s, testcase.expect) ) {
      fprintf (stderr, "opt_dir: %s  filename: %s  expect: %s  result: %s\n",
               testcase.opt_dir ? "(NULL)" : testcase.opt_dir,
               testcase.filename,
               testcase.expect,
               result.s);
      Claim( 0 );
    }

    if (sepidx != 0 && '/' != result.s[sepidx-1]) {
      result.s[sepidx-1] = '\0';
      fprintf (stderr, "dir:%s  file:%s\n",
               result.s,
               &result.s[sepidx]);
      Claim2( '/' ,==, result.s[sepidx-1] );
    }

    lose_AlphaTab (&result);
  }
}

static
  void
testfn_dirname ()
{
  typedef struct TestCase TestCase;
  struct TestCase {
    const char* input;
    const char* expect;
  };
  const TestCase tests[] =
  {  { "path/to/file", "./path/to/" }
    ,{ "./path/to/file", "./path/to/" }
    ,{ "/path/to/file", "/path/to/" }
    ,{ "/", "/" }
    ,{ "./", "./" }
    ,{ "", "./" }
  };

  for (i ; ArraySz( tests )) {
    const TestCase testcase = tests[i];
    AlphaTab path = dflt1_AlphaTab (testcase.input);
    AlphaTab result = default;

    dirname_AlphaTab (&result, &path);
    if (!eq_cstr (result.s, testcase.expect) ) {
      fprintf (stderr, "input: %s  expect: %s  result: %s\n",
               testcase.input,
               testcase.expect,
               result.s);
      Claim( 0 );
    }
    lose_AlphaTab (&result);
  }
}

static
  void
Test(const char testname[])
{
  void (*fn) () = 0;

  /* cswitch testname
   *   -case-pfx "fn = testfn_"
   *   -array AllTests
   *   -x testlist.txt
   *   -o test-dep/switch.c
   */
#include "test-dep/switch.c"

  if (fn) {
    fn();
  }
  else if (!testname[0]) {
    for (uint i = 0; i < ArraySz(AllTests); ++i) {
      Test(AllTests[i]);
    }
  }
  else {
    Claim( 0 && "Test does not exist." );
  }
}


int main(int argc, char** argv)
{
  int argi = init_sysCx (&argc, &argv);

  // Special test as child process.
  {:if (eql_cstr (argv[argi], "echo"))
    OFile* of = stdout_OFile ();
    {:for (argi += 1; argi < argc; ++argi)
      oput_cstr_OFile (of, argv[argi]);
      oput_char_OFile (of, (argi + 1 < argc) ? ' ' : '\n');
    }
    lose_sysCx ();
    return 0;
  }
  {:if (eql_cstr (argv[argi], "wait0"))
    argv[argi] = dup_cstr ("wait1");
    fputs (" V exec() called V\n", stderr);
    execvp_sysCx (argv);
    fputs (" ^ exec() failed? ^\n", stderr);
    return 1;
  }
  {:if (eql_cstr (argv[argi], "wait1"))
    /* _sleep (1); */
    /* sleep (1); */
    fputs ("  V exec()'d process exits V\n", stderr);
    return atoi (argv[argi+1]);
  }

  if (argi == argc) {
    Test("");
  }
  else {
    while (argi < argc) {
      Test(argv[argi++]);
    }
  }

  oput_testsep (0);
  lose_sysCx ();
  return 0;
}


