/**
 * \file table.hh
 * Dynamic array.
 **/
#ifndef Table_HH_
#define Table_HH_

#include "synhax.hh"
extern "C" {
#include "table.h"
}

namespace Cx {
namespace C {
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
  Table(const Table<T>& a) {
    t = dflt1_Table (sizeof(T));
    for (uint i = 0; i < a.sz(); ++i) {
      this->push(a[i]);
    }
  }
  const Table<T>& operator=(const Table<T>& a) {
    this->resize(a.sz());
    for (uint i = 0; i < a.sz(); ++i) {
      (*this)[i] = a[i];
    }
    return *this;
  }
  ~Table() {
    for (ujint i = 0; i < t.sz; ++i)
      (*this)[i].~T();
    lose_Table (&t);
  }

  explicit Table(ujint capac) {
    t = dflt1_Table (sizeof(T));
    this->grow(capac);
  }
  Table(ujint capac, const T& e) {
    t = dflt1_Table (sizeof(T));
    this->grow(capac, e);
  }

  ujint sz() const {
    return t.sz;
  }
  ujint size() const { return this->sz(); }

  void affy(ujint capac) {
    ujint old_sz = this->sz();
    for (ujint i = capac; i < old_sz; ++i)
      (*this)[i].~T();
    affy_Table (&t, capac);
  }

  void affysz(ujint capac, const T& e = T()) {
    ujint old_sz = this->sz();
    for (ujint i = capac; i < old_sz; ++i)
      (*this)[i].~T();
    affysz_Table (&t, capac);
    for (ujint i = old_sz; i < t.sz; ++i) {
      new (&(*this)[i]) T(e);
    }
  }

  void grow(ujint capac, const T& e = T()) {
    ujint old_sz = t.sz;
    grow_Table (&t, capac);
    for (ujint i = old_sz; i < t.sz; ++i) {
      new (&(*this)[i]) T(e);
    }
  }

  void resize(ujint capac) {
    ujint old_sz = this->sz();
    for (ujint i = capac; i < old_sz; ++i)
      (*this)[i].~T();
    size_Table (&t, capac);
    for (ujint i = old_sz; i < capac; ++i)
      new (&(*this)[i]) T();
  }
  void clear() {
    this->resize(0);
  }

  T& operator[](ujint i) {
    return *(T*) elt_Table (&t, i);
  }
  const T& operator[](ujint i) const {
    return *(const T*) elt_Table ((C::Table*)&t, i);
  }

  bool elem_ck(const T& e) const
  {
    for (ujint i = 0; i < t.sz; ++i) {
      if (e == (*this)[i])
        return true;
    }
    return false;
  }

  T& grow1() {
    T* e = (T*) grow1_Table (&t);
    new (e) T();
    return *e;
  }
  T& push(const T& x) {
    T* e = (T*) grow1_Table (&t);
    new (e) T(x);
    return *e;
  }
  void mpop(ujint n) {
    for (ujint i = this->sz() - n; i < this->sz(); ++i)
      (*this)[i].~T();
    mpop_Table (&t, n);
  }

  T& top() {
    return *(T*) top_Table (&t);
  }
  const T& top() const {
    return *(T*) top_Table ((C::Table*)&t);
  }

  bool operator==(const Table<T>& b) const {
    const Table<T>& a = *this;
    const ujint n = a.sz();
    if (n != b.sz())  return false;
    for (ujint i = 0; i < n; ++i) {
      if (a[i] != b[i])  return false;
    }
    return true;
  }
  bool operator!=(const Table<T>& b) const {
    return !(*this == b);
  }

  Sign cmp(const Table<T>& b) const
  {
    const Table<T>& a = *this;
    const ujint n = (a.sz() <= b.sz()) ? a.sz() : b.sz();
    for (ujint i = 0; i < n; ++i) {
      if (a[i] < b[i])  return -1;
      if (b[i] < a[i])  return  1;
    }
    if (a.sz() < b.sz())  return -1;
    if (b.sz() < a.sz())  return  1;
    return 0;
  }

  bool operator<=(const Table<T>& b) const {
    return (this->cmp(b) <= 0);
  }
  bool operator<(const Table<T>& b) const {
    return (this->cmp(b) < 0);
  }
  bool operator>(const Table<T>& b) const {
    return (this->cmp(b) > 0);
  }
  bool operator>=(const Table<T>& b) const {
    return (this->cmp(b) >= 0);
  }

  T* begin() {
    return (T*) elt_Table ((C::Table*)&t, 0);
  }
  T* end() {
    return (T*) elt_Table ((C::Table*)&t, t.sz);
  }
  const T* begin() const {
    return (T*) elt_Table ((C::Table*)&t, 0);
  }
  const T* end() const {
    return (T*) elt_Table ((C::Table*)&t, t.sz);
  }
  void reverse() {
    ujint n = this->sz() / 2;
    for (ujint i = 0; i < n; ++i)
      SwapT( T, (*this)[i], (*this)[this->sz()-1-i] );
  }

  /** If this Table represents a mapping from state indices,
   * this method grows the state space to allow a new variable of
   * size /domsz/.
   *
   * \sa Cx::state_of_index()
   * \sa Cx::index_of_index()
   */
  void add_domain(uint domsz) {
    Table<T>& state_map = *this;
    const ujint n = state_map.sz();
    state_map.grow(n * (domsz-1));
    for (ujint i = n; i > 0; --i) {
      for (uint j = 0; j < domsz; ++j) {
        state_map[(i-1) * domsz + j] = state_map[i-1];
      }
    }
  }

};

template <class T>
  ujint
sz_of (const Table<T>& t)
{
  return t.sz();
}

inline
  void
state_of_index (uint* state, ujint idx, const Table<uint>& doms)
{
  ::state_of_index (state, idx, &doms[0], doms.sz());
}

inline
  ujint
index_of_state (const uint* state, const Table<uint>& doms)
{
  return ::index_of_state (state, &doms[0], doms.sz());
}
}

#endif

