/**
 * \file lgtable.hh
 * Dynamic array which does not relocate objects in memory.
 **/
#ifndef LgTable_HH_
#define LgTable_HH_

#include "synhax.hh"
extern "C" {
#include "lgtable.h"
}

namespace Cx {
namespace C {
  using ::LgTable;
}

/** Table that does not reallocate existing elements.
 *
 * Due to its nature, the LgTable does not store elements contiguously.
 */
template <class T>
class LgTable
{
private:
  C::LgTable t;
public:
  LgTable() {
    t = dflt1_LgTable (sizeof(T));
  }

  ujint begidx() const {
    return begidx_LgTable (&t);
  }
  ujint nextidx(ujint idx) const {
    return nextidx_LgTable (&t, idx);
  }
  bool endidx_ck(ujint idx) const {
    return endidx_ck_LgTable (&t, idx);
  }

  ~LgTable() {
    for (ujint i = begidx(); i != Max_ujint; i = nextidx(i))
      (*this)[i].~T();
    lose_LgTable (&t);
  }

  ujint sz() const {
    return t.sz;
  }

  const T& operator[](ujint i) const {
    return *(const T*) elt_LgTable ((C::LgTable*)&t, i);
  }
  T& operator[](ujint i) {
    return *(T*) elt_LgTable (&t, i);
  }

  ujint takeidx() {
    ujint idx = takeidx_LgTable (&t);
    new (&(*this)[idx]) T();
    return idx;
  }
  void giveidx(ujint idx) {
    (*this)[idx].~T();
    giveidx_LgTable (&t, idx);
  }

  void clear()
  {
    for (ujint i = begidx(); i != Max_ujint; i = nextidx(i))
    {
      (*this)[i].~T();
      giveidx_LgTable (&t, i);
    }
  }

  T& grow1() {
    T* e = (T*) take_LgTable (&t);
    new (e) T();
    return *e;
  }
  T& push(const T& x) {
    T* e = (T*) take_LgTable (&t);
    new (e) T(x);
    return *e;
  }

  T& top() {
    return (*this)[t.sz-1];
  }
  const T& top() const {
    return (*this)[t.sz-1];
  }

  LgTable<T>& operator=(const LgTable<T>& b) {
    LgTable<T>& a = *this;
    if (&a == &b)  return a;
    a.clear();
    for (ujint i = b.begidx(); i != Max_ujint; i = b.nextidx(i))
      a[a.takeidx()] = b[i];;
    return a;
  }
};

template <class T>
  ujint
sz_of (const LgTable<T>& t)
{
  return t.sz();
}
}

#endif

