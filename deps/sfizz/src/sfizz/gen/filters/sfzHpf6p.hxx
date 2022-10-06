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

#ifndef  __faustHpf6p_H__
#define  __faustHpf6p_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faustHpf6p
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faustHpf6p : public sfzFilterDsp {
	//[Begin:class]

	
 private:
	
	int fSampleRate;
	double fConst1;
	double fConst2;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fVslider0;
	double fRec2[2];
	double fVec0[2];
	double fRec7[2];
	double fVec1[2];
	double fRec8[2];
	double fVec2[2];
	double fRec9[2];
	double fRec6[2];
	double fRec5[2];
	double fVec3[2];
	double fVec4[2];
	double fVec5[2];
	double fRec4[2];
	double fRec3[2];
	double fVec6[2];
	double fVec7[2];
	double fVec8[2];
	double fRec1[2];
	double fRec0[2];
	
 public:
	

	static constexpr int getNumInputs() {
		return 1;
	}
	static constexpr int getNumOutputs() {
		return 1;
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
		fConst2 = (6.2831853071795862 / fConst0);
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
			fRec2[l0] = 0.0;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			fVec0[l1] = 0.0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec7[l2] = 0.0;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fVec1[l3] = 0.0;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec8[l4] = 0.0;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fVec2[l5] = 0.0;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fRec9[l6] = 0.0;
		}
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			fRec6[l7] = 0.0;
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec5[l8] = 0.0;
		}
		for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
			fVec3[l9] = 0.0;
		}
		for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
			fVec4[l10] = 0.0;
		}
		for (int l11 = 0; (l11 < 2); l11 = (l11 + 1)) {
			fVec5[l11] = 0.0;
		}
		for (int l12 = 0; (l12 < 2); l12 = (l12 + 1)) {
			fRec4[l12] = 0.0;
		}
		for (int l13 = 0; (l13 < 2); l13 = (l13 + 1)) {
			fRec3[l13] = 0.0;
		}
		for (int l14 = 0; (l14 < 2); l14 = (l14 + 1)) {
			fVec6[l14] = 0.0;
		}
		for (int l15 = 0; (l15 < 2); l15 = (l15 + 1)) {
			fVec7[l15] = 0.0;
		}
		for (int l16 = 0; (l16 < 2); l16 = (l16 + 1)) {
			fVec8[l16] = 0.0;
		}
		for (int l17 = 0; (l17 < 2); l17 = (l17 + 1)) {
			fRec1[l17] = 0.0;
		}
		for (int l18 = 0; (l18 < 2); l18 = (l18 + 1)) {
			fRec0[l18] = 0.0;
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
		FAUSTFLOAT* output0 = outputs[0];
		double fSlow0 = (fSmoothEnable ? fConst1 : 0.0);
		double fSlow1 = (fConst2 * std::max<double>(0.0, std::min<double>(20000.0, std::max<double>(1.0, double(fHslider0)))));
		double fSlow2 = std::cos(fSlow1);
		double fSlow3 = (0.5 * (std::sin(fSlow1) / std::max<double>(0.001, std::pow(10.0, (0.050000000000000003 * std::min<double>(60.0, std::max<double>(-60.0, double(fVslider0))))))));
		double fSlow4 = (fSlow3 + 1.0);
		double fSlow5 = (1.0 - fSlow0);
		double fSlow6 = (((-1.0 - fSlow2) / fSlow4) * fSlow5);
		double fSlow7 = ((0.5 * ((fSlow2 + 1.0) / fSlow4)) * fSlow5);
		double fSlow8 = (((1.0 - fSlow3) / fSlow4) * fSlow5);
		double fSlow9 = (((0.0 - (2.0 * fSlow2)) / fSlow4) * fSlow5);
		for (int i0 = 0; (i0 < count); i0 = (i0 + 1)) {
			double fTemp0 = double(input0[i0]);
			fRec2[0] = ((fSlow0 * fRec2[1]) + fSlow6);
			fVec0[0] = (fTemp0 * fRec2[0]);
			fRec7[0] = ((fSlow0 * fRec7[1]) + fSlow7);
			double fTemp1 = (fTemp0 * fRec7[0]);
			fVec1[0] = fTemp1;
			fRec8[0] = ((fSlow0 * fRec8[1]) + fSlow8);
			fVec2[0] = (fVec1[1] - (fRec8[0] * fRec5[1]));
			fRec9[0] = ((fSlow0 * fRec9[1]) + fSlow9);
			fRec6[0] = ((fVec0[1] + (fTemp1 + fVec2[1])) - (fRec9[0] * fRec6[1]));
			fRec5[0] = fRec6[0];
			fVec3[0] = (fRec2[0] * fRec5[0]);
			double fTemp2 = (fRec7[0] * fRec5[0]);
			fVec4[0] = fTemp2;
			fVec5[0] = (fVec4[1] - (fRec8[0] * fRec3[1]));
			fRec4[0] = ((fVec3[1] + (fTemp2 + fVec5[1])) - (fRec9[0] * fRec4[1]));
			fRec3[0] = fRec4[0];
			fVec6[0] = (fRec2[0] * fRec3[0]);
			double fTemp3 = (fRec7[0] * fRec3[0]);
			fVec7[0] = fTemp3;
			fVec8[0] = (fVec7[1] - (fRec8[0] * fRec0[1]));
			fRec1[0] = ((fVec6[1] + (fTemp3 + fVec8[1])) - (fRec9[0] * fRec1[1]));
			fRec0[0] = fRec1[0];
			output0[i0] = FAUSTFLOAT(fRec0[0]);
			fRec2[1] = fRec2[0];
			fVec0[1] = fVec0[0];
			fRec7[1] = fRec7[0];
			fVec1[1] = fVec1[0];
			fRec8[1] = fRec8[0];
			fVec2[1] = fVec2[0];
			fRec9[1] = fRec9[0];
			fRec6[1] = fRec6[0];
			fRec5[1] = fRec5[0];
			fVec3[1] = fVec3[0];
			fVec4[1] = fVec4[0];
			fVec5[1] = fVec5[0];
			fRec4[1] = fRec4[0];
			fRec3[1] = fRec3[0];
			fVec6[1] = fVec6[0];
			fVec7[1] = fVec7[0];
			fVec8[1] = fVec8[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
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
