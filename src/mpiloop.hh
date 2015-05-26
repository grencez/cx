
#ifndef MpiLoop_HH_
#define MpiLoop_HH_

#include <mpi.h>
#include "cx/synhax.hh"
#include "cx/set.hh"
#include "cx/table.hh"

namespace Cx {
class MpiLoop
{
private:
  int comm_tag;
  MPI_Comm comm;
  bool done;
  bool alldone;

public:
  uint min_complete;
private:
  uint max_idx;
  Cx::Set<uint> pending;

  uint PcIdx;
  uint NPcs;


  Cx::Table<uint> payloads;
  Cx::Table<MPI_Request> requests;
  Cx::Table<MPI_Status > statuses;
  Cx::Table< int > indices;

public:
  MpiLoop(int _comm_tag, MPI_Comm _comm);

private:
  uint x_sz() const { return PcIdx==0 ? NPcs-1 : 1; }
  uint o_sz() const { return x_sz(); }
  uint xo_sz() const { return 2*x_sz(); }
  int x_hood(uint i) const { return PcIdx==0 ? i+1 : 0; }
  int o_hood(uint i) const { return PcIdx==0 ? i+1 : 0; }
  uint* x_payload(uint i) { return &payloads[i]; }
  uint* o_payload(uint i) { return &payloads[x_sz() + i]; }
  MPI_Request* x_request(uint i) { return &requests[i]; }
  MPI_Request* o_request(uint i) { return &requests[this->x_sz() + i]; }

public:
  uint begin_from(uint i);
  uint next();

  bool done_ck();
  void done_fo();
  void check_alldone();
  void finish();
  void reset();
};
}

#endif

