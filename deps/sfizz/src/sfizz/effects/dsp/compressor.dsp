import("stdfaust.lib");

cgain = co.compression_gain_mono(ratio, thresh, att, rel) with {
  ratio = hslider("[1] Ratio", 1.0, 1.0, 20.0, 0.01);
  thresh = hslider("[2] Threshold [unit:dB]", 0.0, -60.0, 0.0, 0.01);
  att = hslider("[3] Attack [unit:s]", 0.0, 0.0, 0.5, 1e-3);
  rel = hslider("[4] Release [unit:s]", 0.0, 0.0, 5.0, 1e-3);
};

process = cgain;
