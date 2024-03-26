#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "tests.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void test_cache() {
  char *filename1 = "build/sofacoustics.org/data/sofa_api_mo_test/Pulse.sofa";
  char *filename2 = "tests/tester.sofa";
  float sr1 = 48000;
  float sr2 = 8000;

  struct MYSOFA_EASY *easy1 = malloc(sizeof(struct MYSOFA_EASY));
  struct MYSOFA_EASY *easy2 = malloc(sizeof(struct MYSOFA_EASY));
  bzero(easy1, sizeof(struct MYSOFA_EASY));
  bzero(easy2, sizeof(struct MYSOFA_EASY));

  mysofa_close(easy2); /* must pass without segfail */

  CU_ASSERT(!mysofa_cache_lookup(filename1, sr1)); /* no entry so far */
  CU_ASSERT(mysofa_cache_store(easy1, filename1, sr1) == easy1); /* add */
  CU_ASSERT(mysofa_cache_lookup(filename1, sr1) ==
            easy1); /* check whether easy1 has been cached. */

  mysofa_cache_release_all();                      /* remove all */
  CU_ASSERT(!mysofa_cache_lookup(filename1, sr1)); /* cache must be empty now */

  /*
   mysofa_cache_release(easy1);
   free(easy1)

   must segfail
   */

  easy1 = malloc(sizeof(struct MYSOFA_EASY));
  bzero(easy1, sizeof(struct MYSOFA_EASY));

  CU_ASSERT(mysofa_cache_store(easy1, filename1, sr1) == easy1); /* add again */

  easy2 = malloc(
      sizeof(struct MYSOFA_EASY)); /* easy2 has been freed automatically */
  bzero(easy2, sizeof(struct MYSOFA_EASY));

  CU_ASSERT(mysofa_cache_store(easy2, filename1, sr1) ==
            easy1); /* second add must be possible too, return cached */

  easy2 = malloc(
      sizeof(struct MYSOFA_EASY)); /* easy2 has been freed automatically */
  bzero(easy2, sizeof(struct MYSOFA_EASY));

  CU_ASSERT(mysofa_cache_store(easy2, filename1, sr2) ==
            easy2); /* now third add with different sample rate */

  CU_ASSERT(mysofa_cache_lookup(filename1, sr2) == easy2);
  mysofa_cache_release(easy2);
  mysofa_cache_release(easy2);
  CU_ASSERT(!mysofa_cache_lookup(filename1, sr2));

  easy2 = malloc(sizeof(struct MYSOFA_EASY));
  bzero(easy2, sizeof(struct MYSOFA_EASY));

  CU_ASSERT(mysofa_cache_store(easy2, filename2, sr2) ==
            easy2); /* now third add with different file name */
  CU_ASSERT(mysofa_cache_lookup(filename2, sr2) == easy2);
  mysofa_cache_release(easy2);
  mysofa_cache_release(easy2);
  CU_ASSERT(!mysofa_cache_lookup(filename1, sr2));
  mysofa_cache_release_all(); /* remove all */
}
