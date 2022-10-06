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

#ifndef  __faustEqLshelf_H__
#define  __faustEqLshelf_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

/* link with : "" */
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faustEqLshelf
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faustEqLshelf : public sfzFilterDsp {
	//[Begin:class]

	
 private:
	
	int fSampleRate;
	double fConst1;
	FAUSTFLOAT fVslider0;
	double fConst2;
	FAUSTFLOAT fHslider0;
	double fConst3;
	FAUSTFLOAT fVslider1;
	double fRec2[2];
	double fVec0[2];
	double fRec3[2];
	double fRec4[2];
	double fVec1[2];
	double fRec5[2];
	double fVec2[2];
	double fRec6[2];
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
		fConst3 = (2.1775860903036022 / fConst0);
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fVslider0 = FAUSTFLOAT(0.0);
		fHslider0 = FAUSTFLOAT(440.0);
		fVslider1 = FAUSTFLOAT(1.0);
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
			fRec3[l2] = 0.0;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec4[l3] = 0.0;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fVec1[l4] = 0.0;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec5[l5] = 0.0;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fVec2[l6] = 0.0;
		}
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			fRec6[l7] = 0.0;
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec1[l8] = 0.0;
		}
		for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
			fRec0[l9] = 0.0;
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
		double fSlow1 = std::pow(10.0, (0.025000000000000001 * std::min<double>(60.0, std::max<double>(-120.0, double(fVslider0)))));
		double fSlow2 = std::min<double>(20000.0, std::max<double>(1.0, double(fHslider0)));
		double fSlow3 = (fConst2 * std::max<double>(0.0, fSlow2));
		double fSlow4 = std::cos(fSlow3);
		double fSlow5 = ((fSlow1 + 1.0) * fSlow4);
		double fSlow6 = ((fSlow1 + -1.0) * fSlow4);
		double fSlow7 = ((std::sqrt(fSlow1) * std::sin(fSlow3)) / std::max<double>(0.001, (0.5 / double(sinh(double((fConst3 * ((fSlow2 * std::min<double>(12.0, std::max<double>(0.01, double(fVslider1)))) / std::sin((fConst2 * fSlow2))))))))));
		double fSlow8 = (fSlow6 + fSlow7);
		double fSlow9 = ((fSlow1 + fSlow8) + 1.0);
		double fSlow10 = (1.0 - fSlow0);
		double fSlow11 = ((2.0 * ((fSlow1 * (fSlow1 + (-1.0 - fSlow5))) / fSlow9)) * fSlow10);
		double fSlow12 = (((fSlow1 * ((fSlow1 + fSlow7) + (1.0 - fSlow6))) / fSlow9) * fSlow10);
		double fSlow13 = (((fSlow1 * (fSlow1 + (1.0 - fSlow8))) / fSlow9) * fSlow10);
		double fSlow14 = ((((fSlow1 + fSlow6) + (1.0 - fSlow7)) / fSlow9) * fSlow10);
		double fSlow15 = (((0.0 - (2.0 * ((fSlow1 + fSlow5) + -1.0))) / fSlow9) * fSlow10);
		for (int i0 = 0; (i0 < count); i0 = (i0 + 1)) {
			double fTemp0 = double(input0[i0]);
			fRec2[0] = ((fSlow0 * fRec2[1]) + fSlow11);
			fVec0[0] = (fTemp0 * fRec2[0]);
			fRec3[0] = ((fSlow0 * fRec3[1]) + fSlow12);
			fRec4[0] = ((fSlow0 * fRec4[1]) + fSlow13);
			fVec1[0] = (fTemp0 * fRec4[0]);
			fRec5[0] = ((fSlow0 * fRec5[1]) + fSlow14);
			fVec2[0] = (fVec1[1] - (fRec5[0] * fRec0[1]));
			fRec6[0] = ((fSlow0 * fRec6[1]) + fSlow15);
			fRec1[0] = ((fVec0[1] + ((fTemp0 * fRec3[0]) + fVec2[1])) - (fRec6[0] * fRec1[1]));
			fRec0[0] = fRec1[0];
			output0[i0] = FAUSTFLOAT(fRec0[0]);
			fRec2[1] = fRec2[0];
			fVec0[1] = fVec0[0];
			fRec3[1] = fRec3[0];
			fRec4[1] = fRec4[0];
			fVec1[1] = fVec1[0];
			fRec5[1] = fRec5[0];
			fVec2[1] = fVec2[0];
			fRec6[1] = fRec6[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getCutoff() const { return fHslider0; }
    void setCutoff(FAUSTFLOAT value) { fHslider0 = value; }

    FAUSTFLOAT getPeakShelfGain() const { return fVslider0; }
    void setPeakShelfGain(FAUSTFLOAT value) { fVslider0 = value; }

    FAUSTFLOAT getBandwidth() const { return fVslider1; }
    void setBandwidth(FAUSTFLOAT value) { fVslider1 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
