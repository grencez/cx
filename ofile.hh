
#ifndef OFile_HH_
#define OFile_HH_

#include "synhax.hh"
extern "C" {
#include "ofile.h"
}
#include "alphatab.hh"

namespace Cx {
namespace C {
  using ::OFile;
}

class OFile
{
private:
  C::OFile* of;

public:
  struct EndL {
    bool empty;
  };

  static EndL endl() { return EndL(); }

  OFile()
    : of( 0 )
  {}
  explicit OFile(C::OFile* of)
  {
    this->of = of;
  }

  static OFile& null() {
    static OFile of( null_OFile () );
    return of;
  }

  OFile& operator<<(int x)
  {
    oput_int_OFile (of, x);
    return *this;
  }
  OFile& operator<<(uint x)
  {
    oput_uint_OFile (of, x);
    return *this;
  }
  OFile& operator<<(ujint x)
  {
    oput_ujint_OFile (of, x);
    return *this;
  }
  OFile& operator<<(char c)
  {
    oput_char_OFile (of, c);
    return *this;
  }
  OFile& operator<<(const char* s)
  {
    oput_cstr_OFile (of, s);
    return *this;
  }
  OFile& operator<<(const Cx::AlphaTab& s)
  {
    oput_AlphaTab (of, &s.t);
    return *this;
  }
  void flush() {
    flush_OFile (of);
  }
  OFile& operator<<(const EndL& e)
  {
    (void) e;
    *this << '\n';
    this->flush();
    return *this;
  }
};
}

#endif

