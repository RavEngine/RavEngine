#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
/* ------------------------------------------------------------
author: "Jean Pierre Cimalando"
license: "BSD-2-Clause"
name: "fverb"
version: "0.5"
Code generated with Faust 2.30.5 (https://faust.grame.fr)
Compilation options: -lang cpp -inpl -es 1 -scal -ftz 0
------------------------------------------------------------ */

#ifndef  __faustFverb_H__
#define  __faustFverb_H__

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif 

#include <algorithm>
#include <cmath>
#include <math.h>


//[Before:class]
class faustFverbSIG0 {
	//[Begin:class]

	
  private:
	
	int iRec36[2];
	
  public:
	
	int getNumInputsfaustFverbSIG0() {
		return 0;
	}
	int getNumOutputsfaustFverbSIG0() {
		return 1;
	}
	int getInputRatefaustFverbSIG0(int channel) {
		int rate;
		switch ((channel)) {
			default: {
				rate = -1;
				break;
			}
		}
		return rate;
	}
	int getOutputRatefaustFverbSIG0(int channel) {
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
	
	void instanceInitfaustFverbSIG0(int sample_rate) {
		for (int l23 = 0; (l23 < 2); l23 = (l23 + 1)) {
			iRec36[l23] = 0;
		}
	}
	
	void fillfaustFverbSIG0(int count, float* table) {
		for (int i = 0; (i < count); i = (i + 1)) {
			iRec36[0] = (iRec36[1] + 1);
			table[i] = std::sin((9.58738019e-05f * float((iRec36[0] + -1))));
			iRec36[1] = iRec36[0];
		}
	}

};

static faustFverbSIG0* newfaustFverbSIG0() { return (faustFverbSIG0*)new faustFverbSIG0(); }
static void deletefaustFverbSIG0(faustFverbSIG0* dsp) { delete dsp; }

static float ftbl0faustFverbSIG0[65536];

#ifndef FAUSTCLASS 
#define FAUSTCLASS faustFverb
#endif

#ifdef __APPLE__ 
#define exp10f __exp10f
#define exp10 __exp10
#endif

class faustFverb {
	
 private:
	
	FAUSTFLOAT fHslider0;
	float fRec0[2];
	FAUSTFLOAT fHslider1;
	float fRec1[2];
	FAUSTFLOAT fHslider2;
	float fRec10[2];
	FAUSTFLOAT fHslider3;
	float fRec24[2];
	int IOTA;
	float fVec0[131072];
	int fSampleRate;
	float fConst0;
	FAUSTFLOAT fHslider4;
	float fRec25[2];
	float fConst1;
	FAUSTFLOAT fHslider5;
	float fRec26[2];
	float fRec23[2];
	FAUSTFLOAT fHslider6;
	float fRec27[2];
	float fRec22[2];
	FAUSTFLOAT fHslider7;
	float fRec28[2];
	float fVec1[131072];
	int iConst2;
	float fRec20[2];
	float fVec2[131072];
	int iConst3;
	float fRec18[2];
	FAUSTFLOAT fHslider8;
	float fRec29[2];
	float fVec3[131072];
	int iConst4;
	float fRec16[2];
	float fVec4[131072];
	int iConst5;
	float fRec14[2];
	int iConst6;
	FAUSTFLOAT fHslider9;
	float fRec30[2];
	float fVec5[131072];
	FAUSTFLOAT fHslider10;
	float fRec35[2];
	FAUSTFLOAT fHslider11;
	float fRec38[2];
	float fRec37[2];
	float fConst7;
	float fConst8;
	float fRec31[2];
	float fRec32[2];
	int iRec33[2];
	int iRec34[2];
	float fRec12[2];
	float fVec6[131072];
	int iConst9;
	FAUSTFLOAT fHslider12;
	float fRec39[2];
	float fRec11[2];
	float fVec7[131072];
	int iConst10;
	float fRec8[2];
	float fRec2[131072];
	float fRec3[131072];
	float fRec4[131072];
	float fVec8[131072];
	float fRec54[2];
	float fRec53[2];
	float fVec9[131072];
	int iConst11;
	float fRec51[2];
	float fVec10[131072];
	int iConst12;
	float fRec49[2];
	float fVec11[131072];
	int iConst13;
	float fRec47[2];
	float fVec12[131072];
	int iConst14;
	float fRec45[2];
	int iConst15;
	float fVec13[131072];
	float fRec55[2];
	float fRec56[2];
	int iRec57[2];
	int iRec58[2];
	float fRec43[2];
	float fVec14[131072];
	int iConst16;
	float fRec42[2];
	float fVec15[131072];
	int iConst17;
	float fRec40[2];
	float fRec5[131072];
	float fRec6[131072];
	float fRec7[131072];
	int iConst18;
	int iConst19;
	int iConst20;
	int iConst21;
	int iConst22;
	int iConst23;
	int iConst24;
	int iConst25;
	int iConst26;
	int iConst27;
	int iConst28;
	int iConst29;
	int iConst30;
	int iConst31;
	
 public:
	

	static constexpr int getNumInputs() {
		return 2;
	}
	static constexpr int getNumOutputs() {
		return 2;
	}
	
	static void classInit(int sample_rate) {
		//[Begin:classInit]
		faustFverbSIG0* sig0 = newfaustFverbSIG0();
		sig0->instanceInitfaustFverbSIG0(sample_rate);
		sig0->fillfaustFverbSIG0(65536, ftbl0faustFverbSIG0);
		deletefaustFverbSIG0(sig0);
		//[End:classInit]
	}
	
	void instanceConstants(int sample_rate) {
		//[Begin:instanceConstants]
		fSampleRate = sample_rate;
		fConst0 = float(fSampleRate);
		fConst1 = (1.0f / fConst0);
		iConst2 = std::min<int>(65536, std::max<int>(0, (int((0.00462820474f * fConst0)) + -1)));
		iConst3 = std::min<int>(65536, std::max<int>(0, (int((0.00370316859f * fConst0)) + -1)));
		iConst4 = std::min<int>(65536, std::max<int>(0, (int((0.013116831f * fConst0)) + -1)));
		iConst5 = std::min<int>(65536, std::max<int>(0, (int((0.00902825873f * fConst0)) + -1)));
		iConst6 = (std::min<int>(65536, std::max<int>(0, int((0.106280029f * fConst0)))) + 1);
		fConst7 = (1.0f / float(int((0.00999999978f * fConst0))));
		fConst8 = (0.0f - fConst7);
		iConst9 = std::min<int>(65536, std::max<int>(0, int((0.141695514f * fConst0))));
		iConst10 = std::min<int>(65536, std::max<int>(0, (int((0.0892443135f * fConst0)) + -1)));
		iConst11 = std::min<int>(65536, std::max<int>(0, (int((0.00491448538f * fConst0)) + -1)));
		iConst12 = std::min<int>(65536, std::max<int>(0, (int((0.00348745007f * fConst0)) + -1)));
		iConst13 = std::min<int>(65536, std::max<int>(0, (int((0.0123527432f * fConst0)) + -1)));
		iConst14 = std::min<int>(65536, std::max<int>(0, (int((0.00958670769f * fConst0)) + -1)));
		iConst15 = (std::min<int>(65536, std::max<int>(0, int((0.124995798f * fConst0)))) + 1);
		iConst16 = std::min<int>(65536, std::max<int>(0, int((0.149625346f * fConst0))));
		iConst17 = std::min<int>(65536, std::max<int>(0, (int((0.0604818389f * fConst0)) + -1)));
		iConst18 = std::min<int>(65536, std::max<int>(0, int((0.00893787201f * fConst0))));
		iConst19 = std::min<int>(65536, std::max<int>(0, int((0.099929437f * fConst0))));
		iConst20 = std::min<int>(65536, std::max<int>(0, int((0.067067638f * fConst0))));
		iConst21 = std::min<int>(65536, std::max<int>(0, int((0.0642787516f * fConst0))));
		iConst22 = std::min<int>(65536, std::max<int>(0, int((0.0668660328f * fConst0))));
		iConst23 = std::min<int>(65536, std::max<int>(0, int((0.0062833908f * fConst0))));
		iConst24 = std::min<int>(65536, std::max<int>(0, int((0.0358186886f * fConst0))));
		iConst25 = std::min<int>(65536, std::max<int>(0, int((0.0118611604f * fConst0))));
		iConst26 = std::min<int>(65536, std::max<int>(0, int((0.121870905f * fConst0))));
		iConst27 = std::min<int>(65536, std::max<int>(0, int((0.0898155272f * fConst0))));
		iConst28 = std::min<int>(65536, std::max<int>(0, int((0.041262053f * fConst0))));
		iConst29 = std::min<int>(65536, std::max<int>(0, int((0.070931755f * fConst0))));
		iConst30 = std::min<int>(65536, std::max<int>(0, int((0.0112563418f * fConst0))));
		iConst31 = std::min<int>(65536, std::max<int>(0, int((0.00406572362f * fConst0))));
		//[End:instanceConstants]
	}
	
	void instanceResetUserInterface() {
		//[Begin:instanceResetUserInterface]
		fHslider0 = FAUSTFLOAT(100.0f);
		fHslider1 = FAUSTFLOAT(50.0f);
		fHslider2 = FAUSTFLOAT(50.0f);
		fHslider3 = FAUSTFLOAT(100.0f);
		fHslider4 = FAUSTFLOAT(0.0f);
		fHslider5 = FAUSTFLOAT(10000.0f);
		fHslider6 = FAUSTFLOAT(100.0f);
		fHslider7 = FAUSTFLOAT(75.0f);
		fHslider8 = FAUSTFLOAT(62.5f);
		fHslider9 = FAUSTFLOAT(70.0f);
		fHslider10 = FAUSTFLOAT(0.5f);
		fHslider11 = FAUSTFLOAT(1.0f);
		fHslider12 = FAUSTFLOAT(5500.0f);
		//[End:instanceResetUserInterface]
	}
	
	void instanceClear() {
		//[Begin:instanceClear]
		for (int l0 = 0; (l0 < 2); l0 = (l0 + 1)) {
			fRec0[l0] = 0.0f;
		}
		for (int l1 = 0; (l1 < 2); l1 = (l1 + 1)) {
			fRec1[l1] = 0.0f;
		}
		for (int l2 = 0; (l2 < 2); l2 = (l2 + 1)) {
			fRec10[l2] = 0.0f;
		}
		for (int l3 = 0; (l3 < 2); l3 = (l3 + 1)) {
			fRec24[l3] = 0.0f;
		}
		IOTA = 0;
		for (int l4 = 0; (l4 < 131072); l4 = (l4 + 1)) {
			fVec0[l4] = 0.0f;
		}
		for (int l5 = 0; (l5 < 2); l5 = (l5 + 1)) {
			fRec25[l5] = 0.0f;
		}
		for (int l6 = 0; (l6 < 2); l6 = (l6 + 1)) {
			fRec26[l6] = 0.0f;
		}
		for (int l7 = 0; (l7 < 2); l7 = (l7 + 1)) {
			fRec23[l7] = 0.0f;
		}
		for (int l8 = 0; (l8 < 2); l8 = (l8 + 1)) {
			fRec27[l8] = 0.0f;
		}
		for (int l9 = 0; (l9 < 2); l9 = (l9 + 1)) {
			fRec22[l9] = 0.0f;
		}
		for (int l10 = 0; (l10 < 2); l10 = (l10 + 1)) {
			fRec28[l10] = 0.0f;
		}
		for (int l11 = 0; (l11 < 131072); l11 = (l11 + 1)) {
			fVec1[l11] = 0.0f;
		}
		for (int l12 = 0; (l12 < 2); l12 = (l12 + 1)) {
			fRec20[l12] = 0.0f;
		}
		for (int l13 = 0; (l13 < 131072); l13 = (l13 + 1)) {
			fVec2[l13] = 0.0f;
		}
		for (int l14 = 0; (l14 < 2); l14 = (l14 + 1)) {
			fRec18[l14] = 0.0f;
		}
		for (int l15 = 0; (l15 < 2); l15 = (l15 + 1)) {
			fRec29[l15] = 0.0f;
		}
		for (int l16 = 0; (l16 < 131072); l16 = (l16 + 1)) {
			fVec3[l16] = 0.0f;
		}
		for (int l17 = 0; (l17 < 2); l17 = (l17 + 1)) {
			fRec16[l17] = 0.0f;
		}
		for (int l18 = 0; (l18 < 131072); l18 = (l18 + 1)) {
			fVec4[l18] = 0.0f;
		}
		for (int l19 = 0; (l19 < 2); l19 = (l19 + 1)) {
			fRec14[l19] = 0.0f;
		}
		for (int l20 = 0; (l20 < 2); l20 = (l20 + 1)) {
			fRec30[l20] = 0.0f;
		}
		for (int l21 = 0; (l21 < 131072); l21 = (l21 + 1)) {
			fVec5[l21] = 0.0f;
		}
		for (int l22 = 0; (l22 < 2); l22 = (l22 + 1)) {
			fRec35[l22] = 0.0f;
		}
		for (int l24 = 0; (l24 < 2); l24 = (l24 + 1)) {
			fRec38[l24] = 0.0f;
		}
		for (int l25 = 0; (l25 < 2); l25 = (l25 + 1)) {
			fRec37[l25] = 0.0f;
		}
		for (int l26 = 0; (l26 < 2); l26 = (l26 + 1)) {
			fRec31[l26] = 0.0f;
		}
		for (int l27 = 0; (l27 < 2); l27 = (l27 + 1)) {
			fRec32[l27] = 0.0f;
		}
		for (int l28 = 0; (l28 < 2); l28 = (l28 + 1)) {
			iRec33[l28] = 0;
		}
		for (int l29 = 0; (l29 < 2); l29 = (l29 + 1)) {
			iRec34[l29] = 0;
		}
		for (int l30 = 0; (l30 < 2); l30 = (l30 + 1)) {
			fRec12[l30] = 0.0f;
		}
		for (int l31 = 0; (l31 < 131072); l31 = (l31 + 1)) {
			fVec6[l31] = 0.0f;
		}
		for (int l32 = 0; (l32 < 2); l32 = (l32 + 1)) {
			fRec39[l32] = 0.0f;
		}
		for (int l33 = 0; (l33 < 2); l33 = (l33 + 1)) {
			fRec11[l33] = 0.0f;
		}
		for (int l34 = 0; (l34 < 131072); l34 = (l34 + 1)) {
			fVec7[l34] = 0.0f;
		}
		for (int l35 = 0; (l35 < 2); l35 = (l35 + 1)) {
			fRec8[l35] = 0.0f;
		}
		for (int l36 = 0; (l36 < 131072); l36 = (l36 + 1)) {
			fRec2[l36] = 0.0f;
		}
		for (int l37 = 0; (l37 < 131072); l37 = (l37 + 1)) {
			fRec3[l37] = 0.0f;
		}
		for (int l38 = 0; (l38 < 131072); l38 = (l38 + 1)) {
			fRec4[l38] = 0.0f;
		}
		for (int l39 = 0; (l39 < 131072); l39 = (l39 + 1)) {
			fVec8[l39] = 0.0f;
		}
		for (int l40 = 0; (l40 < 2); l40 = (l40 + 1)) {
			fRec54[l40] = 0.0f;
		}
		for (int l41 = 0; (l41 < 2); l41 = (l41 + 1)) {
			fRec53[l41] = 0.0f;
		}
		for (int l42 = 0; (l42 < 131072); l42 = (l42 + 1)) {
			fVec9[l42] = 0.0f;
		}
		for (int l43 = 0; (l43 < 2); l43 = (l43 + 1)) {
			fRec51[l43] = 0.0f;
		}
		for (int l44 = 0; (l44 < 131072); l44 = (l44 + 1)) {
			fVec10[l44] = 0.0f;
		}
		for (int l45 = 0; (l45 < 2); l45 = (l45 + 1)) {
			fRec49[l45] = 0.0f;
		}
		for (int l46 = 0; (l46 < 131072); l46 = (l46 + 1)) {
			fVec11[l46] = 0.0f;
		}
		for (int l47 = 0; (l47 < 2); l47 = (l47 + 1)) {
			fRec47[l47] = 0.0f;
		}
		for (int l48 = 0; (l48 < 131072); l48 = (l48 + 1)) {
			fVec12[l48] = 0.0f;
		}
		for (int l49 = 0; (l49 < 2); l49 = (l49 + 1)) {
			fRec45[l49] = 0.0f;
		}
		for (int l50 = 0; (l50 < 131072); l50 = (l50 + 1)) {
			fVec13[l50] = 0.0f;
		}
		for (int l51 = 0; (l51 < 2); l51 = (l51 + 1)) {
			fRec55[l51] = 0.0f;
		}
		for (int l52 = 0; (l52 < 2); l52 = (l52 + 1)) {
			fRec56[l52] = 0.0f;
		}
		for (int l53 = 0; (l53 < 2); l53 = (l53 + 1)) {
			iRec57[l53] = 0;
		}
		for (int l54 = 0; (l54 < 2); l54 = (l54 + 1)) {
			iRec58[l54] = 0;
		}
		for (int l55 = 0; (l55 < 2); l55 = (l55 + 1)) {
			fRec43[l55] = 0.0f;
		}
		for (int l56 = 0; (l56 < 131072); l56 = (l56 + 1)) {
			fVec14[l56] = 0.0f;
		}
		for (int l57 = 0; (l57 < 2); l57 = (l57 + 1)) {
			fRec42[l57] = 0.0f;
		}
		for (int l58 = 0; (l58 < 131072); l58 = (l58 + 1)) {
			fVec15[l58] = 0.0f;
		}
		for (int l59 = 0; (l59 < 2); l59 = (l59 + 1)) {
			fRec40[l59] = 0.0f;
		}
		for (int l60 = 0; (l60 < 131072); l60 = (l60 + 1)) {
			fRec5[l60] = 0.0f;
		}
		for (int l61 = 0; (l61 < 131072); l61 = (l61 + 1)) {
			fRec6[l61] = 0.0f;
		}
		for (int l62 = 0; (l62 < 131072); l62 = (l62 + 1)) {
			fRec7[l62] = 0.0f;
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
		float fSlow0 = (9.99999975e-06f * float(fHslider0));
		float fSlow1 = (9.99999975e-06f * float(fHslider1));
		float fSlow2 = (9.99999975e-06f * float(fHslider2));
		float fSlow3 = (9.99999975e-06f * float(fHslider3));
		float fSlow4 = (9.99999997e-07f * float(fHslider4));
		float fSlow5 = (0.00100000005f * std::exp((fConst1 * (0.0f - (6.28318548f * float(fHslider5))))));
		float fSlow6 = (0.00100000005f * std::exp((fConst1 * (0.0f - (6.28318548f * float(fHslider6))))));
		float fSlow7 = (9.99999975e-06f * float(fHslider7));
		float fSlow8 = (9.99999975e-06f * float(fHslider8));
		float fSlow9 = (9.99999975e-06f * float(fHslider9));
		float fSlow10 = (9.99999997e-07f * float(fHslider10));
		float fSlow11 = (0.00100000005f * float(fHslider11));
		float fSlow12 = (0.00100000005f * std::exp((fConst1 * (0.0f - (6.28318548f * float(fHslider12))))));
		for (int i = 0; (i < count); i = (i + 1)) {
			float fTemp0 = float(input0[i]);
			float fTemp1 = float(input1[i]);
			fRec0[0] = (fSlow0 + (0.999000013f * fRec0[1]));
			fRec1[0] = (fSlow1 + (0.999000013f * fRec1[1]));
			fRec10[0] = (fSlow2 + (0.999000013f * fRec10[1]));
			float fTemp2 = std::min<float>(0.5f, std::max<float>(0.25f, (fRec10[0] + 0.150000006f)));
			fRec24[0] = (fSlow3 + (0.999000013f * fRec24[1]));
			fVec0[(IOTA & 131071)] = (fTemp1 * fRec24[0]);
			fRec25[0] = (fSlow4 + (0.999000013f * fRec25[1]));
			int iTemp3 = std::min<int>(65536, std::max<int>(0, int((fConst0 * fRec25[0]))));
			fRec26[0] = (fSlow5 + (0.999000013f * fRec26[1]));
			fRec23[0] = (fVec0[((IOTA - iTemp3) & 131071)] + (fRec26[0] * fRec23[1]));
			float fTemp4 = (1.0f - fRec26[0]);
			fRec27[0] = (fSlow6 + (0.999000013f * fRec27[1]));
			fRec22[0] = ((fRec23[0] * fTemp4) + (fRec27[0] * fRec22[1]));
			float fTemp5 = (fRec27[0] + 1.0f);
			float fTemp6 = (0.0f - (0.5f * fTemp5));
			fRec28[0] = (fSlow7 + (0.999000013f * fRec28[1]));
			float fTemp7 = (((0.5f * (fRec22[0] * fTemp5)) + (fRec22[1] * fTemp6)) - (fRec28[0] * fRec20[1]));
			fVec1[(IOTA & 131071)] = fTemp7;
			fRec20[0] = fVec1[((IOTA - iConst2) & 131071)];
			float fRec21 = (fRec28[0] * fTemp7);
			float fTemp8 = ((fRec21 + fRec20[1]) - (fRec28[0] * fRec18[1]));
			fVec2[(IOTA & 131071)] = fTemp8;
			fRec18[0] = fVec2[((IOTA - iConst3) & 131071)];
			float fRec19 = (fRec28[0] * fTemp8);
			fRec29[0] = (fSlow8 + (0.999000013f * fRec29[1]));
			float fTemp9 = ((fRec19 + fRec18[1]) - (fRec29[0] * fRec16[1]));
			fVec3[(IOTA & 131071)] = fTemp9;
			fRec16[0] = fVec3[((IOTA - iConst4) & 131071)];
			float fRec17 = (fRec29[0] * fTemp9);
			float fTemp10 = ((fRec17 + fRec16[1]) - (fRec29[0] * fRec14[1]));
			fVec4[(IOTA & 131071)] = fTemp10;
			fRec14[0] = fVec4[((IOTA - iConst5) & 131071)];
			float fRec15 = (fRec29[0] * fTemp10);
			fRec30[0] = (fSlow9 + (0.999000013f * fRec30[1]));
			float fTemp11 = (fRec14[1] + ((fRec10[0] * fRec5[((IOTA - iConst6) & 131071)]) + (fRec15 + (fRec30[0] * fRec12[1]))));
			fVec5[(IOTA & 131071)] = fTemp11;
			fRec35[0] = (fSlow10 + (0.999000013f * fRec35[1]));
			fRec38[0] = (fSlow11 + (0.999000013f * fRec38[1]));
			float fTemp12 = (fRec37[1] + (fConst1 * fRec38[0]));
			fRec37[0] = (fTemp12 - float(int(fTemp12)));
			int iTemp13 = (int((fConst0 * ((fRec35[0] * ftbl0faustFverbSIG0[int((65536.0f * (fRec37[0] + (0.25f - float(int((fRec37[0] + 0.25f)))))))]) + 0.0305097271f))) + -1);
			float fTemp14 = ((fRec31[1] != 0.0f) ? (((fRec32[1] > 0.0f) & (fRec32[1] < 1.0f)) ? fRec31[1] : 0.0f) : (((fRec32[1] == 0.0f) & (iTemp13 != iRec33[1])) ? fConst7 : (((fRec32[1] == 1.0f) & (iTemp13 != iRec34[1])) ? fConst8 : 0.0f)));
			fRec31[0] = fTemp14;
			fRec32[0] = std::max<float>(0.0f, std::min<float>(1.0f, (fRec32[1] + fTemp14)));
			iRec33[0] = (((fRec32[1] >= 1.0f) & (iRec34[1] != iTemp13)) ? iTemp13 : iRec33[1]);
			iRec34[0] = (((fRec32[1] <= 0.0f) & (iRec33[1] != iTemp13)) ? iTemp13 : iRec34[1]);
			float fTemp15 = fVec5[((IOTA - std::min<int>(65536, std::max<int>(0, iRec33[0]))) & 131071)];
			fRec12[0] = (fTemp15 + (fRec32[0] * (fVec5[((IOTA - std::min<int>(65536, std::max<int>(0, iRec34[0]))) & 131071)] - fTemp15)));
			float fRec13 = (0.0f - (fRec30[0] * fTemp11));
			float fTemp16 = (fRec13 + fRec12[1]);
			fVec6[(IOTA & 131071)] = fTemp16;
			fRec39[0] = (fSlow12 + (0.999000013f * fRec39[1]));
			fRec11[0] = (fVec6[((IOTA - iConst9) & 131071)] + (fRec39[0] * fRec11[1]));
			float fTemp17 = (1.0f - fRec39[0]);
			float fTemp18 = ((fTemp2 * fRec8[1]) + ((fRec10[0] * fRec11[0]) * fTemp17));
			fVec7[(IOTA & 131071)] = fTemp18;
			fRec8[0] = fVec7[((IOTA - iConst10) & 131071)];
			float fRec9 = (0.0f - (fTemp2 * fTemp18));
			fRec2[(IOTA & 131071)] = (fRec9 + fRec8[1]);
			fRec3[(IOTA & 131071)] = (fRec11[0] * fTemp17);
			fRec4[(IOTA & 131071)] = fTemp16;
			fVec8[(IOTA & 131071)] = (fTemp0 * fRec24[0]);
			fRec54[0] = (fVec8[((IOTA - iTemp3) & 131071)] + (fRec26[0] * fRec54[1]));
			fRec53[0] = ((fTemp4 * fRec54[0]) + (fRec27[0] * fRec53[1]));
			float fTemp19 = (((0.5f * (fRec53[0] * fTemp5)) + (fTemp6 * fRec53[1])) - (fRec28[0] * fRec51[1]));
			fVec9[(IOTA & 131071)] = fTemp19;
			fRec51[0] = fVec9[((IOTA - iConst11) & 131071)];
			float fRec52 = (fRec28[0] * fTemp19);
			float fTemp20 = ((fRec52 + fRec51[1]) - (fRec28[0] * fRec49[1]));
			fVec10[(IOTA & 131071)] = fTemp20;
			fRec49[0] = fVec10[((IOTA - iConst12) & 131071)];
			float fRec50 = (fRec28[0] * fTemp20);
			float fTemp21 = ((fRec50 + fRec49[1]) - (fRec29[0] * fRec47[1]));
			fVec11[(IOTA & 131071)] = fTemp21;
			fRec47[0] = fVec11[((IOTA - iConst13) & 131071)];
			float fRec48 = (fRec29[0] * fTemp21);
			float fTemp22 = ((fRec48 + fRec47[1]) - (fRec29[0] * fRec45[1]));
			fVec12[(IOTA & 131071)] = fTemp22;
			fRec45[0] = fVec12[((IOTA - iConst14) & 131071)];
			float fRec46 = (fRec29[0] * fTemp22);
			float fTemp23 = (fRec45[1] + ((fRec10[0] * fRec2[((IOTA - iConst15) & 131071)]) + (fRec46 + (fRec30[0] * fRec43[1]))));
			fVec13[(IOTA & 131071)] = fTemp23;
			int iTemp24 = (int((fConst0 * ((fRec35[0] * ftbl0faustFverbSIG0[int((65536.0f * fRec37[0]))]) + 0.025603978f))) + -1);
			float fTemp25 = ((fRec55[1] != 0.0f) ? (((fRec56[1] > 0.0f) & (fRec56[1] < 1.0f)) ? fRec55[1] : 0.0f) : (((fRec56[1] == 0.0f) & (iTemp24 != iRec57[1])) ? fConst7 : (((fRec56[1] == 1.0f) & (iTemp24 != iRec58[1])) ? fConst8 : 0.0f)));
			fRec55[0] = fTemp25;
			fRec56[0] = std::max<float>(0.0f, std::min<float>(1.0f, (fRec56[1] + fTemp25)));
			iRec57[0] = (((fRec56[1] >= 1.0f) & (iRec58[1] != iTemp24)) ? iTemp24 : iRec57[1]);
			iRec58[0] = (((fRec56[1] <= 0.0f) & (iRec57[1] != iTemp24)) ? iTemp24 : iRec58[1]);
			float fTemp26 = fVec13[((IOTA - std::min<int>(65536, std::max<int>(0, iRec57[0]))) & 131071)];
			fRec43[0] = (fTemp26 + (fRec56[0] * (fVec13[((IOTA - std::min<int>(65536, std::max<int>(0, iRec58[0]))) & 131071)] - fTemp26)));
			float fRec44 = (0.0f - (fRec30[0] * fTemp23));
			float fTemp27 = (fRec44 + fRec43[1]);
			fVec14[(IOTA & 131071)] = fTemp27;
			fRec42[0] = (fVec14[((IOTA - iConst16) & 131071)] + (fRec39[0] * fRec42[1]));
			float fTemp28 = ((fTemp2 * fRec40[1]) + ((fRec10[0] * fTemp17) * fRec42[0]));
			fVec15[(IOTA & 131071)] = fTemp28;
			fRec40[0] = fVec15[((IOTA - iConst17) & 131071)];
			float fRec41 = (0.0f - (fTemp2 * fTemp28));
			fRec5[(IOTA & 131071)] = (fRec41 + fRec40[1]);
			fRec6[(IOTA & 131071)] = (fTemp17 * fRec42[0]);
			fRec7[(IOTA & 131071)] = fTemp27;
			output0[i] = FAUSTFLOAT(((fTemp0 * fRec0[0]) + (0.600000024f * (fRec1[0] * (((fRec4[((IOTA - iConst18) & 131071)] + fRec4[((IOTA - iConst19) & 131071)]) + fRec2[((IOTA - iConst20) & 131071)]) - (((fRec3[((IOTA - iConst21) & 131071)] + fRec7[((IOTA - iConst22) & 131071)]) + fRec6[((IOTA - iConst23) & 131071)]) + fRec5[((IOTA - iConst24) & 131071)]))))));
			output1[i] = FAUSTFLOAT(((fTemp1 * fRec0[0]) + (0.600000024f * (fRec1[0] * (((fRec7[((IOTA - iConst25) & 131071)] + fRec7[((IOTA - iConst26) & 131071)]) + fRec5[((IOTA - iConst27) & 131071)]) - (((fRec6[((IOTA - iConst28) & 131071)] + fRec4[((IOTA - iConst29) & 131071)]) + fRec3[((IOTA - iConst30) & 131071)]) + fRec2[((IOTA - iConst31) & 131071)]))))));
			fRec0[1] = fRec0[0];
			fRec1[1] = fRec1[0];
			fRec10[1] = fRec10[0];
			fRec24[1] = fRec24[0];
			IOTA = (IOTA + 1);
			fRec25[1] = fRec25[0];
			fRec26[1] = fRec26[0];
			fRec23[1] = fRec23[0];
			fRec27[1] = fRec27[0];
			fRec22[1] = fRec22[0];
			fRec28[1] = fRec28[0];
			fRec20[1] = fRec20[0];
			fRec18[1] = fRec18[0];
			fRec29[1] = fRec29[0];
			fRec16[1] = fRec16[0];
			fRec14[1] = fRec14[0];
			fRec30[1] = fRec30[0];
			fRec35[1] = fRec35[0];
			fRec38[1] = fRec38[0];
			fRec37[1] = fRec37[0];
			fRec31[1] = fRec31[0];
			fRec32[1] = fRec32[0];
			iRec33[1] = iRec33[0];
			iRec34[1] = iRec34[0];
			fRec12[1] = fRec12[0];
			fRec39[1] = fRec39[0];
			fRec11[1] = fRec11[0];
			fRec8[1] = fRec8[0];
			fRec54[1] = fRec54[0];
			fRec53[1] = fRec53[0];
			fRec51[1] = fRec51[0];
			fRec49[1] = fRec49[0];
			fRec47[1] = fRec47[0];
			fRec45[1] = fRec45[0];
			fRec55[1] = fRec55[0];
			fRec56[1] = fRec56[0];
			iRec57[1] = iRec57[0];
			iRec58[1] = iRec58[0];
			fRec43[1] = fRec43[0];
			fRec42[1] = fRec42[0];
			fRec40[1] = fRec40[0];
		}
		//[End:compute]
	}


    FAUSTFLOAT getPredelay() const { return fHslider4; }
    void setPredelay(FAUSTFLOAT value) { fHslider4 = value; }

    FAUSTFLOAT getInputAmount() const { return fHslider3; }
    void setInputAmount(FAUSTFLOAT value) { fHslider3 = value; }

    FAUSTFLOAT getInputLowPassCutoff() const { return fHslider5; }
    void setInputLowPassCutoff(FAUSTFLOAT value) { fHslider5 = value; }

    FAUSTFLOAT getInputHighPassCutoff() const { return fHslider6; }
    void setInputHighPassCutoff(FAUSTFLOAT value) { fHslider6 = value; }

    FAUSTFLOAT getInputDiffusion1() const { return fHslider7; }
    void setInputDiffusion1(FAUSTFLOAT value) { fHslider7 = value; }

    FAUSTFLOAT getInputDiffusion2() const { return fHslider8; }
    void setInputDiffusion2(FAUSTFLOAT value) { fHslider8 = value; }

    FAUSTFLOAT getTailDensity() const { return fHslider9; }
    void setTailDensity(FAUSTFLOAT value) { fHslider9 = value; }

    FAUSTFLOAT getDecay() const { return fHslider2; }
    void setDecay(FAUSTFLOAT value) { fHslider2 = value; }

    FAUSTFLOAT getDamping() const { return fHslider12; }
    void setDamping(FAUSTFLOAT value) { fHslider12 = value; }

    FAUSTFLOAT getModulatorFrequency() const { return fHslider11; }
    void setModulatorFrequency(FAUSTFLOAT value) { fHslider11 = value; }

    FAUSTFLOAT getModulatorDepth() const { return fHslider10; }
    void setModulatorDepth(FAUSTFLOAT value) { fHslider10 = value; }

    FAUSTFLOAT getDry() const { return fHslider0; }
    void setDry(FAUSTFLOAT value) { fHslider0 = value; }

    FAUSTFLOAT getWet() const { return fHslider1; }
    void setWet(FAUSTFLOAT value) { fHslider1 = value; }

	//[End:class]
};
//[After:class]


#endif
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
#undef FAUSTFLOAT
#undef FAUSTCLASS
