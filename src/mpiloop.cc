
#include "mpiloop.hh"

#include "cx/map.hh"
#include <algorithm>

namespace Cx {

MpiLoop::MpiLoop(int _comm_tag, MPI_Comm _comm)
  : comm_tag(_comm_tag)
  , comm(_comm)
  , done(false)
  , alldone(false)
{
  {
    int _PcIdx = 0;
    int _NPcs = 0;
    MPI_Comm_rank (this->comm, &_PcIdx);
    MPI_Comm_size (comm, &_NPcs);
    this->PcIdx = _PcIdx;
    this->NPcs = _NPcs;
  }

  payloads.grow(xo_sz(), 0);
  requests.grow(xo_sz(), MPI_REQUEST_NULL);
  statuses.grow(xo_sz());
  indices.grow(xo_sz());
}

  uint
MpiLoop::begin_from(uint i)
{
  min_complete = i;
  max_idx = i;
  if (PcIdx == 0) {
    for (uint i = 0; i < x_sz(); ++i)
      *x_payload(i) = 1;
    for (uint i = 0; i < o_sz(); ++i)
      *o_payload(i) = 0;
    for (uint i = 0; i < x_sz(); ++i) {
      MPI_Irecv(x_payload(i), 1, MPI_UNSIGNED,
                x_hood(i), comm_tag, comm,
                x_request(i));
    }
  }
  return next();
}

  uint
MpiLoop::next()
{
  if (alldone) {
    // Nothing.
  }
  else if (PcIdx==0) {
    int count = 0;
    MPI_Waitsome(xo_sz(), &requests[0],
                 &count, &indices[0],
                 &statuses[0]);
    if (count == MPI_UNDEFINED || count == 0)
      return 0;
    for (int indices_idx = 0; indices_idx < count; ++indices_idx) {
      uint i = (uint) indices[indices_idx];
      if (i >= x_sz())
        i -= x_sz();
      if (*x_request(i) != MPI_REQUEST_NULL)  continue;
      if (*o_request(i) != MPI_REQUEST_NULL)  continue;

      pending.erase(*o_payload(i));
      if (*x_payload(i) == 0) {
        done = true;
        check_alldone();
        *o_payload(i) = 0;
        continue;
      }

      if (done) {
        *o_payload(i) = 0;
      }
      else {
        if (!pending.empty()) {
          min_complete = *pending.begin();
        }
        else {
          min_complete = max_idx;
        }
        max_idx += 1;
        // Send an index that is 1 greater than it should be.
        *o_payload(i) = max_idx;
      }

      MPI_Isend(o_payload(i), 1, MPI_UNSIGNED,
                o_hood(i), comm_tag, comm,
                o_request(i));
      MPI_Irecv(x_payload(i), 1, MPI_UNSIGNED,
                x_hood(i), comm_tag, comm,
                x_request(i));

    }
    return min_complete;
  }
  else if (done) {
    uint i = 0;
    *o_payload(i) = 0;
    MPI_Send(o_payload(i), 1, MPI_UNSIGNED,
             o_hood(i), comm_tag, comm);
    alldone = true;
  }
  else {
    uint i = 0;
    *o_payload(i) = 1;
    MPI_Send(o_payload(i), 1, MPI_UNSIGNED,
             o_hood(i), comm_tag, comm);
    MPI_Recv(x_payload(i), 1, MPI_UNSIGNED,
             x_hood(i), comm_tag, comm,
             &statuses[i]);
    max_idx = *x_payload(i);
    if (max_idx == 0)
      this->done_fo();
    else
      max_idx -= 1;
    return max_idx;
  }

  return 0;
}


  bool
MpiLoop::done_ck()
{
  if (done) {
    finish();
    return true;
  }
  return false;
}

  void
MpiLoop::done_fo()
{
  done = true;
}

  void
MpiLoop::check_alldone()
{
  if (PcIdx == 0 && done && !alldone) {
    bool tmp_alldone = true;
    for (uint i = 0; i < x_sz(); ++i) {
      if (*x_payload(i) != 0) {
        tmp_alldone = false;
      }
    }
    alldone = tmp_alldone;
  }
}

  void
MpiLoop::finish()
{
  this->done_fo();
  while (!alldone) {
    next();
    check_alldone();
  }
}

  void
MpiLoop::reset()
{
  this->finish();
  done = false;
  alldone = false;
}

}

