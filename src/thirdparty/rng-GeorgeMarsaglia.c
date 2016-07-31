
/** KISS RNG.
 *
 * Multiply-With-Carry combined with some others.
 *
 * http://www.cse.yorku.ca/~oz/marsaglia-rng.html
 **/
qual_inline
  uint32_t
uint32_GMRand (URandom* ctx)
{
#define k0  36969
#define k1  18000
#define z  ctx->state[0]
#define w  ctx->state[1]
#define jsr  ctx->state[2]
#define jcong  ctx->state[3]

  z = k0 * MASK16(z) + (z >> 16);
  w = k1 * MASK16(w) + (w >> 16);
#define MWC  ((z << 16) + w)

  jsr ^= (jsr << 17);
  jsr ^= (jsr >> 13);
  jsr ^= (jsr << 5);
#define SHR3  jsr

  jcong = 69069 * jcong + 1234567;
#define CONG  jcong

#define KISS  ((MWC ^ CONG) + SHR3)
  return KISS;

#undef MWC
#undef SHR3
#undef CONG
#undef KISS
}

/** Initialization.**/
qual_inline
  void
init_GMRand (URandom* ctx)
{
  /* Instead of 36969 and 18000 for the constants {k0} and {k1},
   * you may use any  pair of distinct constants from this list.
   * (or any other 16-bit constants k for which both k*2^16-1
   * and k*2^15-1 are prime)
   */
#if 0
  static const uint32_t ks[] =
  {
    18000, 18030, 18273, 18513, 18879, 19074, 19098, 19164, 19215, 19584,
    19599, 19950, 20088, 20508, 20544, 20664, 20814, 20970, 21153, 21243,
    21423, 21723, 21954, 22125, 22188, 22293, 22860, 22938, 22965, 22974,
    23109, 23124, 23163, 23208, 23508, 23520, 23553, 23658, 23865, 24114,
    24219, 24660, 24699, 24864, 24948, 25023, 25308, 25443, 26004, 26088,
    26154, 26550, 26679, 26838, 27183, 27258, 27753, 27795, 27810, 27834,
    27960, 28320, 28380, 28689, 28710, 28794, 28854, 28959, 28980, 29013,
    29379, 29889, 30135, 30345, 30459, 30714, 30903, 30963, 31059, 31083
  };
  const uint n = ArraySz( ks );
#endif
  (void) ctx;

  /* Random seeds used to set {z}, {w}, {jsr}, and {jcong}.*/

#undef k0
#undef k1
#undef z
#undef w
#undef jsr
#undef jcong
}

