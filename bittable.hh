/**
 * \file bittable.hh
 * C++ bit array.
 **/
#ifndef BitTable_HH_
#define BitTable_HH_

#include "synhax.hh"
extern "C" {
#include "bittable.h"
}

namespace Cx {
namespace C {
  using ::BitTable;
}

class BitTable
{
private:
  C::BitTable bt;
public:

  class BitTableElement {
    C::BitTable bt;
    ujint idx;
  public:
    BitTableElement(C::BitTable _bt, ujint _idx)
      : bt(_bt)
      , idx(_idx)
    {}

    BitTableElement& operator=(Bit b)
    {
      setb_BitTable (bt, idx, b);
      return *this;
    }

    operator bool() const
    { return ck_BitTable (bt, idx); }
  };

  BitTable() {
    bt = dflt_BitTable ();
  }
  BitTable(ujint n, Bit b) {
    bt = cons2_BitTable (n, b);
  }
  BitTable(const BitTable& a) {
    bt = cons1_BitTable (a.sz());
    op_BitTable (bt, BitOp_IDEN1, a.bt);
  }
  BitTable(const C::BitTable& a) {
    bt = cons1_BitTable (a.sz);
    op_BitTable (bt, BitOp_IDEN1, a);
  }
  explicit BitTable(const std::vector<bool>& a) {
    bt = dflt_BitTable ();
    (*this) = a;
  }
  BitTable& operator=(const BitTable& a) {
    size_fo_BitTable (&bt, a.sz());
    op_BitTable (bt, BitOp_IDEN1, a.bt);
    return *this;
  }
  BitTable& operator=(const std::vector<bool>& a) {
    size_fo_BitTable (&bt, a.size());
    for (ujint i = 0; i < bt.sz; ++i) {
      setb_BitTable (bt, i, a[i] ? 1 : 0);
    }
    return *this;
  }
  ~BitTable() {
    lose_BitTable (&bt);
  }

  ujint sz() const {
    return bt.sz;
  }
  ujint size() const { return this->sz(); }

  void wipe(Bit b) {
    wipe_BitTable (bt, b);
  }

  void grow(ujint capac) {
    grow_BitTable (&bt, capac);
  }

  void resize(ujint capac) {
    size_fo_BitTable (&bt, capac);
  }
  void clear() {
    this->resize(0);
  }

  Bit operator[](ujint i) const {
    return ck_BitTable (bt, i);
  }

  BitTableElement operator[](ujint i) {
    return BitTableElement(bt, i);
  }

  bool operator==(const BitTable& b) const {
    return (this->cmp(b) == 0);
  }
  bool operator!=(const BitTable& b) const {
    return !(*this == b);
  }

  Sign cmp(const BitTable& b) const
  {
    return cmp_BitTable (bt, b.bt);
  }

  bool operator<=(const BitTable& b) const {
    return (this->cmp(b) <= 0);
  }
  bool operator<(const BitTable& b) const {
    return (this->cmp(b) < 0);
  }
  bool operator>(const BitTable& b) const {
    return (this->cmp(b) > 0);
  }
  bool operator>=(const BitTable& b) const {
    return (this->cmp(b) >= 0);
  }

  bool subseteq_ck(const BitTable& b) const {
    return fold_map2_BitTable (BitOp_AND, BitOp_IMP, this->bt, b.bt);
  }

  ujint begidx() const {
    return begidx_BitTable (bt);
  }
  ujint nextidx(ujint i) const {
    return nextidx_BitTable (bt, i);
  }

};

  ujint
sz_of (const BitTable& t)
{
  return t.sz();
}
}

#endif

