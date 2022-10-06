#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
name: "gate"
Code generated with Faust 2.30.5 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __faustGate_H__
#define  __faustGate_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


#ifndef FAUSTCLASS 
#define FAUSTCLASS faustGate
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif


//[Before:class]
class faustGate {
	//[Begin:class]

	
 private:
	
	FAUSTFLOAT fHslider0;
	FAUSTFLOAT fHslider1;
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fRec3[2];
	FAUSTFLOAT fHslider2;
	int iVec0[2];
	FAUSTFLOAT fHslider3;
	int iRec4[2];
	float fRec1[2];
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
		fConst0 = float(fSampleRate);
		fConst1 = (1.0f / fConst0);
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fHslider0 = FAUSTFLOAT(0.0f);
		fHslider1 = FAUSTFLOAT(0.0f);
		fHslider2 = FAUSTFLOAT(0.0f);
		fHslider3 = FAUSTFLOAT(0.0f);
		//[End:instanceResetUserInterface]
	}
	
	void instanceClear() {
		//[Begin:instanceClear]
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec3[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			iVec0[l1] = 0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			iRec4[l2] = 0;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec1[l3] = 0.0f;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fRec0[l4] = 0.0f;
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
		float fSlow1 = float(fHslider1);
		float fSlow2 = std::min<float>(fSlow0, fSlow1);
		int iSlow3 = (std::fabs(fSlow2) < 1.1920929e-07f);
		float fSlow4 = (iSlow3 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow3 ? 1.0f : fSlow2)))));
		float fSlow5 = (1.0f - fSlow4);
		float fSlow6 = std::pow(10.0f, (0.0500000007f * float(fHslider2)));
		int iSlow7 = int((fConst0 * float(fHslider3)));
		int iSlow8 = (std::fabs(fSlow0) < 1.1920929e-07f);
		float fSlow9 = (iSlow8 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow8 ? 1.0f : fSlow0)))));
		int iSlow10 = (std::fabs(fSlow1) < 1.1920929e-07f);
		float fSlow11 = (iSlow10 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow10 ? 1.0f : fSlow1)))));
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			fRec3[0] = ((fRec3[1] * fSlow4) + (std::fabs(fTemp0) * fSlow5));
			float fRec2 = fRec3[0];
			int iTemp1 = (fRec2 > fSlow6);
			iVec0[0] = iTemp1;
			iRec4[0] = std::max<int>(int((iSlow7 * (iTemp1 < iVec0[1]))), int((iRec4[1] + -1)));
			float fTemp2 = std::fabs(std::max<float>(float(iTemp1), float((iRec4[0] > 0))));
			float fTemp3 = ((fRec0[1] > fTemp2) ? fSlow11 : fSlow9);
			fRec1[0] = ((fRec1[1] * fTemp3) + (fTemp2 * (1.0f - fTemp3)));
			fRec0[0] = fRec1[0];
			output0[i] = FAUSTFLOAT(fRec0[0]);
			fRec3[1] = fRec3[0];
			iVec0[1] = iVec0[0];
			iRec4[1] = iRec4[0];
			fRec1[1] = fRec1[0];
			fRec0[1] = fRec0[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getThreshold() const { return fHslider2; }
    void setThreshold(FAUSTFLOAT value) { fHslider2 = value; }

    FAUSTFLOAT getAttack() const { return fHslider0; }
    void setAttack(FAUSTFLOAT value) { fHslider0 = value; }

    FAUSTFLOAT getHold() const { return fHslider3; }
    void setHold(FAUSTFLOAT value) { fHslider3 = value; }

    FAUSTFLOAT getRelease() const { return fHslider1; }
    void setRelease(FAUSTFLOAT value) { fHslider1 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
