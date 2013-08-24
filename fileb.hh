
#ifndef FileB_HH_
#define FileB_HH_

#include "ofile.hh"
extern "C" {
#include "fileb.h"
}

namespace Cx {
namespace C {
  using ::OFileB;
}
class OFileB : public Cx::OFile
{
private:
  C::OFileB ofb;

public:
  OFileB()
    : OFile( &ofb.of )
  {
    init_OFileB (&ofb);
  }
  ~OFileB()
  {
    lose_OFileB (&ofb);
  }

  bool open(const String& pathname, const String& filename) {
    return open_FileB (&ofb.fb, pathname.cstr(), filename.cstr());
  }
  bool open(const String& filename) {
    return this->open ("", filename);
  }

private:
  OFileB(const OFileB&);
  OFileB& operator=(const OFileB&);
};
}

#endif
