
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
  C::OFile* ofile;

public:
  struct EndL {
    bool empty;
  };

  static EndL endl() { return EndL(); }

  OFile()
    : ofile( 0 )
  {}
  explicit OFile(C::OFile* ofile)
  {
    this->ofile = ofile;
  }

  static OFile& null() {
    static OFile ofile( null_OFile () );
    return ofile;
  }

  OFile& operator<<(int x)
  {
    oput_int_OFile (ofile, x);
    return *this;
  }
  OFile& operator<<(uint x)
  {
    oput_uint_OFile (ofile, x);
    return *this;
  }
  OFile& operator<<(ujint x)
  {
    oput_ujint_OFile (ofile, x);
    return *this;
  }
  OFile& operator<<(real x)
  {
    oput_real_OFile(ofile, x);
    return *this;
  }
  OFile& operator<<(char c)
  {
    oput_char_OFile (ofile, c);
    return *this;
  }
  OFile& operator<<(const char* s)
  {
    oput_cstr_OFile (ofile, s);
    return *this;
  }
  OFile& operator<<(const Cx::AlphaTab& s)
  {
    oput_AlphaTab (ofile, &s.t);
    return *this;
  }
  void flush() {
    flush_OFile (ofile);
  }
  OFile& operator<<(const EndL& e)
  {
    (void) e;
    *this << '\n';
    this->flush();
    return *this;
  }

  void write(const char* s, ujint n)
  {
    oputn_char_OFile (ofile, s, n);
  }
};
}

#endif

