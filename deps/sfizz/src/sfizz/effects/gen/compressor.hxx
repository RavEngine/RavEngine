#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
name: "compressor"
Code generated with Faust 2.30.5 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __faustCompressor_H__
#define  __faustCompressor_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faustCompressor
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faustCompressor {
	//[Begin:class]

	
 private:
	
	FAUSTFLOAT fHslider0;
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider1;
	FAUSTFLOAT fHslider2;
	float fRec2[2];
	float fRec1[2];
	FAUSTFLOAT fHslider3;
	float fRec0[2];
	
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
		fConst0 = (1.0f / float(fSampleRate));
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fHslider0 = FAUSTFLOAT(0.0f);
		fHslider1 = FAUSTFLOAT(1.0f);
		fHslider2 = FAUSTFLOAT(0.0f);
		fHslider3 = FAUSTFLOAT(0.0f);
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
		float fSlow0 = float(fHslider0);
		float fSlow1 = (0.5f * fSlow0);
		int iSlow2 = (std::fabs(fSlow1) < 1.1920929e-07f);
		float fSlow3 = (iSlow2 ? 0.0f : std::exp((0.0f - (fConst0 / (iSlow2 ? 1.0f : fSlow1)))));
		float fSlow4 = ((1.0f / std::max<float>(1.1920929e-07f, float(fHslider1))) + -1.0f);
		int iSlow5 = (std::fabs(fSlow0) < 1.1920929e-07f);
		float fSlow6 = (iSlow5 ? 0.0f : std::exp((0.0f - (fConst0 / (iSlow5 ? 1.0f : fSlow0)))));
		float fSlow7 = float(fHslider2);
		int iSlow8 = (std::fabs(fSlow7) < 1.1920929e-07f);
		float fSlow9 = (iSlow8 ? 0.0f : std::exp((0.0f - (fConst0 / (iSlow8 ? 1.0f : fSlow7)))));
		float fSlow10 = float(fHslider3);
		float fSlow11 = (1.0f - fSlow3);
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			float fTemp1 = std::fabs(fTemp0);
			float fTemp2 = ((fRec1[1] > fTemp1) ? fSlow9 : fSlow6);
			fRec2[0] = ((fRec2[1] * fTemp2) + (fTemp1 * (1.0f - fTemp2)));
			fRec1[0] = fRec2[0];
			fRec0[0] = ((fSlow3 * fRec0[1]) + (fSlow4 * (std::max<float>(((20.0f * std::log10(fRec1[0])) - fSlow10), 0.0f) * fSlow11)));
			output0[i] = FAUSTFLOAT(std::pow(10.0f, (0.0500000007f * fRec0[0])));
			fRec2[1] = fRec2[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getRatio() const { return fHslider1; }
    void setRatio(FAUSTFLOAT value) { fHslider1 = value; }

    FAUSTFLOAT getThreshold() const { return fHslider3; }
    void setThreshold(FAUSTFLOAT value) { fHslider3 = value; }

    FAUSTFLOAT getAttack() const { return fHslider0; }
    void setAttack(FAUSTFLOAT value) { fHslider0 = value; }

    FAUSTFLOAT getRelease() const { return fHslider2; }
    void setRelease(FAUSTFLOAT value) { fHslider2 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
