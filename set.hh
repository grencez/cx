
#ifndef Set_HH_
#define Set_HH_

#include "synhax.hh"
#include "table.hh"
#include <set>

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
    std::set<T>(&a[0], &a[sz()])
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

  ujint sz() const { return this->size(); }
};
}
using Cx::Set;

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

