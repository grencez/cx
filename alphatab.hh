/**
 * \file alphatab.hh
 * Dynamic array.
 **/
#ifndef AlphaTab_HH_
#define AlphaTab_HH_

#include "synhax.hh"
extern "C" {
#include "alphatab.h"
}

namespace Cx {
namespace C {
  using ::AlphaTab;
}

class AlphaTab
{
private:
  C::AlphaTab t;
public:
  AlphaTab() {
    t = dflt_AlphaTab ();
  }
  AlphaTab(const char* s) {
    t = cons1_AlphaTab (s);
  }

  AlphaTab(const AlphaTab& b) {
    t = dflt_AlphaTab ();
    copy_AlphaTab (&t, &b.t);
  }
  const AlphaTab& operator=(const AlphaTab& b) {
    copy_AlphaTab (&t, &b.t);
    return *this;
  }
  ~AlphaTab() {
    lose_AlphaTab (&t);
  }

  const AlphaTab& operator=(uint x) {
    copy_cstr_AlphaTab (&t, "");
    cat_uint_AlphaTab (&t, x);
    return *this;
  }
  const AlphaTab& operator=(int x) {
    copy_cstr_AlphaTab (&t, "");
    cat_int_AlphaTab (&t, x);
    return *this;
  }

  const AlphaTab& operator+=(const AlphaTab& b) {
    cat_AlphaTab (&t, &b.t);
    return *this;
  }
  AlphaTab operator+(const AlphaTab& b) const {
    AlphaTab a( *this );
    a += b;
    return a;
  }

  const AlphaTab& operator+=(uint x) {
    cat_uint_AlphaTab (&t, x);
    return *this;
  }
  AlphaTab operator+(uint x) const {
    AlphaTab a( *this );
    a += x;
    return a;
  }

  const AlphaTab& operator+=(int x) {
    cat_int_AlphaTab (&t, x);
    return *this;
  }
  AlphaTab operator+(int x) const {
    AlphaTab a( *this );
    a += x;
    return a;
  }

  bool operator==(const AlphaTab& b) const {
    return (May == swapped_AlphaTab (&t, &b.t));
  }
  bool operator<(const AlphaTab& b) const {
    return (Nil == swapped_AlphaTab (&t, &b.t));
  }

  const char* cstr() const {
    return t.s;
  }
};

inline
std::ostream& operator<<(ostream& out, const AlphaTab& a) {
  return out << a.cstr();
}

typedef AlphaTab String;
}

#endif

