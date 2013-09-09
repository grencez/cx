
#ifndef Set_HH_
#define Set_HH_

#include "synhax.hh"
#include "table.hh"
#include <set>
#include <algorithm>

namespace Cx {
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

  Set<T>& operator-=(const Set<T>& b)
  {
    Set<T>& a = *this;
#if 1
    for (typename Set<T>::const_iterator itb = b.begin();
         itb != b.end();
         ++ itb)
    {
      a.erase(*itb);
    }
#else
    typename Set<T>::const_iterator itb = b.begin();
    typename Set<T>::iterator ita = a.lower_bound(*itb);
    typename Set<T>::key_compare f = a.key_comp();

    while (ita != a.end() && itb != b.end()) {
      if (f(*ita,*itb)) {
        ita = a.lower_bound(*itb);
      }
      else if (f(*itb,*ita)) {
        itb = b.lower_bound(*ita);
      }
      else {
        typename Set<T>::iterator tmp = ita;
        ++ita;
        ++itb;
        a.erase(tmp);
      }
    }
#endif
    return a;
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
class FlatSet
{
private:
  Table<T> t;
public:
  FlatSet(const FlatSet<T>& a) {
    t.affy(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)  
      t.push(a[i]);
  }
  ~FlatSet() {}
  void operator=(const FlatSet<T>& a) {
    t.affysz(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)  
      t[i] = a[i];
  }

  explicit FlatSet(const Table<T>& a) {
    t.affy(a.sz());
    for (ujint i = 0; i < a.sz(); ++i)  
      t.push(a[i]);
    std::sort (t.begin(), t.end());
  }
  explicit FlatSet(const Set<T>& a) {
    t.affy(a.sz());
    typename Set<T>::const_iterator it = a.begin();
    while (t.sz() < a.sz()) {
      t.push(*it);
      ++it;
    }
  }

  bool operator==(const FlatSet& b) const { return (t == b.t); }
  bool operator!=(const FlatSet& b) const { return (t != b.t); }
  bool operator< (const FlatSet& b) const { return (t <  b.t); }
  bool operator<=(const FlatSet& b) const { return (t <= b.t); }
  bool operator> (const FlatSet& b) const { return (t >  b.t); }
  bool operator>=(const FlatSet& b) const { return (t >= b.t); }
  const T& operator[](ujint i) const { return t[i]; }
  T& operator[](ujint i) { return t[i]; }
  ujint sz() const { return t.sz(); }

  bool elem_ck(const T& e) const {
    return std::binary_search (t.begin(), t.end(), e);
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
};
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

