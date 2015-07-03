
/** Generate output for RNG test.
 *
 * For example, the following tests 8 streams:
 *   ./bld/rngtest 8 | dieharder -g 200 -a
 **/

#include "cx/syscx.h"
#include "cx/urandom.h"
#include "cx/table.h"
#include <stdio.h>

int main(int argc, char** argv)
{
  int argi = init_sysCx (&argc, &argv);
  DeclTableT( URandom, URandom );
  DeclTable( URandom, urandoms );
  uint32_t buf[2048];
  uint bufsz = ArraySz(buf);
  FILE* out = stdout;
  uint n = 1;
  uint idx = 0;

  if (argi < argc) {
    n = atoi(argv[argi++]);
    if (n == 0) {
      return 1;
    }
  }

  for (i ; n) {
    GrowTable( urandoms, 1 );
    init2_URandom (&urandoms.s[i], i, n);
  }

  do {
    for (i ; bufsz) {
      buf[i] = uint32_URandom (&urandoms.s[idx]);
      idx = (idx + 1) % n;
    }
  } while (fwrite(buf, sizeof(buf[0]), bufsz, out) > 0);

  LoseTable( urandoms );
  lose_sysCx ();
  return 0;
}
