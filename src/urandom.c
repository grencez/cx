
#include "bittable.h"
#include "urandom.h"

/** If our uint32 type is more than 32 bits, then this mask
 * should be applied to ensure overflowing sums are truncated.
 * But uint32_t should be 32 bits, so it isn't used.
 **/
#define MASK32(x)  (UINT32_MAX & (x))
#define MASK16(x)  (UINT16_MAX & (x))

#include "thirdparty/rng-GeorgeMarsaglia.c"

#include "thirdparty/hash-ThomasWang.c"
#define uint32_hash uint32_hash_ThomasWang

#include "thirdparty/rng-ChrisLomont.c"



  void
init2_seeded_URandom (URandom* urandom, uint pcidx, uint npcs)
{
  (void) npcs;
  init_WELL512 (urandom);
  /* init_GMRand (urandom); */
  urandom->salt = uint32_hash(pcidx);
}

  void
init3_URandom (URandom* urandom, uint pcidx, uint npcs, uint seed)
{
  (void) npcs;
  for (uint i = 0; i < ArraySz(urandom->state); ++i) {
    uint32 x = seed + i + ArraySz(urandom->state) * pcidx;
    urandom->state[i] = uint32_hash(x);
  }

  init2_seeded_URandom (urandom, pcidx, npcs);
}

  void
init2_URandom (URandom* urandom, uint pcidx, uint npcs)
{
  (void) npcs;
  for (uint i = 0; i < ArraySz(urandom->state); ++i) {
    uint32 x = i + ArraySz(urandom->state) * pcidx;
    urandom->state[i] = uint32_hash(x);
  }

  Randomize( urandom->state );
  init2_seeded_URandom (urandom, pcidx, npcs);
}

  void
init1_URandom (URandom* urandom, uint seed)
{
  init3_URandom (urandom, 0, 1, seed);
}

  uint32
uint32_URandom (URandom* urandom)
{
  uint32 x = uint32_WELL512 (urandom);
  /* uint32 x = uint32_GMRand (urandom); */
  return (x ^ urandom->salt);
}

  Bit
bit_URandom (URandom* urandom)
{
  return (Bit) (uint32_URandom (urandom) >> 31);
}

/** Generate a uint in {0,...,n-1}.**/
  uint
uint_URandom (URandom* urandom, uint n)
{
#if 1
  uint x = uint32_URandom (urandom);
  return (uint) (n * (x * 2.328306e-10));
#else
  /* May screw with the randomness.*/
  const uint32 q = (UINT32_MAX / n);
  const uint32 m = UINT32_MAX - (UINT32_MAX % n);
  uint32 x;
  do {
    x = uint32_URandom (a);
  } while (x >= m);
  return x / q;
#endif
}


/** Shuffle integers in an array.**/
  void
shuffle_uints_URandom (URandom* urandom, uint* a, uint n)
{
  for (; n > 1; --n)
  {
    uint i = n-1;
    uint j = uint_URandom (urandom, n);
    SwapT( uint, a[i], a[j] );
  }
}

  uint
randommod_sysCx(uint n)
{
  const uint max = n-1;
  FixDeclBitTable( bt, BYTE_BIT, 0 );
  const uint nbits = lg_luint (max) + 1;
  const uint nbytes = ceil_quot(nbits, BYTE_BIT);
  uint x;

  do {
    randomize_sysCx (bt.s, nbytes);

    /* We can assume each bit is uniformly random,
     * so truncate as much as possible without excluding {max}.
     */
    x = get_uint_BitTable (bt, 0, nbits);

    /* If {x} is outside of our range, then try again.
     * This has less than a 50% chance of happening, so this loop will terminate eventually.
     */
  } while (x > max);

  return x;
}

