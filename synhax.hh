
#ifndef SYNHAX_HH_
#define SYNHAX_HH_

#include <iostream>
#include <string>
#include <utility>
#include <vector>

using std::pair;
using std::vector;
using std::string;
using std::ostream;

extern "C" {
#include "cx/def.h"
}

template <class T>
  T&
Grow1(vector<T>& a)
{
  a.resize(a.size() + 1);
  return a.back();
}

template <class T>
  T
Pop1(vector<T>& a)
{
  T x( a.back() );
  a.pop_back();
  return x;
}

template <class T>
  bool
Remove1(vector<T>& a, const T& elem)
{
  typename vector<T>::iterator it;
  for (it = a.begin(); it != a.end(); ++it) {
    if (*it == elem) {
      a.erase(it);
      return true;
    }
  }
  return false;
}

inline
  uint
umod_int (int i, uint n)
{
  if (i >= 0) {
    i = i % n;
  }
  else {
    i = n - ((- i) % n);
    if ((uint) i == n) {
      i = 0;
    }
  }
  return (uint) i;
}

#endif

