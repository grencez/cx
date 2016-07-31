
#include "thirdparty/resource-usage-DavidRobertNadeau.c"

  size_t
peak_memory_use_sysCx ()
{
  return getPeakRSS();
}

  size_t
memory_use_sysCx ()
{
  return getCurrentRSS();
}

