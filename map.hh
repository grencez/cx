
#ifndef Map_HH_
#define Map_HH_

#include "synhax.hh"
#include <map>

namespace Cx {

template <class K, class V>
class Map : public std::map<K,V>
{
public:
  Map() {}

  const V* lookup(const K& key) const
  {
    typename Map<K,V>::const_iterator it = this->find(key);
    if (it == this->end())  return NULL;
    return &it->second;
  }

  V* lookup(const K& key)
  {
    typename Map<K,V>::iterator it = this->find(key);
    if (it == this->end())  return NULL;
    return &it->second;
  }

  ujint sz() const { return this->size(); }
};

template <class K, class V>
  ujint
sz_of(const Map<K,V>& m)
{
  return m.sz();
}

}
using Cx::Map;

#endif

