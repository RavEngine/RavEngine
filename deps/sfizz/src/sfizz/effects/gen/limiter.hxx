#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
name: "limiter"
Code generated with Faust 2.30.5 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __faustLimiter_H__
#define  __faustLimiter_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faustLimiter
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faustLimiter {
	//[Begin:class]

	
 private:
	
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	float fConst6;
	float fRec2[2];
	float fRec1[2];
	float fRec0[2];
	float fRec5[2];
	float fRec4[2];
	float fRec3[2];
	
 public:
	

	static constexpr int getNumInputs() {
		return 2;
	}
	static constexpr int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
		//[Begin:classInit]
		//[End:classInit]
	}
	
	void instanceConstants(int sample_rate) {
		//[Begin:instanceConstants]
		fSampleRate = sample_rate;
		fConst0 = float(fSampleRate);
		fConst1 = std::exp((0.0f - (2500.0f / fConst0)));
		fConst2 = (1.0f - fConst1);
		fConst3 = std::exp((0.0f - (1250.0f / fConst0)));
		fConst4 = (1.0f - fConst3);
		fConst5 = std::exp((0.0f - (2.0f / fConst0)));
		fConst6 = (1.0f - fConst5);
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		//[End:instanceResetUserInterface]
	}
	
	void instanceClear() {
		//[Begin:instanceClear]
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec2[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec0[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec5[l3] = 0.0f;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec4[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec3[l5] = 0.0f;
		}
		//[End:instanceClear]
	}
	
	void init(int sample_rate) {
		//[Begin:init]
		classInit(sample_rate);
		instanceInit(sample_rate);
		//[End:init]
	}
	void instanceInit(int sample_rate) {
		//[Begin:instanceInit]
		instanceConstants(sample_rate);
		instanceResetUserInterface();
		instanceClear();
		//[End:instanceInit]
	}
	
	
	int getSampleRate() {
		return fSampleRate;
	}
	
	
	void compute(int count, FAUSTFLOAT const* const* inputs, FAUSTFLOAT* const* outputs) {
		//[Begin:compute]
		FAUSTFLOAT const* input0 = inputs[0];
		FAUSTFLOAT const* input1 = inputs[1];
		FAUSTFLOAT* output0 = outputs[0];
		FAUSTFLOAT* output1 = outputs[1];
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			float fTemp1 = float(input1[i]);
			float fTemp2 = std::fabs(fTemp0);
			fRec2[0] = std::max<float>(fTemp2, ((fConst5 * fRec2[1]) + (fConst6 * fTemp2)));
			fRec1[0] = ((fConst3 * fRec1[1]) + (fConst4 * fRec2[0]));
			fRec0[0] = ((fConst1 * fRec0[1]) + (fConst2 * ((fRec1[0] > 1.0f) ? (1.0f / fRec1[0]) : 1.0f)));
			output0[i] = FAUSTFLOAT((fTemp0 * fRec0[0]));
			float fTemp3 = std::fabs(fTemp1);
			fRec5[0] = std::max<float>(fTemp3, ((fConst5 * fRec5[1]) + (fConst6 * fTemp3)));
			fRec4[0] = ((fConst3 * fRec4[1]) + (fConst4 * fRec5[0]));
			fRec3[0] = ((fConst1 * fRec3[1]) + (fConst2 * ((fRec4[0] > 1.0f) ? (1.0f / fRec4[0]) : 1.0f)));
			output1[i] = FAUSTFLOAT((fTemp1 * fRec3[0]));
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
			fRec5[1] = fRec5[0];
			fRec4[1] = fRec4[0];
			fRec3[1] = fRec3[0];
		}
		//[End:compute]
	}


	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
