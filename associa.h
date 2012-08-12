/**
 * \file associa.h
 * Associative array implementation.
 **/
#ifndef Associa_H_
#define Associa_H_
#include "lgtable.h"
#include "rbtree.h"

typedef struct Assoc Assoc;
typedef struct Associa Associa;

/** Associative array element (association).**/
struct Assoc
{
    RBTNode rbt;
    Associa* map;
};

/** Associative array.**/
struct Associa
{
    LgTable nodes;
    SwappedFn swapped;
    RBTree rbt;
    size_t key_sz;
    size_t val_sz;
    size_t assoc_offset;
    size_t key_offset;
    size_t val_offset;
};

/** Create a new associative array.
 * \param K  Type of key.
 * \param V  Type of value.
 * \param name  Name of variable to create.
 * \param swapped  \ref SwappedFn which acts on keys.
 **/
#define DeclAssocia( K, V, name, swapped ) \
    struct Assoc_##name \
    { \
        Assoc assoc; \
        K key; \
        V val; \
    }; \
    typedef struct Assoc_##name Assoc_##name; \
    Associa stacked_##name = \
        cons7_Associa (swapped, \
                       sizeof(Assoc_##name), \
                       sizeof(K), \
                       sizeof(V), \
                       offsetof( Assoc_##name, assoc ), \
                       offsetof( Assoc_##name, key ), \
                       offsetof( Assoc_##name, val )); \
    Associa* const name = &stacked_##name

static
Trit swapped_Assoc (const BSTNode* lhs_bst, const BSTNode* rhs_bst);

/** Construct an associative array.
 * Don't use this directly.
 * \sa DeclAssocia()
 **/
qual_inline
    Associa
cons7_Associa (SwappedFn swapped,
               size_t node_sz,
               size_t key_sz,
               size_t val_sz,
               size_t assoc_offset,
               size_t key_offset,
               size_t val_offset)
{
    Associa map;
    map.nodes = dflt1_LgTable (node_sz);
    map.swapped      = swapped;
    map.key_sz       =  key_sz;
    map.val_sz       =  val_sz;
    map.assoc_offset =  assoc_offset;
    map.key_offset   =  key_offset;
    map.val_offset   =  val_offset;

    {
        void* node = req_LgTable (&map.nodes);
        Assoc* assoc = (Assoc*) ((size_t) node + map.assoc_offset);
        map.rbt = dflt2_RBTree (&assoc->rbt, swapped_Assoc);
    }
    return map;
}

/** Free everything.**/
qual_inline
    void
lose_Associa (Associa* map)
{
    lose_LgTable (&map->nodes);
}

/** Get the map (associative array) to which the association belongs.**/
qual_inline
    Associa*
map_of_Assoc (Assoc* a)
{
    return a->map;
}

/** Get the key of an association.**/
qual_inline
    void*
key_of_Assoc (Assoc* a)
{
    Associa* map = map_of_Assoc (a);
    return (void*) ((size_t) a - map->assoc_offset + map->key_offset);
}

/** Get the value of an association.**/
qual_inline
    void*
val_of_Assoc (Assoc* a)
{
    Associa* map = map_of_Assoc (a);
    return (void*) ((size_t) a - map->assoc_offset + map->val_offset);
}

/** Set the key for an association (don't use directly).
 * Only use this if the association is not in /really/ in the map.
 * That is, if it exists in the red-black tree.
 * \sa insert_Associa()
 **/
qual_inline
    void
key_fo_Assoc (Assoc* a, const void* key)
{
    Associa* map = map_of_Assoc (a);
    void* p = key_of_Assoc (a);
    memcpy (p, key, map->key_sz);
}

/** Set the value for an association.**/
qual_inline
    void
val_fo_Assoc (Assoc* a, const void* val)
{
    Associa* map = map_of_Assoc (a);
    void* p = val_of_Assoc (a);
    memcpy (p, val, map->val_sz);
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
    Associa* map = map_of_Assoc (lhs);

    return map->swapped (key_of_Assoc (lhs),
                         key_of_Assoc (rhs));
}

/** Request a new element from the map.
 * I recommend not using this directly.
 * \sa insert_Associa()
 * \sa ensure_Associa()
 * \sa ensure1_Associa()
 **/
qual_inline
    Assoc*
req_Associa (Associa* map)
{
    void* node = req_LgTable (&map->nodes);
    Assoc* a = (Assoc*) ((size_t) node + map->assoc_offset);
    a->rbt.bst.joint = 0;
    a->map = map;
    return a;
}

/** Give an element back to the map.
 *
 * This is safe to use directly and will do all necessary node removal
 * from the underlying search data structure.
 *
 * \sa lose_Assoc()
 * \sa remove_Associa()
 **/
qual_inline
    void
giv_Associa (Associa* map, Assoc* assoc)
{
    if (assoc->rbt.bst.joint)
        remove_RBTree (&map->rbt, &assoc->rbt);
    giv_LgTable (&map->nodes, (void*) ((size_t) assoc - map->assoc_offset));
}

/** Lose an association.
 * \sa giv_Associa()
 **/
qual_inline
    void
lose_Assoc (Assoc* assoc)
{
    Associa* map = map_of_Assoc (assoc);
    giv_Associa (map, assoc);
}

/** Associate a key with a value in the map.
 * This can form duplicates.
 * \sa ensure_Associa()
 **/
qual_inline
    Assoc*
insert_Associa (Associa* map, const void* key, const void* val)
{
    Assoc* a = req_Associa (map);
    key_fo_Assoc (a, key);
    val_fo_Assoc (a, val);
    insert_RBTree (&map->rbt, &a->rbt);
    return a;
}

/** Find the association for the given key.
 * \return  NULL when the association could not be found.
 **/
qual_inline
    Assoc*
lookup_Associa (Associa* map, const void* key)
{
    Assoc* a = req_Associa (map);
    BSTNode* bst;

    key_fo_Assoc (a, key);
    bst = find_BSTree (&map->rbt.bst, &a->rbt.bst);
    giv_Associa (map, a);

    if (!bst)  return 0;
    return CastUp( Assoc, rbt, CastUp( RBTNode, bst, bst ) );
}

/** Ensure an entry exists for the given key.
 * This will not cause duplicates.
 * \sa ensure_Associa()
 **/
qual_inline
    Assoc*
ensure1_Associa (Associa* map, const void* key, bool* added)
{
    Assoc* b = req_Associa (map);
    Assoc* a = 0;
    key_fo_Assoc (b, key);

    {
        RBTNode* rbt = ensure_RBTree (&map->rbt, &b->rbt);
        a = CastUp( Assoc, rbt, rbt );
    }
    /* If /b/ was added to the tree,
     * we must replace it with a node which is on the heap.
     */
    *added = (a == b);
    if (!*added)
        giv_Associa (map, b);
    return a;
}

/** Ensure an entry exists for a given key.
 * This will not cause duplicates.
 * \sa ensure1_Associa()
 **/
qual_inline
    Assoc*
ensure_Associa (Associa* map, const void* key)
{
    bool added = false;
    return ensure1_Associa (map, key, &added);
}

/** Remove an association with the given key.
 * \sa giv_Associa()
 **/
qual_inline
    void
remove_Associa (Associa* map, const void* key)
{
    Assoc* a = lookup_Associa (map, key);
    giv_Associa (map, a);
}

static
    void
walkfn_bst_Assoc (BSTNode* bst, void* dats)
{
    Assoc* a = CastUp( Assoc, rbt, CastUp( RBTNode, bst, bst ) );
    void* f = ((void**) dats)[0];
    void* dat = ((void**) dats)[1];

    (* (void (**) (Assoc*, void*)) f) (a, dat);
}

/** Call a function for every association in the map.**/
qual_inline
    void
walk_Associa (Associa* map, void (* f) (Assoc*, void*), void* dat)
{
    void* dats[2];
    dats[0] = &f;
    dats[1] = dat;
    walk_BSTree (&map->rbt.bst, Nil, walkfn_bst_Assoc, dats);
}


#endif

