/**
  Note(jpc): this is a personal edit of the Sallen-Key state-variable filter
             from `vaeffects.lib`. This changes:
             - the frequency control, now in Hz instead of 0-1
             - the smooth parameter, allowing for slower parameter transitions
               while keeping some expensive computation out of the frame loop
*/

import("stdfaust.lib");

declare author "Eric Tarr";
declare license "MIT-style STK-4.3 license";

//================================Sallen Key Filters======================================
// The following filters were implemented based on VA models of synthesizer
// filters.
//
// The modeling approach is based on a Topology Preserving Transform (TPT) to
// resolve the delay-free feedback loop in the corresponding analog filters.
//
// The primary processing block used to build other filters (Moog, Korg, etc.) is
// based on a 1st-order Sallen-Key filter.
//
// The filters included in this script are 1st-order LPF/HPF and 2nd-order
// state-variable filters capable of LPF, HPF, and BPF.
//
// #### Resources:
//
// * Vadim Zavalishin (2018) "The Art of VA Filter Design", v2.1.0
// <https://www.native-instruments.com/fileadmin/ni_media/downloads/pdf/VAFilterDesign_2.1.0.pdf>
// * Will Pirkle (2014) "Resolving Delay-Free Loops in Recursive Filters Using
// the Modified Härmä Method", AES 137 <http://www.aes.org/e-lib/browse.cfm?elib=17517>
// * Description and diagrams of 1st- and 2nd-order TPT filters:
// <https://www.willpirkle.com/706-2/>
//========================================================================================

//------------------`(fi.)sallenKey2ndOrder`-----------------
// Sallen-Key generic multi-outputs 2nd order filter.
//
// This is a 2nd-order Sallen-Key state-variable filter. The idea is that by
// "tapping" into different points in the circuit, different filters
// (LPF,BPF,HPF) can be achieved. See Figure 4.6 of
// <https://www.willpirkle.com/706-2/>
//
// This is also a good example of the next step for generalizing the Faust
// programming approach used for all these VA filters. In this case, there are
// three things to calculate each recursive step (y,s1,s2). For each thing, the
// circuit is only calculated up to that point.
//
// Comparing the LPF to BPF, the output signal (y) is calculated similarly.
// Except, the output of the BPF stops earlier in the circuit. Similarly, the
// states (s1 and s2) only differ in that s2 includes a couple more terms
// beyond what is used for s1.
//
// #### Usage
//
// ```
// _ : sallenKey2ndOrder(smooth,freq,Q) : _,_,_
// ```
//
// Where:
//
// * `freq`: cutoff frequency
// * `Q`: q
//---------------------------------------------------------------------
sallenKey2ndOrder(smooth,freq,Q) = _<:(s1,s2,ylpf,ybpf,yhpf) : !,!,_,_,_
letrec{
  's1 = -(s2):-(s1*FBs1):*(alpha0):*(g*2):+(s1);
  's2 = -(s2):-(s1*FBs1):*(alpha0):*(g):+(s1):*(g*2):+(s2);
  // Compute the LPF, BPF, HPF outputs
  'ylpf = -(s2):-(s1*FBs1):*(alpha0):*(g*2):+(s1):*(g):+(s2);
  'ybpf = -(s2):-(s1*FBs1):*(alpha0):*(g):+(s1);
  'yhpf = -(s2):-(s1*FBs1):*(alpha0);
}
with{
  wd = 2*ma.PI*freq;
  T = 1/ma.SR;
  wa = (2/T)*tan(wd*T/2);
  g = (wa*T/2):smooth;
  G = g/(1.0 + g);
  R = 1/(2*Q);
  FBs1 = (2*R+g):smooth;
  alpha0 = (1/(1 + 2*R*g + g*g)):smooth;
};

//------------------`(fi.)sallenKey2ndOrderLPF`-----------------
// Sallen-Key 2nd order lowpass filter (see description above).
//
//
// #### Usage
//
// ```
// _ : sallenKey2ndOrderLPF(smooth,freq,Q) : _
// ```
//
// Where:
//
// * `freq`: cutoff frequency
// * `Q`: q
//---------------------------------------------------------------------
// Specialize the generic implementation: keep the first LPF output, the compiler will only generate the needed code
sallenKey2ndOrderLPF(smooth,freq,Q) = sallenKey2ndOrder(smooth,freq,Q) : _,!,!;


//------------------`(fi.)sallenKey2ndOrderBPF`-----------------
// Sallen-Key 2nd order bandpass filter (see description above).
//
//
// #### Usage
//
// ```
// _ : sallenKey2ndOrderBPF(smooth,freq,Q) : _
// ```
//
// Where:
//
// * `freq`: cutoff frequency
// * `Q`: q
//---------------------------------------------------------------------
// Specialize the generic implementation: keep the second BPF output, the compiler will only generate the needed code
sallenKey2ndOrderBPF(smooth,freq,Q) = sallenKey2ndOrder(smooth,freq,Q) : !,_,!;


//------------------`(fi.)sallenKey2ndOrderHPF`-----------------
// Sallen-Key 2nd order highpass filter (see description above).
//
//
// #### Usage
//
// ```
// _ : sallenKey2ndOrderHPF(smooth,freq,Q) : _
// ```
//
// Where:
//
// * `freq`: cutoff frequency
// * `Q`: q
//---------------------------------------------------------------------
// Specialize the generic implementation: keep the third HPF output, the compiler will only generate the needed code
sallenKey2ndOrderHPF(smooth,freq,Q) = sallenKey2ndOrder(smooth,freq,Q) : !,!,_;
