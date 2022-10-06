#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
author: "Jean Pierre Cimalando"
license: "BSD-2-Clause"
name: "sfz_filters"
Code generated with Faust 2.37.3 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -double -ftz 0
------------------------------------------------------------ */

#ifndef  __faust2chHpf2pSv_H__
#define  __faust2chHpf2pSv_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faust2chHpf2pSv
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faust2chHpf2pSv : public sfzFilterDsp {
	//[Begin:class]

	
 private:
	
	int fSampleRate;
	double fConst1;
	double fConst2;
	FAUSTFLOAT fHslider0;
	double fRec4[2];
	FAUSTFLOAT fVslider0;
	double fRec3[2];
	double fRec5[2];
	double fRec1[2];
	double fRec2[2];
	double fRec7[2];
	double fRec8[2];
	
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
		double fConst0 = double(fSampleRate);
		fConst1 = std::exp((0.0 - (1000.0 / fConst0)));
		fConst2 = (3.1415926535897931 / fConst0);
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fHslider0 = FAUSTFLOAT(440.0);
		fVslider0 = FAUSTFLOAT(0.0);
		//[End:instanceResetUserInterface]
	}
	
	void instanceClear() {
		//[Begin:instanceClear]
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec4[l0] = 0.0;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			fRec3[l1] = 0.0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec5[l2] = 0.0;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec1[l3] = 0.0;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec2[l4] = 0.0;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec7[l5] = 0.0;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fRec8[l6] = 0.0;
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
		double fSlow0 = (fSmoothEnable ? fConst1 : 0.0);
		double fSlow1 = (1.0 - fSlow0);
		double fSlow2 = (std::tan((fConst2 * std::min<double>(20000.0, std::max<double>(1.0, double(fHslider0))))) * fSlow1);
		double fSlow3 = (1.0 / std::pow(10.0, (0.050000000000000003 * std::min<double>(60.0, std::max<double>(-60.0, double(fVslider0))))));
		for (int i0 = 0; (i0 < count); i0 = (i0 + 1)) {
			double fTemp0 = double(input0[i0]);
			double fTemp1 = double(input1[i0]);
			fRec4[0] = ((fSlow0 * fRec4[1]) + fSlow2);
			double fTemp2 = (fSlow3 + fRec4[0]);
			fRec3[0] = ((fSlow0 * fRec3[1]) + (fSlow1 / ((fRec4[0] * fTemp2) + 1.0)));
			fRec5[0] = ((fSlow0 * fRec5[1]) + (fSlow1 * fTemp2));
			double fTemp3 = (fTemp0 - (fRec1[1] + (fRec5[0] * fRec2[1])));
			double fRec0 = (fRec3[0] * fTemp3);
			double fTemp4 = (fRec4[0] * fRec3[0]);
			double fTemp5 = (fTemp4 * fTemp3);
			double fTemp6 = (fRec2[1] + fTemp5);
			fRec1[0] = (fRec1[1] + (2.0 * (fRec4[0] * fTemp6)));
			double fTemp7 = (fRec2[1] + (2.0 * fTemp5));
			fRec2[0] = fTemp7;
			output0[i0] = FAUSTFLOAT(fRec0);
			double fTemp8 = (fTemp1 - (fRec7[1] + (fRec5[0] * fRec8[1])));
			double fRec6 = (fRec3[0] * fTemp8);
			double fTemp9 = (fTemp4 * fTemp8);
			double fTemp10 = (fRec8[1] + fTemp9);
			fRec7[0] = (fRec7[1] + (2.0 * (fRec4[0] * fTemp10)));
			double fTemp11 = (fRec8[1] + (2.0 * fTemp9));
			fRec8[0] = fTemp11;
			output1[i0] = FAUSTFLOAT(fRec6);
			fRec4[1] = fRec4[0];
			fRec3[1] = fRec3[0];
			fRec5[1] = fRec5[0];
			fRec1[1] = fRec1[0];
			fRec2[1] = fRec2[0];
			fRec7[1] = fRec7[0];
			fRec8[1] = fRec8[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getCutoff() const { return fHslider0; }
    void setCutoff(FAUSTFLOAT value) { fHslider0 = value; }

    FAUSTFLOAT getResonance() const { return fVslider0; }
    void setResonance(FAUSTFLOAT value) { fVslider0 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
