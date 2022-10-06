import("stdfaust.lib");

ggain = ef.gate_gain_mono(thresh, att, hold, rel) with {
  thresh = hslider("[1] Threshold [unit:dB]", 0.0, -60.0, 0.0, 0.01);
  att = hslider("[2] Attack [unit:s]", 0.0, 0.0, 10.0, 1e-3);
  hold = hslider("[3] Hold [unit:s]", 0.0, 0.0, 10.0, 1e-3);
  rel = hslider("[4] Release [unit:s]", 0.0, 0.0, 5.0, 1e-3);
};

process = ggain;
