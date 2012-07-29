
#ifndef Table_H_
#define Table_H_
#include "def.h"

#include <stdlib.h>
#include <string.h>

typedef byte TableLgSz;
typedef unsigned short TableElSz;

#define TableT( S )  TableT_##S
#define TableElT( S )  TableElT_##S

typedef struct Table Table;
struct Table
{
    void* s;
    ujint sz;
    TableElSz elsz;
    TableLgSz alloc_lgsz;
};

#define DeclTableT( S, T ) \
    typedef struct TableT_##S TableT_##S; \
    typedef T TableElT_##S; \
    struct TableT_##S { \
        TableElT_##S* s; \
        ujint sz; \
        TableLgSz alloc_lgsz; \
    }

#define DeclTableT_MemLoc
DeclTableT( MemLoc, void* );
#define DeclTableT_byte
DeclTableT( byte, byte );
#define DeclTableT_char
DeclTableT( char, char );
#define DeclTableT_uint
DeclTableT( uint, uint );
#define DeclTableT_ujint
DeclTableT( ujint, ujint );

qual_inline
    Table
dflt4_Table (void* s, ujint sz, TableElSz elsz, TableLgSz alloc_lgsz)
{
    Table t;
    t.s = s;
    t.sz = sz;
    t.elsz = elsz;
    t.alloc_lgsz = alloc_lgsz;
    return t;
}
qual_inline
    Table
dflt1_Table (TableElSz elsz)
{
    return dflt4_Table (0, 0, elsz, 0);
}
#define DeclTable( S, table ) \
    TableT_##S table = { 0, 0, 0 }


#define MakeCastTable( t ) \
    dflt4_Table ((t).s, (t).sz, sizeof(*(t).s), (t).alloc_lgsz)

#define XferCastTable( t, name )  do \
{ \
    memcpy (&(t).s, &(name).s, sizeof(void*)); \
    BSfx( t ,=, name ,.sz ); \
    BSfx( t ,=, name ,.alloc_lgsz ); \
} while (0)


qual_inline
    void
init1_Table (Table* t, TableElSz elsz)
{
    *t = dflt1_Table (elsz);
}
#define InitTable( t )  do \
{ \
    (t).s = 0; \
    (t).sz = 0; \
    (t).alloc_lgsz = 0; \
} while (0)

qual_inline
    void
lose_Table (Table* t)
{
    if (t->alloc_lgsz > 0)
        free (t->s);
}
#define LoseTable( t )  do \
{ \
    Table LoseTable_t = MakeCastTable( t ); \
    lose_Table (&LoseTable_t); \
} while (0)

#define AllocszTable( t ) \
    ((t).alloc_lgsz == 0 ? 0 : (ujint)1 << ((t).alloc_lgsz - 1))
qual_inline
    ujint
allocsz_Table (const Table* t)
{
    return AllocszTable( *t );
}

qual_inline
    void*
elt_Table (Table* t, ujint idx)
{
    return EltZ( t->s, idx, t->elsz );
}
#define DeclEltTable( S, x, t, idx ) \
    TableElT_##S* const x = Elt( (t).s, idx )

qual_inline
    ujint
idxelt_Table (const Table* t, const void* el)
{
    return (ujint) IdxEltZ( t->s, el, t->elsz );
}
#define IdxEltTable( t, el ) \
    (ujint) IdxEltZ( (t).s, el, sizeof(*(t).s) )


qual_inline
    void
grow_Table (Table* t, ujint capac)
{
    t->sz += capac;
    if ((t->sz << 1) > ((ujint)1 << t->alloc_lgsz))
    {
        if (t->alloc_lgsz == 0)
        {
            t->s = 0;
            t->alloc_lgsz = 1;
        }
        while (t->sz > ((ujint)1 << t->alloc_lgsz))
            t->alloc_lgsz += 1;

        t->alloc_lgsz += 1;
        t->s = realloc (t->s, allocsz_Table (t) * t->elsz);
    }
}
#define GrowTable( t, capac )  do \
{ \
    Table GrowTable_t = MakeCastTable( t ); \
    grow_Table (&GrowTable_t, capac); \
    XferCastTable( t, GrowTable_t ); \
} while (0)


qual_inline
    void
mpop_Table (Table* t, ujint capac)
{
    t->sz -= capac;
    if ((t->alloc_lgsz >= 3) && ((t->sz >> (t->alloc_lgsz - 3)) == 0))
    {
        while ((t->alloc_lgsz >= 4) && ((t->sz >> (t->alloc_lgsz - 4)) == 0))
            t->alloc_lgsz -= 1;
        t->alloc_lgsz -= 1;
        t->s = realloc (t->s, allocsz_Table (t) * t->elsz);
    }
}
#define MPopTable( t, capac )  do \
{ \
    Table MPopTable_t = MakeCastTable( t ); \
    mpop_Table (&MPopTable_t, capac); \
    XferCastTable( t, MPopTable_t ); \
} while (0)


qual_inline
    void*
top_Table (Table* t)
{
    return elt_Table (t, t->sz - 1);
}
#define TopTable( t )  Elt((t).s, (t).sz-1)

qual_inline
    bool
elt_in_Table (Table* t, void* el)
{
    return EltInZ( t->s, el, t->sz, t->elsz );
}
#define EltInTable( t, el ) \
    EltInZ( (t).s, (el), (t).sz, sizeof(*(t).s) )

qual_inline
    void*
grow1_Table (Table* t)
{
    grow_Table (t, 1);
    return top_Table (t);
}

    /** Don't use this... It's a hack for the Grow1Table() macro.**/
qual_inline
    void
synhax_grow1_Table (void* ps, void* s, ujint* sz,
                    TableElSz elsz, TableLgSz* alloc_lgsz)
{
    Table t = dflt4_Table (s, *sz, elsz, *alloc_lgsz);
    grow1_Table (&t);
    memcpy (ps, &t.s, sizeof(void*));
    *alloc_lgsz = t.alloc_lgsz;
    *sz = t.sz;
}
#define Grow1Table( t ) \
    (synhax_grow1_Table (&(t).s, (t).s, &(t).sz, \
                         sizeof(*(t).s), &(t).alloc_lgsz), \
     TopTable( t ))
#define DeclGrow1Table( S, x, t ) \
    TableElT_##S* const x = Grow1Table( t )
#define PushTable( table, x ) \
    *(Grow1Table( table )) = (x)


qual_inline
    void
size_Table (Table* t, ujint capac)
{
    if (t->sz <= capac)  grow_Table (t, capac - t->sz);
    else                 mpop_Table (t, t->sz - capac);
}
#define SizeTable( t, capac )  do \
{ \
    Table SizeTable_t = MakeCastTable( t ); \
    size_Table (&SizeTable_t, capac); \
    XferCastTable( t, SizeTable_t ); \
} while (0)

    /** Never downsize.**/
qual_inline
    void
ensize_Table (Table* t, ujint capac)
{
    if (t->sz < capac)
        grow_Table (t, capac - t->sz);
    else
        t->sz = capac;
}
#define EnsizeTable( t, capac )  do \
{ \
    Table EnsizeTable_t = MakeCastTable( t ); \
    ensize_Table (&EnsizeTable_t, capac); \
    XferCastTable( t, EnsizeTable_t ); \
} while (0)


qual_inline
    void
pack_Table (Table* t)
{
    if ((t->sz << 1) < ((ujint) 1 << t->alloc_lgsz))
    {
        if (t->sz == 0)
        {
            free (t->s);
            t->s = 0;
            t->alloc_lgsz = 0;
        }
        else
        {
            t->s = realloc (t->s, t->sz * t->elsz);
            while ((t->sz << 1) < ((ujint) 1 << t->alloc_lgsz))
                t->alloc_lgsz -= 1;
        }
    }
}
#define PackTable( t )  do \
{ \
    Table PackTable_t = MakeCastTable( t ); \
    pack_Table (&PackTable_t); \
    XferCastTable( t, PackTable_t ); \
} while (0)

qual_inline
    void
affy_Table (Table* t, ujint capac)
{
    t->alloc_lgsz = sizeof(ujint) * NBits_byte - 1;
    t->s = (byte*) realloc (t->s, t->elsz * capac);
}
#define AffyTable( t, capac )  do \
{ \
    Table AffyTable_t = MakeCastTable( t ); \
    affy_Table (&AffyTable_t, capac); \
    XferCastTable( t, AffyTable_t ); \
} while (0)

qual_inline
    void
copy_Table (Table* a, const Table* b)
{
    if (a->elsz != b->elsz)
    {
        a->sz = a->sz * a->elsz / b->elsz;
        a->elsz = b->elsz;
    }

    ensize_Table (a, b->sz);
    memcpy (a->s, b->s, a->sz * a->elsz);
}
#define CopyTable( a, b )  do \
{ \
    Table CopyTable_a = MakeCastTable( a ); \
    Table CopyTable_b = MakeCastTable( b ); \
    copy_Table (&CopyTable_a, &CopyTable_b); \
    XferCastTable( a, CopyTable_a ); \
} while (0)


#endif

