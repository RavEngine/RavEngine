import("stdfaust.lib");

f = hslider("[1] Resonance frequency [unit:Hz]", 1, 0, 22000, 1);
r = hslider("[2] Resonance feedback", 0, 0, 1, 0.001);
b = hslider("[3] Bandwidth [unit:Hz]", 1, 0, 10, 0.01);
g = hslider("[4] Gain", 0, 0, 1, 0.01);

process = fi.bandpass(1, f-0.5*b, f+0.5*b) : fi.nlf2(f, r) : (_,!) : *(g);
