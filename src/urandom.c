
#include "urandom.h"
#include "bittable.h"

  uint
uint_SysURandom(uint n)
{
  const uint max = n-1;
  FixDeclBitTable( bt, NBits_uint, 0 );
  const uint nbits = lg_ujint (max) + 1;
  const uint nbytes = ceil_quot(nbits, NBits_byte);
  uint x;

  do {
    randomize(bt.s, nbytes);

    /* We can assume each bit is uniformly random,
     * so truncate as much as possible without excluding {max}.
     */
    x = get_uint_BitTable (bt, 0, nbits);

    /* If {x} is outside of our range, then try again.
     * This has less than a 50% chance of happening, so this loop will terminate eventually.
     */
  } while (x > max);

  return x;
}

