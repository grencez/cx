
#ifndef XFile_HH_
#define XFile_HH_

#include "synhax.hh"
extern "C" {
#include "xfile.h"
}

namespace Cx {
namespace C {
  using ::XFile;
}

class XFile
{
private:
  C::XFile* xf;

public:
  bool allgood;

  XFile()
    : xf( 0 )
    , allgood( true )
  {}
  explicit XFile(C::XFile* xf)
    : xf( 0 )
    , allgood( true )
  {
    this->xf = xf;
  }

  bool good() const
  { return allgood; }

  XFile& operator>>(int& x)
  {
    bool good =
      xget_int_XFile (xf, &x);
    allgood = allgood && good;
    return *this;
  }
  XFile& operator>>(uint& x)
  {
    bool good =
      xget_uint_XFile (xf, &x);
    allgood = allgood && good;
    return *this;
  }
  XFile& operator>>(ujint& x)
  {
    bool good =
      xget_ujint_XFile (xf, &x);
    allgood = allgood && good;
    return *this;
  }
  XFile& operator>>(char& c)
  {
    bool good =
      xget_char_XFile (xf, &c);
    allgood = allgood && good;
    return *this;
  }
};
}

#endif
