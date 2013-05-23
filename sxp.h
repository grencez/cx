/**
 * \file sxp.h
 * Base for S-expressions.
 * TODO: Not yet in use.
 **/
#ifndef Sxp_H_
#define Sxp_H_
#include "lgtable.h"
#include "fileb.h"

typedef struct SxpBase SxpBase;
typedef SxpBase* Sxp;
typedef struct SxpCtx SxpCtx;
typedef struct SxpVT SxpVT;

struct SxpBase
{
  SxpCtx* ctx;
};

struct SxpCtx
{
  LgTable cells;
  const SxpOpVT* vt;
};

struct SxpOpVT
{
  void (*car_fn) (SxpCtx*, Sxp*, const Sxp);
  void (*cdr_fn) (SxpCtx*, Sxp*, const Sxp);

  PFmla (*make_fn) (SxpCtx*);
  void (*free_fn) (SxpCtx*, Sxp);

  void* (*ctx_lose_fn) (SxpCtx*);
};

#endif
