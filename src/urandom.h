
#ifndef URandom_H_
#define URandom_H_
#include "gmrand.h"

typedef GMRand URandom;

qual_inline
  void
init2_URandom (URandom* urandom, uint pcidx, uint npcs)
{
  init2_GMRand (urandom, pcidx, npcs);
}

qual_inline
  void
init_URandom (URandom* urandom)
{
  init_GMRand (urandom);
}

qual_inline
  uint32
uint32_URandom (URandom* urandom)
{
  return uint32_GMRand (urandom);
}

/** Generate a real in [0,1).**/
qual_inline
  real
real_URandom (URandom* urandom)
{
  return real_GMRand (urandom);
}

/** Generate a uint in {0,...,n-1}.**/
qual_inline
  uint
uint_URandom (URandom* urandom, uint n)
{
  uint x = uint_GMRand (urandom, n);
  return x;
  //return uint_GMRand (urandom, n);
}

qual_inline
  Bit
bit_URandom (URandom* urandom)
{
  return bit_GMRand (urandom);
}

qual_inline
  bool
bool_URandom (URandom* urandom)
{
  return bool_GMRand (urandom);
}

qual_inline
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
uint_SysURandom(uint n);

#endif


