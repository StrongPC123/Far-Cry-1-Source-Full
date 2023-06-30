#ifndef __RADMEMH__
  #define __RADMEMH__

  #ifndef __RADBASEH__
    #include "radbase.h"
  #endif

  RADDEFSTART

  typedef void PTR4* (RADLINK PTR4* RADMEMALLOC) (U32 bytes);
  typedef void       (RADLINK PTR4* RADMEMFREE)  (void PTR4* ptr);

  #ifdef __RADMAC__
    #pragma export on
  #endif
  RADEXPFUNC void RADEXPLINK RADSetMemory(RADMEMALLOC a,RADMEMFREE f);
  #ifdef __RADMAC__
    #pragma export off
  #endif

  RADEXPFUNC void PTR4* RADEXPLINK radmalloc(U32 numbytes);
  RADEXPFUNC void RADEXPLINK radfree(void PTR4* ptr);

  RADDEFEND

#endif