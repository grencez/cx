
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
  XFile()
    : xf( 0 )
  {}
  explicit XFile(OFile* xf)
  {
    this->xf = xf;
  }
};
}

#endif
