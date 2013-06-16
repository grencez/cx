
/** George Marsaglia's fast + simple random number generator.**/
#ifndef GMRand_H_
#define GMRand_H_
#include "def.h"

typedef struct GMRand GMRand;
struct GMRand {
  uint32 z;
  uint32 w;
  uint32 k0;
  uint32 k1;
};

qual_inline
  void
init1_GMRand (GMRand* a, uint i)
{
  /* You may replace the two constants 36969 and 18000 by any
   * pair of distinct constants from this list.
   * (or any other 16-bit constants k for which both k*2^16-1
   * and k*2^15-1 are prime)
   */
  static const uint32 ks[] =
  {
    18000, 18030, 18273, 18513, 18879, 19074, 19098, 19164, 19215, 19584,
    19599, 19950, 20088, 20508, 20544, 20664, 20814, 20970, 21153, 21243,
    21423, 21723, 21954, 22125, 22188, 22293, 22860, 22938, 22965, 22974,
    23109, 23124, 23163, 23208, 23508, 23520, 23553, 23658, 23865, 24114,
    24219, 24660, 24699, 24864, 24948, 25023, 25308, 25443, 26004, 26088,
    26154, 26550, 26679, 26838, 27183, 27258, 27753, 27795, 27810, 27834,
    27960, 28320, 28380, 28689, 28710, 28794, 28854, 28959, 28980, 29013,
    29379, 29889, 30135, 30345, 30459, 30714, 30903, 30963, 31059, 31083
  };
  const uint n = ArraySz( ks );

  i = i % n;

  a->z = 362436069;
  a->w = 521288629;
  a->k0 = 36969;
  a->k1 = ks[i];
}

qual_inline
  void
init_GMRand (GMRand* a)
{
  init1_GMRand (a, 0);
}

qual_inline
  uint32
uint32_GMRand (GMRand* a)
{
  /* Need mask for 64-bit machines,
   * otherwise overflowing sums
   * are not guaranteed to be truncated.
   */
  static const uint32 mask = 0xffffffff;
  uint32 z = a->z;
  uint32 w = a->w;
  z = mask&(a->k0*(z&65535)+(z>>16));
  w = mask&(a->k1*(w&65535)+(w>>16));
  a->z = z;
  a->w = w;
  return (mask & ((z<<16) + (w&65535)));
}

qual_inline
  void
step_GMRand (GMRand* a, uint n)
{
  {:for (i ; n)
    uint32_GMRand (a);
  }
}

/** A random real in [0,1).**/
qual_inline
  real
real_GMRand (GMRand* a)
{
  return (real) (uint32_GMRand (a) * 2.328306e-10);
}

qual_inline
  uint
uint_GMRand (GMRand* a, uint n)
{
  return (uint) (n * real_GMRand (a));
}

qual_inline
  bool
bool_GMRand (GMRand* a)
{
  return real_GMRand (a) < .5;
}

#endif

