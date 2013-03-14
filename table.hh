/**
 * \file table.hh
 * Dynamic array.
 **/
#ifndef Table_HH_
#define Table_HH_

#include "synhax.hh"
extern "C" {
#include "lgtable.h"
#include "table.h"
}

namespace Cx {
namespace C {
  using ::LgTable;
  using ::Table;
}

/** Standard vector.*/
template <class T>
class Table
{
private:
  C::Table t;
public:
  Table() {
    t = dflt1_Table (sizeof(T));
  }
  ~Table() {
    for (ujint i = 0; i < t.sz; ++i)
      (*this)[i].~T();
    lose_Table (&t);
  }

  ujint sz() const {
    return t.sz;
  }

  T& operator[](ujint i) {
    return *(T*) elt_Table (&t, i);
  }
  const T& operator[](ujint i) const {
    return *(const T*) elt_Table ((C::Table*)&t, i);
  }

  T& grow1() {
    T* e = (T*) grow1_Table (&t);
    new (e) T();
    return *e;
  }
  T& push(const T& x) {
    return (this->grow1() = x);
  }
  void mpop(const T& x, ujint n) {
    for (ujint i = this->sz() - n; i < this->szz; ++i)
      (*this)[i].~T();
    mpop_Table (&t, n);
  }

  T& top() {
    return *(T*) top_Table (&t);
  }
  const T& top() const {
    return *(T*) top_Table ((C::Table*)&t);
  }
};

template <class T>
  void
sz_of (const Table<T>& t, ujint x)
{
  return t.sz();
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
  ~LgTable() {
    for (ujint i = begidx_LgTable (&t);
         i != Max_ujint;
         i = nextidx_LgTable (&t, i))
    {
      (*this)[i].~T();
    }
    lose_LgTable (&t);
  }

  const T& operator[](ujint i) const {
    return *(const T*) elt_LgTable ((C::LgTable*)&t, i);
  }
  T& operator[](ujint i) {
    return *(T*) elt_LgTable (&t, i);
  }

  T& grow1() {
    T* e = (T*) take_LgTable (&t);
    new (e) T();
    return *e;
  }
  T& push(const T& x) {
    return (this->grow1() = x);
  }

  T& top() {
    return (*this)[t.sz-1];
  }
  const T& top() const {
    return (*this)[t.sz-1];
  }
};
}

#endif

