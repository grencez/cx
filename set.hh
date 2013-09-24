
#ifndef Set_HH_
#define Set_HH_

#include "synhax.hh"
#include "table.hh"
#include <set>
#include <algorithm>

namespace Cx {
template <class T> class Set;
template <class T> class FlatSet;

template <class T>
class Set : public std::set<T>
{
public:
  Set() {}
  explicit Set(const T& e) { *this |= e; }
  explicit Set(const vector<T>& a) :
    std::set<T>(a.begin(), a.end())
  {}
  explicit Set(const Table<T>& a) :
    std::set<T>(a.begin(), a.end())
  {}
  explicit Set(const FlatSet<T>& a);

  bool elem_ck(const T& e) const
  {
    return (this->find(e) != this->end());
  }

  const T& elem() const
  {
    return *this->begin();
  }

  Set<T>& operator|=(const T& e)
  { this->insert(e); return *this; }

  Set<T>& operator|=(const Set<T>& b)
  {
    this->insert(b.begin(), b.end());
    return *this;
  }

  Set<T> operator|(const Set<T>& b) const
  {
    Set c( *this );
    return c |= b;
  }

  Set<T>& operator-=(const T& e)
  {
    this->erase(e);
    return *this;
  }

  Set<T>& operator-=(const Set<T>& b)
  {
    Set<T>& a = *this;
    for (typename Set<T>::const_iterator itb = b.begin();
         itb != b.end();
         ++ itb)
    {
      a -= *itb;
    }
    return a;
  }

  Set<T>& operator-(const T& e) const
  {
    Set<T> c( *this );
    c -= e;
    return c;
  }
  Set<T> operator-(const Set<T>& b) const
  {
    Set<T> c( *this );
    c -= b;
    return c;
  }

  Set<T>& operator&=(const Set<T>& b)
  {
    return (*this -= (*this - b));
  }

  Set<T> operator&(const Set<T>& b) const
  {
    if (this->size() < b.size()) {
      return (*this - (*this - b));
    }
    return (b - (b - *this));
  }

  void fill(vector<T>& a) const
  {
    a.assign(this->begin(), this->end());
  }

  bool subseteq_ck(const Set<T>& b) const
  {
    const Set<T>& a = *this;
    if (a.sz() > b.sz())
      return false;
    for (typename Set<T>::const_iterator ita = a.begin();
         ita != a.end();
         ++ ita)
    {
      if (!b.elem_ck(*ita))
        return false;
    }
    return true;
  }

  ujint sz() const { return this->size(); }
};

template <class T>
class FlatSet : public Table<T>
{
public:
  FlatSet() {}
  FlatSet(const FlatSet<T>& a)
    : Table<T>()
  {
    this->affy(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)
      this->push(a[i]);
  }
  ~FlatSet() {}
  void operator=(const FlatSet<T>& a) {
    this->affysz(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)
      (*this)[i] = a[i];
  }

  explicit FlatSet(const Table<T>& a) {
    this->affy(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)
      this->push(a[i]);
    std::sort (this->begin(), this->end());
  }
  explicit FlatSet(const vector<T>& a) {
    this->affy(a.size());
    for (ujint i = 0; i < a.size(); ++i)
      this->push(a[i]);
    std::sort (this->begin(), this->end());
  }
  explicit FlatSet(const Set<T>& a) {
    this->affy(a.sz());
    typename Set<T>::const_iterator it = a.begin();
    while (this->sz() < a.sz()) {
      this->push(*it);
      ++it;
    }
  }

  bool elem_ck(const T& e) const {
    return std::binary_search (this->begin(), this->end(), e);
  }

  bool subseteq_ck(const FlatSet<T>& b) const {
    const FlatSet<T>& a = *this;
    if (a.sz() > b.sz())  return false;
    ujint off = 0;
    for (ujint i = 0; i < a.sz();)
    {
      if (a[i] < b[i+off])
        return false;

      if (a[i] == b[i+off]) {
        ++ i;
      }
      else {
        if (a.sz()+off == b.sz())
          return false;
        ++ off;
      }
    }
    return true;
  }

  bool subseteq_fuzz_ck(Cx::Table<T>* diff, const FlatSet<T>& b, ujint nmisses) const {
    const FlatSet<T>& a = *this;
    if (diff)
      diff->flush();
    if (a.sz() > b.sz() + nmisses)
      return false;
    ujint i = 0;
    ujint j = 0;
    while (i < a.sz() && j < b.sz())
    {
      Sign si = (a[i] == b[j] ? 0 : (a[i] < b[j] ? -1 : 1));
      if (si == 0) {
        i += 1;
        j += 1;
      }
      else if (si > 0) {
        j += 1;
        if (a.sz() - i > b.sz() - j + nmisses)
          return false;
      }
      else {
        if (nmisses == 0)
          return false;
        nmisses -= 1;
        if (diff)
          diff->push(a[i]);
        i += 1;
      }
    }
    while (i < a.sz()) {
      if (nmisses == 0)
        return false;
      nmisses -= 1;
      if (diff)
        diff->push(a[i]);
      i += 1;
    }
    return true;
  }

  bool overlap_ck(const FlatSet<T>& b) const {
    const FlatSet<T>& a = *this;
    if (a.sz() > b.sz())  return b.overlap_ck(a);
    ujint i = 0;
    ujint j = 0;
    while (i < a.sz() && j < b.sz()) {
      if (a[i] == b[j])  return true;
      if (a[i] < b[j])  ++i;
      if (b[j] < a[i])  ++j;
    }
    return false;
  }

  bool overlap_ck(const Set<T>& b) const {
    const FlatSet<T>& a = *this;
    for (ujint i = 0; i < a.sz(); ++i) {
      if (b.elem_ck(a[i]))  return true;
    }
    return false;
  }
};
template <class T>
Set<T>::Set(const FlatSet<T>& a) :
  std::set<T>(a.begin(), a.end())
{}
}
using Cx::Set;
using Cx::FlatSet;

template <class T>
  void
Remove(vector<T>& a, const Cx::Set<T>& set)
{
  uint n = 0;
  for (uint i = 0; i < a.size(); ++i) {
    if (set.elem_ck(a[i])) {
      ++ n;
    }
    else if (n > 0) {
      a[i-n] = a[i];
    }
  }
  a.resize(a.size() - n);
}

#endif

