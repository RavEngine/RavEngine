import("stdfaust.lib");

disto_stage(depth, x) = shs*hh(x)+(1.0-shs)*lh(x) : fi.dcblockerat(40.0) with {
  // sigmoid parameters
  a = depth*0.2+2.0;
  b = 2.0;

  // smooth hysteresis transition
  shs = hs : si.smooth(ba.tau2pole(10e-3));

  // the low and high hysteresis
  lh(x) = sig(a*x)*b;
  hh(x) = (sig(a*x)-1.0)*b;

  //
  sig10 = environment { // sigmoid sampled from -10 to +10
      tablesize = 256;
      table(i) = rdtable(tablesize, exact(float(ba.time)/float(tablesize)*20.0-10.0), i);
      exact(x) = exp(x)/(exp(x)+1.0);
      approx(x) = s1+mu*(s2-s1) with {
        index = (x+10.0)*(1.0/20.0)*(sig10.tablesize-1) : max(0.0);
        mu = index-int(index);
        s1 = sig10.table(int(index) : min(sig10.tablesize-1));
        s2 = sig10.table(int(index) : +(1) : min(sig10.tablesize-1));
      };
  };

  //sig = sig10.exact;
  sig = sig10.approx;
}
letrec {
  // hysteresis selection
  'hs = ba.if((x<x') & (x<-0.25), 1, ba.if((x>x') & (x>0.25), 0, hs));
};

process = disto_stage(d) with {
  d = hslider("[1] Depth", 100.0, 0.0, 100.0, 0.01);
};
