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

#ifndef  __faust2chBpf6p_H__
#define  __faust2chBpf6p_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faust2chBpf6p
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faust2chBpf6p : public sfzFilterDsp {
	//[Begin:class]

	
 private:
	
	int fSampleRate;
	double fConst1;
	double fConst2;
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fVslider0;
	double fRec2[2];
	double fRec7[2];
	double fVec0[2];
	double fRec8[2];
	double fVec1[2];
	double fRec9[2];
	double fVec2[2];
	double fRec10[2];
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
	double fVec9[2];
	double fVec10[2];
	double fVec11[2];
	double fRec16[2];
	double fRec15[2];
	double fVec12[2];
	double fVec13[2];
	double fVec14[2];
	double fRec14[2];
	double fRec13[2];
	double fVec15[2];
	double fVec16[2];
	double fVec17[2];
	double fRec12[2];
	double fRec11[2];
	
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
			fRec7[l1] = 0.0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fVec0[l2] = 0.0;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec8[l3] = 0.0;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fVec1[l4] = 0.0;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec9[l5] = 0.0;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fVec2[l6] = 0.0;
		}
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			fRec10[l7] = 0.0;
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec6[l8] = 0.0;
		}
		for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
			fRec5[l9] = 0.0;
		}
		for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
			fVec3[l10] = 0.0;
		}
		for (int l11 = 0; (l11 < 2); l11 = (l11 + 1)) {
			fVec4[l11] = 0.0;
		}
		for (int l12 = 0; (l12 < 2); l12 = (l12 + 1)) {
			fVec5[l12] = 0.0;
		}
		for (int l13 = 0; (l13 < 2); l13 = (l13 + 1)) {
			fRec4[l13] = 0.0;
		}
		for (int l14 = 0; (l14 < 2); l14 = (l14 + 1)) {
			fRec3[l14] = 0.0;
		}
		for (int l15 = 0; (l15 < 2); l15 = (l15 + 1)) {
			fVec6[l15] = 0.0;
		}
		for (int l16 = 0; (l16 < 2); l16 = (l16 + 1)) {
			fVec7[l16] = 0.0;
		}
		for (int l17 = 0; (l17 < 2); l17 = (l17 + 1)) {
			fVec8[l17] = 0.0;
		}
		for (int l18 = 0; (l18 < 2); l18 = (l18 + 1)) {
			fRec1[l18] = 0.0;
		}
		for (int l19 = 0; (l19 < 2); l19 = (l19 + 1)) {
			fRec0[l19] = 0.0;
		}
		for (int l20 = 0; (l20 < 2); l20 = (l20 + 1)) {
			fVec9[l20] = 0.0;
		}
		for (int l21 = 0; (l21 < 2); l21 = (l21 + 1)) {
			fVec10[l21] = 0.0;
		}
		for (int l22 = 0; (l22 < 2); l22 = (l22 + 1)) {
			fVec11[l22] = 0.0;
		}
		for (int l23 = 0; (l23 < 2); l23 = (l23 + 1)) {
			fRec16[l23] = 0.0;
		}
		for (int l24 = 0; (l24 < 2); l24 = (l24 + 1)) {
			fRec15[l24] = 0.0;
		}
		for (int l25 = 0; (l25 < 2); l25 = (l25 + 1)) {
			fVec12[l25] = 0.0;
		}
		for (int l26 = 0; (l26 < 2); l26 = (l26 + 1)) {
			fVec13[l26] = 0.0;
		}
		for (int l27 = 0; (l27 < 2); l27 = (l27 + 1)) {
			fVec14[l27] = 0.0;
		}
		for (int l28 = 0; (l28 < 2); l28 = (l28 + 1)) {
			fRec14[l28] = 0.0;
		}
		for (int l29 = 0; (l29 < 2); l29 = (l29 + 1)) {
			fRec13[l29] = 0.0;
		}
		for (int l30 = 0; (l30 < 2); l30 = (l30 + 1)) {
			fVec15[l30] = 0.0;
		}
		for (int l31 = 0; (l31 < 2); l31 = (l31 + 1)) {
			fVec16[l31] = 0.0;
		}
		for (int l32 = 0; (l32 < 2); l32 = (l32 + 1)) {
			fVec17[l32] = 0.0;
		}
		for (int l33 = 0; (l33 < 2); l33 = (l33 + 1)) {
			fRec12[l33] = 0.0;
		}
		for (int l34 = 0; (l34 < 2); l34 = (l34 + 1)) {
			fRec11[l34] = 0.0;
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
		double fSlow1 = (fConst2 * std::max<double>(0.0, std::min<double>(20000.0, std::max<double>(1.0, double(fHslider0)))));
		double fSlow2 = std::sin(fSlow1);
		double fSlow3 = std::max<double>(0.001, std::pow(10.0, (0.050000000000000003 * std::min<double>(60.0, std::max<double>(-60.0, double(fVslider0))))));
		double fSlow4 = (0.5 * (fSlow2 / fSlow3));
		double fSlow5 = (fSlow4 + 1.0);
		double fSlow6 = (0.5 * (fSlow2 / (fSlow3 * fSlow5)));
		double fSlow7 = (1.0 - fSlow0);
		double fSlow8 = ((0.0 - fSlow6) * fSlow7);
		double fSlow9 = (fSlow6 * fSlow7);
		double fSlow10 = (((1.0 - fSlow4) / fSlow5) * fSlow7);
		double fSlow11 = (((0.0 - (2.0 * std::cos(fSlow1))) / fSlow5) * fSlow7);
		for (int i0 = 0; (i0 < count); i0 = (i0 + 1)) {
			double fTemp0 = double(input0[i0]);
			double fTemp1 = double(input1[i0]);
			fRec2[0] = ((fSlow0 * fRec2[1]) + fSlow8);
			fRec7[0] = (fSlow0 * fRec7[1]);
			fVec0[0] = (fTemp0 * fRec7[0]);
			fRec8[0] = ((fSlow0 * fRec8[1]) + fSlow9);
			fVec1[0] = (fTemp0 * fRec2[0]);
			fRec9[0] = ((fSlow0 * fRec9[1]) + fSlow10);
			fVec2[0] = (fVec1[1] - (fRec9[0] * fRec5[1]));
			fRec10[0] = ((fSlow0 * fRec10[1]) + fSlow11);
			fRec6[0] = ((fVec0[1] + ((fTemp0 * fRec8[0]) + fVec2[1])) - (fRec10[0] * fRec6[1]));
			fRec5[0] = fRec6[0];
			fVec3[0] = (fRec2[0] * fRec5[0]);
			fVec4[0] = (fVec3[1] - (fRec9[0] * fRec3[1]));
			fVec5[0] = (fRec7[0] * fRec5[0]);
			fRec4[0] = (((fVec4[1] + fVec5[1]) + (fRec8[0] * fRec5[0])) - (fRec10[0] * fRec4[1]));
			fRec3[0] = fRec4[0];
			fVec6[0] = (fRec2[0] * fRec3[0]);
			fVec7[0] = (fVec6[1] - (fRec9[0] * fRec0[1]));
			fVec8[0] = (fRec7[0] * fRec3[0]);
			fRec1[0] = (((fVec7[1] + fVec8[1]) + (fRec8[0] * fRec3[0])) - (fRec10[0] * fRec1[1]));
			fRec0[0] = fRec1[0];
			output0[i0] = FAUSTFLOAT(fRec0[0]);
			fVec9[0] = (fTemp1 * fRec7[0]);
			fVec10[0] = (fTemp1 * fRec2[0]);
			fVec11[0] = (fVec10[1] - (fRec9[0] * fRec15[1]));
			fRec16[0] = ((fVec9[1] + ((fTemp1 * fRec8[0]) + fVec11[1])) - (fRec10[0] * fRec16[1]));
			fRec15[0] = fRec16[0];
			fVec12[0] = (fRec2[0] * fRec15[0]);
			fVec13[0] = (fVec12[1] - (fRec9[0] * fRec13[1]));
			fVec14[0] = (fRec7[0] * fRec15[0]);
			fRec14[0] = (((fVec13[1] + fVec14[1]) + (fRec8[0] * fRec15[0])) - (fRec10[0] * fRec14[1]));
			fRec13[0] = fRec14[0];
			fVec15[0] = (fRec2[0] * fRec13[0]);
			fVec16[0] = (fVec15[1] - (fRec9[0] * fRec11[1]));
			fVec17[0] = (fRec7[0] * fRec13[0]);
			fRec12[0] = (((fVec16[1] + fVec17[1]) + (fRec8[0] * fRec13[0])) - (fRec10[0] * fRec12[1]));
			fRec11[0] = fRec12[0];
			output1[i0] = FAUSTFLOAT(fRec11[0]);
			fRec2[1] = fRec2[0];
			fRec7[1] = fRec7[0];
			fVec0[1] = fVec0[0];
			fRec8[1] = fRec8[0];
			fVec1[1] = fVec1[0];
			fRec9[1] = fRec9[0];
			fVec2[1] = fVec2[0];
			fRec10[1] = fRec10[0];
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
			fVec9[1] = fVec9[0];
			fVec10[1] = fVec10[0];
			fVec11[1] = fVec11[0];
			fRec16[1] = fRec16[0];
			fRec15[1] = fRec15[0];
			fVec12[1] = fVec12[0];
			fVec13[1] = fVec13[0];
			fVec14[1] = fVec14[0];
			fRec14[1] = fRec14[0];
			fRec13[1] = fRec13[0];
			fVec15[1] = fVec15[0];
			fVec16[1] = fVec16[0];
			fVec17[1] = fVec17[0];
			fRec12[1] = fRec12[0];
			fRec11[1] = fRec11[0];
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
