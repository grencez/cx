
#ifndef URandom_HH_
#define URandom_HH_

#include "synhax.hh"
extern "C" {
#include "urandom.h"
}

namespace Cx {

namespace C {
  using ::URandom;
}

class URandom
{
private:
  C::URandom urandom;
public:
  URandom()
  {
    init_URandom (&urandom);
  }
  URandom(uint pcidx, uint npcs)
  {
    init2_URandom (&urandom, pcidx, npcs);
  }

  uint pick(uint n)
  {
    return uint_URandom (&urandom, n);
  }

  template <typename T>
  void shuffle(T* a, uint n)
  {
    for (; n > 1; --n)
    {
      uint i = n-1;
      uint j = uint_URandom (&urandom, n);
      SwapT( uint, a[i], a[j] );
    }
  }
};
}

#endif

