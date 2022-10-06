// -*- mode: faust; -*-

declare author "Jean Pierre Cimalando";
declare license "BSD-2-Clause";

import("stdfaust.lib");
fm = library("filters_modulable.dsp");
sk = library("sallenkey_modulable.dsp");

//==============================================================================
// Generators

// the SFZ *noise generator
sfzNoise = no.noise : *(0.25);

//==============================================================================
// Filters
// To generate a specific filter from this file, use:
//      faust2jack -double -pn sfzPeq src/sfizz/dsp/filters/sfz_filters.dsp
// and replace sfzPeq by the filter you want

// the SFZ lowpass 1-pole filter
sfzLpf1p = fm.lp1Smooth(smoothCoefs,cutoff);

// the SFZ lowpass 2-pole filter
sfzLpf2p = fm.rbjLpfSmooth(smoothCoefs,cutoff,0.,Q);

// the SFZ lowpass 4-pole filter
sfzLpf4p = sfzLpf2p : sfzLpf2p;

// the SFZ lowpass 6-pole filter
sfzLpf6p = sfzLpf2p : sfzLpf2p : sfzLpf2p;

// the SFZ highpass 1-pole filter
sfzHpf1p = fm.hp1Smooth(smoothCoefs,cutoff);

// the SFZ highpass 2-pole filter
sfzHpf2p = fm.rbjHpfSmooth(smoothCoefs,cutoff,0.,Q);

// the SFZ highpass 4-pole filter
sfzHpf4p = sfzHpf2p : sfzHpf2p;

// the SFZ highpass 6-pole filter
sfzHpf6p = sfzHpf2p : sfzHpf2p : sfzHpf2p;

// the SFZ bandpass 1-pole filter
sfzBpf1p = sfzLpf1p : sfzHpf1p;

// the SFZ bandpass 2-pole filter
sfzBpf2p = fm.rbjBpfSmooth(smoothCoefs,cutoff,0.,Q);

// the SFZ bandpass 4-pole filter
//   Note: bpf_4p not in specification but here anyway
sfzBpf4p = sfzBpf2p : sfzBpf2p;

// the SFZ bandpass 6-pole filter
//   Note: bpf_6p not in specification but here anyway
sfzBpf6p = sfzBpf2p : sfzBpf2p : sfzBpf2p;

// the SFZ allpass 1-pole filter
sfzApf1p = fm.ap1Smooth(smoothCoefs,cutoff);

// the SFZ notch 1-pole filter
//   Note: this thing is my invention, may not be correct.
//         in Sforzando, 1p seems implemented the same as 2p.
sfzBrf1p = _ <: (_, (sfzApf1p : sfzApf1p)) :> +;

// the SFZ notch 2-pole filter
sfzBrf2p = fm.rbjNotchSmooth(smoothCoefs,cutoff,0.,Q);

// the SFZ pink filter
sfzPink = no.pink_filter;

// the SFZ 2-pole state-variable lowpass filter
sfzLpf2pSv = sk.sallenKey2ndOrderLPF(smoothCoefs,cutoff,Q);

// the SFZ 2-pole state-variable highpass filter
sfzHpf2pSv = sk.sallenKey2ndOrderHPF(smoothCoefs,cutoff,Q);

// the SFZ 2-pole state-variable bandpass filter
sfzBpf2pSv = sk.sallenKey2ndOrderBPF(smoothCoefs,cutoff,Q);

// the SFZ 2-pole state-variable notch filter
sfzBrf2pSv = _ <: (sfzLpf2pSv, sfzHpf2pSv) :> +;

// the SFZ low-shelf filter
sfzLsh = fm.rbjLowShelfSmooth(smoothCoefs,cutoff,pkShGain,Q);

// the SFZ high-shelf filter
sfzHsh = fm.rbjHighShelfSmooth(smoothCoefs,cutoff,pkShGain,Q);

// the SFZ peaking EQ filter
sfzPeq = fm.rbjPeakingEqSmooth(smoothCoefs,cutoff,pkShGain,Q);

// the SFZ equalizer band
sfzEqPeak = fm.rbjPeakingEqSmooth(smoothCoefs,cutoff,pkShGain,Q) with {
  Q = 1./(2.*ma.sinh(0.5*log(2)*bandwidth*w0/sin(w0)));
  w0 = 2*ma.PI*cutoff/ma.SR;
};

// the SFZ low-shelf with EQ controls
sfzEqLshelf = fm.rbjLowShelfSmooth(smoothCoefs,cutoff,pkShGain,Q) with {
//  Q = sfzGetQFromSlope(slope);
  Q = 1./(2.*ma.sinh(0.5*log(2)*bandwidth*w0/sin(w0)));
  w0 = 2*ma.PI*cutoff/ma.SR;
};

// the SFZ high-shelf with EQ controls
sfzEqHshelf = fm.rbjHighShelfSmooth(smoothCoefs,cutoff,pkShGain,Q) with {
//  Q = sfzGetQFromSlope(slope);
  Q = 1./(2.*ma.sinh(0.5*log(2)*bandwidth*w0/sin(w0)));
  w0 = 2*ma.PI*cutoff/ma.SR;
};

//==============================================================================
// Utility

// a common function that computes the EQ shelf parameter
sfzGetQFromSlope(slope) = 1.0/sqrt((A+1.0/A)*(1.0/S-1.0)+2.0) with {
  // note(jpc) slope is a 0-1 control that is reduced into a domain of validity,
  //   and clamped to avoid the extremes at both sides.
  S = (slope*root) : max(1e-2) : min(root-1e-2);
  A = 10^(pkShGain/40);
  root = (A*A+1)/((A-1)*(A-1)); // the root of the expression under sqrt()
};

//==============================================================================
// Filters (stereo)

sfz2chLpf1p = par(i,2,sfzLpf1p);
sfz2chLpf2p = par(i,2,sfzLpf2p);
sfz2chLpf4p = par(i,2,sfzLpf4p);
sfz2chLpf6p = par(i,2,sfzLpf6p);
sfz2chHpf1p = par(i,2,sfzHpf1p);
sfz2chHpf2p = par(i,2,sfzHpf2p);
sfz2chHpf4p = par(i,2,sfzHpf4p);
sfz2chHpf6p = par(i,2,sfzHpf6p);
sfz2chBpf1p = par(i,2,sfzBpf1p);
sfz2chBpf2p = par(i,2,sfzBpf2p);
sfz2chBpf4p = par(i,2,sfzBpf4p);
sfz2chBpf6p = par(i,2,sfzBpf6p);
sfz2chApf1p = par(i,2,sfzApf1p);
sfz2chBrf1p = par(i,2,sfzBrf1p);
sfz2chBrf2p = par(i,2,sfzBrf2p);
sfz2chPink = par(i,2,sfzPink);
sfz2chLpf2pSv = par(i,2,sfzLpf2pSv);
sfz2chHpf2pSv = par(i,2,sfzHpf2pSv);
sfz2chBpf2pSv = par(i,2,sfzBpf2pSv);
sfz2chBrf2pSv = par(i,2,sfzBrf2pSv);
sfz2chLsh = par(i,2,sfzLsh);
sfz2chHsh = par(i,2,sfzHsh);
sfz2chPeq = par(i,2,sfzPeq);
sfz2chEqPeak = par(i,2,sfzEqPeak);
sfz2chEqLshelf = par(i,2,sfzEqLshelf);
sfz2chEqHshelf = par(i,2,sfzEqHshelf);

//==============================================================================
// Filter parameters

cutoff = hslider("[01] Cutoff [unit:Hz] [scale:log]", 440.0, 50.0, 10000.0, 1.0) : max(1.0) : min(20000.0);
Q = vslider("[02] Resonance [unit:dB]", 0.0, 0.0, 40.0, 0.1) : max(-60.0) : min(60.0) : ba.db2linear;
pkShGain = vslider("[03] Peak/shelf gain [unit:dB]", 0.0, 0.0, 40.0, 0.1) : max(-120.0) : min(60.0);
bandwidthOrSlope = vslider("[04] Bandwidth [unit:octave]", 1.0, 0.1, 10.0, 0.01);
bandwidth = bandwidthOrSlope : max(1e-2) : min(12.0);
slope = bandwidthOrSlope; // limited further down in code

// smoothing function to prevent fast changes of filter coefficients
// The basic si.smoo is a bit longish and creates strange modulation sounds
// The smoothing coefficients seem to start at 0, and thus there's a small
// time where the coefficients go e.g. from 0 to cutoff.
// smoothCoefs = si.smoo;

// The below line uses no smoothing at all. This could require smoothing in code
// done on the filter parameters rather than directly on the biquad's coefficients.
// smoothCoefs = _ ; // No smoothing applied at all

// This applies a quicker smoothing but which may render the filter unstable
smoothCoefs = si.smooth(ba.if(smoothEnable, pole, 0.0)) with {
  timeConstant = 1e-3; // time constant = 1ms
  pole = ba.tau2pole(timeConstant);
  smoothEnable = fvariable(int fSmoothEnable, <math.h>);
};
