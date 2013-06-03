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
    t = dflt1_AlphaTab (s);
  }

  AlphaTab(const AlphaTab& b) {
    t = dflt_AlphaTab ();
    copy_AlphaTab (&t, &b.t);
  }

  AlphaTab(const AlphaTab& b) {
    t = dflt_AlphaTab ();
    copy_AlphaTab (&t, &b.t);
  }

  ~AlphaTab() {
    lose_AlphaTab (&t);
  }
};

typedef AlphaTab String;
}

#endif

