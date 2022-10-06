#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
name: "disto_stage"
Code generated with Faust 2.30.5 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __faustDisto_H__
#define  __faustDisto_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


//[Before:class]
class faustDistoSIG0 {
	//[Begin:class]

	
  private:
	
	int iRec3[2];
	
  public:
	
	int getNumInputsfaustDistoSIG0() {
		return 0;
	}
	int getNumOutputsfaustDistoSIG0() {
		return 1;
	}
	int getInputRatefaustDistoSIG0(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRatefaustDistoSIG0(int channel) {
		int rate;
		switch ((channel)) {
			case 0: {
				rate = 0;
				break;
			}
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	
	void instanceInitfaustDistoSIG0(int sample_rate) {
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			iRec3[l3] = 0;
		}
	}
	
	void fillfaustDistoSIG0(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			iRec3[0] = (iRec3[1] + 1);
			float fTemp1 = std::exp(((0.078125f * float((iRec3[0] + -1))) + -10.0f));
			table[i] = (fTemp1 / (fTemp1 + 1.0f));
			iRec3[1] = iRec3[0];
		}
	}

};

static faustDistoSIG0* newfaustDistoSIG0() { return (faustDistoSIG0*)new faustDistoSIG0(); }
static void deletefaustDistoSIG0(faustDistoSIG0* dsp) { delete dsp; }

static float ftbl0faustDistoSIG0[256];

#ifndef FAUSTCLASS 
#define FAUSTCLASS faustDisto
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class faustDisto {
	
 private:
	
	float fVec0[2];
	int fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fConst3;
	float fConst4;
	float fConst5;
	int iRec2[2];
	float fRec1[2];
	FAUSTFLOAT fHslider0;
	float fVec1[2];
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
		faustDistoSIG0* sig0 = newfaustDistoSIG0();
		sig0->instanceInitfaustDistoSIG0(sample_rate);
		sig0->fillfaustDistoSIG0(256, ftbl0faustDistoSIG0);
		deletefaustDistoSIG0(sig0);
		//[End:classInit]
	}
	
	void instanceConstants(int sample_rate) {
		//[Begin:instanceConstants]
		fSampleRate = sample_rate;
		fConst0 = float(fSampleRate);
		fConst1 = (125.663704f / fConst0);
		fConst2 = (1.0f / (fConst1 + 1.0f));
		fConst3 = (1.0f - fConst1);
		fConst4 = std::exp((0.0f - (100.0f / fConst0)));
		fConst5 = (1.0f - fConst4);
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fHslider0 = FAUSTFLOAT(100.0f);
		//[End:instanceResetUserInterface]
	}
	
	void instanceClear() {
		//[Begin:instanceClear]
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fVec0[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			iRec2[l1] = 0;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec1[l2] = 0.0f;
		}
		for (int l4 = 0; (l4 < 2); l4 = (l4 + 1)) {
			fVec1[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec0[l5] = 0.0f;
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
		float fSlow0 = ((0.200000003f * float(fHslider0)) + 2.0f);
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			fVec0[0] = fTemp0;
			iRec2[0] = (((fTemp0 < fVec0[1]) & (fTemp0 < -0.25f)) ? 1 : (((fTemp0 > fVec0[1]) & (fTemp0 > 0.25f)) ? 0 : iRec2[1]));
			fRec1[0] = ((fConst4 * fRec1[1]) + (fConst5 * float(iRec2[0])));
			float fTemp2 = std::max<float>(0.0f, (12.75f * ((fSlow0 * fTemp0) + 10.0f)));
			int iTemp3 = int(fTemp2);
			float fTemp4 = ftbl0faustDistoSIG0[std::min<int>(255, iTemp3)];
			float fTemp5 = (fTemp4 + ((fTemp2 - float(iTemp3)) * (ftbl0faustDistoSIG0[std::min<int>(255, (iTemp3 + 1))] - fTemp4)));
			float fTemp6 = ((fRec1[0] * (fTemp5 + -1.0f)) + ((1.0f - fRec1[0]) * fTemp5));
			fVec1[0] = fTemp6;
			fRec0[0] = (fConst2 * ((fConst3 * fRec0[1]) + (2.0f * (fTemp6 - fVec1[1]))));
			output0[i] = FAUSTFLOAT(fRec0[0]);
			fVec0[1] = fVec0[0];
			iRec2[1] = iRec2[0];
			fRec1[1] = fRec1[0];
			fVec1[1] = fVec1[0];
			fRec0[1] = fRec0[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getDepth() const { return fHslider0; }
    void setDepth(FAUSTFLOAT value) { fHslider0 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
