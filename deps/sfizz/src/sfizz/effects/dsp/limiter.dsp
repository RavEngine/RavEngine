import("stdfaust.lib");

limiter(x) = gain*x with {
  att = 0.0008;
  rel = 0.5;
  peak = x : an.amp_follower_ud(att, rel);
  gain = ba.if(peak>1.0, 1.0/peak, 1.0) : si.smooth(ba.tau2pole(0.5*att));
};

process = limiter, limiter;
