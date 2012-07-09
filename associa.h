
#ifndef Associa_H_
#define Associa_H_
#include "rbtree.h"
#include "table.h"

typedef struct Associa Associa;
typedef struct Assoc Assoc;

#define DeclTableT_Assoc
DeclTableT( Assoc, Assoc );

struct Assoc
{
    RBTNode rbt;
    union Assoc_union {
        Associa* map;
        void* key;
    } u;
};

struct Associa
{
    Table keys;
    Table vals;
    TableT(Assoc) nodes;
    Trit (* swapped) (const void* lhs, const void* rhs);
    RBTree rbt;
    Assoc sentinel;
    bool ensize;
};

static
Trit swapped_Assoc (const BSTNode* lhs_bst, const BSTNode* rhs_bst);

qual_inline
    void
init3_Associa (Associa* map, size_t keysz, size_t valsz,
               Trit (* swapped) (const void* lhs, const void* rhs))
{
    init1_Table (&map->keys, keysz);
    init1_Table (&map->vals, valsz);
    InitTable( map->nodes );
    map->swapped = swapped;
    init_RBTree (&map->rbt, &map->sentinel.rbt, swapped_Assoc);
    map->ensize = false;
}

qual_inline
    void
lose_Associa (Associa* map)
{
    lose_Table (&map->keys);
    lose_Table (&map->vals);
    LoseTable( map->nodes );
}

qual_inline
    Associa*
map_of_Assoc (Assoc* a)
{
    return (a->rbt.bst.joint ? a->u.map : 0);
}

qual_inline
    void*
key_of_Assoc (Assoc* a)
{
    Associa* map = map_of_Assoc (a);
    return (map
            ? elt_Table (&map->keys, IdxEltTable( map->nodes, a ))
            : a->u.key);
}

qual_inline
    void*
val_of_Assoc (Assoc* a)
{
    Associa* map = map_of_Assoc (a);
    return elt_Table (&map->vals, IdxEltTable( map->nodes, a ));
}

qual_inline
    void
key_fo_Assoc (Assoc* a, const void* key)
{
    Associa* map = map_of_Assoc (a);
    if (map)
    {
        void* p = key_of_Assoc (a);
        memcpy (p, key, map->keys.elsz);
    }
    else
    {
        a->u.key = (void*) key;
    }
}

qual_inline
    void
val_fo_Assoc (Assoc* a, const void* val)
{
    Associa* map = map_of_Assoc (a);
    void* p = val_of_Assoc (a);
    if (val)  memcpy (p, val, map->vals.elsz);
    else      memset (p,   0, map->vals.elsz);
}

    /** Check if two Assoc nodes are swapped in order.
     * One of them must be in the tree.
     **/
    Trit
swapped_Assoc (const BSTNode* lhs_bst, const BSTNode* rhs_bst)
{
    Assoc* lhs =
        CastUp( Assoc, rbt, CastUp( RBTNode, bst, lhs_bst ) );
    Assoc* rhs =
        CastUp( Assoc, rbt, CastUp( RBTNode, bst, rhs_bst ) );
    Associa* map = (lhs->rbt.bst.joint ? lhs->u.map : rhs->u.map);

    return map->swapped (key_of_Assoc (lhs),
                         key_of_Assoc (rhs));
}

qual_inline
    void
fixlinks_Assoc (Assoc* node, ptrdiff_t diff)
{
    BSTNode* a = &node->rbt.bst;
    if (a->joint)
        a->joint = (BSTNode*) (diff + (ptrdiff_t) a->joint);
    if (a->split[0])
        a->split[0] = (BSTNode*) (diff + (ptrdiff_t) a->split[0]);
    if (a->split[1])
        a->split[1] = (BSTNode*) (diff + (ptrdiff_t) a->split[1]);
}

qual_inline
    void
fixlinks_Associa (Associa* map, const void* old)
{
    ptrdiff_t diff = (ptrdiff_t) map->nodes.s - (ptrdiff_t) old;
    BSTNode* root;
    ujint i;
    if (diff == 0)  return;

    for (i = 0; i < map->nodes.sz; ++i)
        fixlinks_Assoc (&map->nodes.s[i], diff);
    fixlinks_Assoc (&map->sentinel, diff);

        /* Sentinel did not move since it is not in the node table.*/
    root = root_of_BSTree (&map->rbt.bst);
    if (root)  root->joint = &map->sentinel.rbt.bst;
}

    /**
     * Don't use this directly.
     * See: insert_Associa()
     * See: ensure_Associa()
     **/
qual_inline
    Assoc*
acqu_Assoc (Associa* map)
{
    Assoc* a;
    Assoc* old = map->nodes.s;
    a = Grow1Table( map->nodes );
    -- map->nodes.sz;
    fixlinks_Associa (map, old);
    ++ map->nodes.sz;

    ensize_Table (&map->keys, map->nodes.sz);
    ensize_Table (&map->vals, map->nodes.sz);
    a->rbt.bst.joint = &map->sentinel.rbt.bst;
    a->u.map = map;
    return a;
}

    /** Use /a/ to represent /b/.
     * This is used before invalidating /b/'s memory.
     * See: lose_Assoc()
     * See: ensure_Assoc()
     **/
qual_inline
    void
plac_Assoc (Assoc* a, Assoc* b, const void* key, const void* val)
{
    BSTNode* aa;
    BSTNode* bb;
    if (a == b)  return;
    a->rbt = b->rbt;
    aa = &a->rbt.bst;
    bb = &b->rbt.bst;
    join_BSTNode (aa->joint, aa, side_BSTNode (bb));
    join_BSTNode (aa, aa->split[0], 0);
    join_BSTNode (aa, aa->split[1], 1);
    key_fo_Assoc (a, key);
    val_fo_Assoc (a, val);
}

qual_inline
    void
lose_Assoc (Assoc* a)
{
    Associa* map = map_of_Assoc (a);
    Assoc* b;
    remove_RBTree (&map->rbt, &a->rbt);
    b = &map->nodes.s[map->nodes.sz-1];
    plac_Assoc (a, b, key_of_Assoc (b), val_of_Assoc (b));

    if (!map->ensize)
    {
        Assoc* old = map->nodes.s;
        MPopTable( map->nodes, 1 );
        fixlinks_Associa (map, old);
        size_Table (&map->keys, map->nodes.sz);
        size_Table (&map->vals, map->nodes.sz);
    }
    else
    {
        -- map->nodes.sz;
        map->keys.sz = map->nodes.sz;
        map->vals.sz = map->nodes.sz;
    }
}

qual_inline
    void
insert_Associa (Associa* map, const void* key, const void* val)
{
    Assoc* a = acqu_Assoc (map);
    key_fo_Assoc (a, key);
    val_fo_Assoc (a, val);
    insert_RBTree (&map->rbt, &a->rbt);
}

qual_inline
    Assoc*
lookup_Associa (Associa* map, const void* key)
{
    Assoc a;
    BSTNode* bst;
    a.rbt.bst.joint = 0;
    key_fo_Assoc (&a, key);
    bst = find_BSTree (&map->rbt.bst, &a.rbt.bst);
    if (!bst)  return 0;
    return CastUp( Assoc, rbt, CastUp( RBTNode, bst, bst ) );
}

qual_inline
    Assoc*
ensure_Associa (Associa* map, const void* key)
{
    Assoc x;
    Assoc* a = &x;
    a->rbt.bst.joint = 0;
    key_fo_Assoc (a, key);
    {
        RBTNode* rbt = ensure_RBTree (&map->rbt, &a->rbt);
        a = CastUp( Assoc, rbt, rbt );
    }
    if (a == &x)
    {
        a = acqu_Assoc (map);
        plac_Assoc (a, &x, key, 0);
    }
    return a;
}

qual_inline
    void
remove_Associa (Associa* map, const void* key)
{
    Assoc* a = lookup_Associa (map, key);
    if (a)  lose_Assoc (a);
}

#endif

