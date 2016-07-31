
/** Chris Lomont's random number generator.
 * From: http://stackoverflow.com/a/1227137/5039395
 **/
qual_inline
  uint32_t
uint32_WELL512 (URandom* ctx)
{
  uint32_t* state = ctx->state;
#define index  ctx->state[16]
  uint32_t a, b, c, d;
  a = state[index];
  c = state[(index+13)&15];
  b = a^c^(a<<16)^(c<<15);
  c = state[(index+9)&15];
  c ^= (c>>11);
  a = state[index] = b^c;
  d = a^((a<<5)&0xDA442D24UL);
  index = (index + 15)&15;
  a = state[index];
  state[index] = a^b^d^(a<<2)^(b<<18)^(c<<28);
  return state[index];
}

qual_inline
  void
init_WELL512 (URandom* ctx)
{
  index = 0;
#undef index
}

