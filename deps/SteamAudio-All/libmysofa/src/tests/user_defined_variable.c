#include "../hrtf/tools.h"
#include "tests.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static void user_defined_variable(char *filename) {
  struct MYSOFA_HRTF *hrtf;
  int err;

  hrtf = mysofa_load(filename, &err);

  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  CU_ASSERT(hrtf->variables != NULL);
  CU_ASSERT(hrtf->variables->next == NULL);
  CU_ASSERT(!strcmp(hrtf->variables->name, "GLOBALAdditionalVariable"));
  CU_ASSERT(hrtf->variables->value != NULL);
  CU_ASSERT(hrtf->variables->value->values != NULL);
  CU_ASSERT(hrtf->variables->value->elements ==
            hrtf->I * hrtf->M); // must be 836

  mysofa_free(hrtf);
}

void test_user_defined_variable() {
  user_defined_variable(
      "tests/example_dummy_sofa48_with_user_defined_variable.sofa");
}
