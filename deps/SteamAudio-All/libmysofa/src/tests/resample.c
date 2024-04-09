#include "../hrtf/mysofa.h"
#include "../hrtf/tools.h"
#include "tests.h"
#include <assert.h>
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

void test_resample() {
  struct MYSOFA_HRTF *hrtf;
  int err = 0, i;
  float *backupIr;
  float *backupDelays;
  int resampFactor = 2;    // integer, >= 1
  int num_ir_to_test = 10; // number of filters to test in the IR, to avoid
                           // going through all potential positions / channels
  float irPeakValue = 0.0;

  // hrtf = mysofa_load("tests/Pulse.sofa", &err);
  hrtf = mysofa_load("tests/CIPIC_subject_003_hrir_final_itdInDelayField.sofa",
                     &err);
  if (!hrtf) {
    CU_FAIL_FATAL("Error reading file.");
    return;
  }

  // backup (non resampled) IR
  backupIr = malloc(sizeof(float) * hrtf->N * num_ir_to_test);
  if (!backupIr) {
    CU_FAIL_FATAL("No memory, N (IR) is too large.");
    mysofa_free(hrtf);
    return;
  }

  for (i = 0; i < hrtf->N * num_ir_to_test; i++) {
    backupIr[i] = hrtf->DataIR.values[i];
    irPeakValue = fmax(irPeakValue, backupIr[i]);
  }

  // backup (non resampled) Delays
  backupDelays = malloc(sizeof(float) * hrtf->DataDelay.elements);
  if (!backupDelays) {
    CU_FAIL_FATAL("No memory, N (Delay) is too large.");
    mysofa_free(hrtf);
    return;
  }

  for (i = 0; i < hrtf->DataDelay.elements; i++) {
    backupDelays[i] = hrtf->DataDelay.values[i];
  }

  float fsOld = hrtf->DataSamplingRate.values[0];
  float fsNew = resampFactor * fsOld;
  err = mysofa_resample(hrtf, fsNew);
  CU_ASSERT_FATAL(err == MYSOFA_OK);

  // loop over delays (in samples), check resampling factor correctly applied
  float delayThresh = 0.001; // in samples, maximum acceptable error threshold
  for (i = 0; i < hrtf->DataDelay.elements; i += 2 * resampFactor) {
#ifdef VDEBUG
    printf("%f %f ", backupDelays[i] / fsOld,
           hrtf->DataDelay.values[i] / fsNew);
#endif
    CU_ASSERT(fabs(backupDelays[i] / fsOld -
                   hrtf->DataDelay.values[i] / fsNew) <= delayThresh);
  }

  // loop over IR to check if paired values still match after resampling
  float irThresh = 0.001 * irPeakValue; // relative acceptable error threshold
  for (i = 0; i < hrtf->N * num_ir_to_test; i += resampFactor) {

#ifdef VDEBUG
    printf("%f %f ", backupIr[i / resampFactor], hrtf->DataIR.values[i]);
    if ((i % hrtf->N) == (hrtf->N - 1))
      printf("\n");
#endif
    CU_ASSERT((fabs(hrtf->DataIR.values[i] - backupIr[i / resampFactor]) <=
               irThresh));
  }

  free(backupIr);
  free(backupDelays);
  mysofa_free(hrtf);
}
