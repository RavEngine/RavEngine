// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

struct Meta {};
struct UI {};

/**
   Base of the faust DSP for filters
 */
struct sfzFilterDsp {
public:
    virtual ~sfzFilterDsp() {}

    virtual void init(int) = 0;
    virtual void instanceClear() = 0;
    virtual void compute(int, const float *const *, float *const *) = 0;

    virtual void configureStandard(float, float, float) {}
    virtual void configureEq(float, float, float) {}

    bool isSmoothingEnabled() const
    {
        return fSmoothEnable;
    }

    void setSmoothingEnabled(bool smooth)
    {
        fSmoothEnable = smooth;
    }

protected:
    bool fSmoothEnable = true; // external variable used from faust code
};

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "gen/filters/sfzApf1p.hxx"
#include "gen/filters/sfzBpf1p.hxx"
#include "gen/filters/sfzBpf2p.hxx"
#include "gen/filters/sfzBpf4p.hxx"
#include "gen/filters/sfzBpf6p.hxx"
#include "gen/filters/sfzBrf1p.hxx"
#include "gen/filters/sfzBrf2p.hxx"
#include "gen/filters/sfzHpf1p.hxx"
#include "gen/filters/sfzHpf2p.hxx"
#include "gen/filters/sfzHpf4p.hxx"
#include "gen/filters/sfzHpf6p.hxx"
#include "gen/filters/sfzLpf1p.hxx"
#include "gen/filters/sfzLpf2p.hxx"
#include "gen/filters/sfzLpf4p.hxx"
#include "gen/filters/sfzLpf6p.hxx"
#include "gen/filters/sfzPink.hxx"
#include "gen/filters/sfzLpf2pSv.hxx"
#include "gen/filters/sfzHpf2pSv.hxx"
#include "gen/filters/sfzBpf2pSv.hxx"
#include "gen/filters/sfzBrf2pSv.hxx"
#include "gen/filters/sfzLsh.hxx"
#include "gen/filters/sfzHsh.hxx"
#include "gen/filters/sfzPeq.hxx"
#include "gen/filters/sfzEqPeak.hxx"
#include "gen/filters/sfzEqLshelf.hxx"
#include "gen/filters/sfzEqHshelf.hxx"

#include "gen/filters/sfz2chApf1p.hxx"
#include "gen/filters/sfz2chBpf1p.hxx"
#include "gen/filters/sfz2chBpf2p.hxx"
#include "gen/filters/sfz2chBpf4p.hxx"
#include "gen/filters/sfz2chBpf6p.hxx"
#include "gen/filters/sfz2chBrf1p.hxx"
#include "gen/filters/sfz2chBrf2p.hxx"
#include "gen/filters/sfz2chHpf1p.hxx"
#include "gen/filters/sfz2chHpf2p.hxx"
#include "gen/filters/sfz2chHpf4p.hxx"
#include "gen/filters/sfz2chHpf6p.hxx"
#include "gen/filters/sfz2chLpf1p.hxx"
#include "gen/filters/sfz2chLpf2p.hxx"
#include "gen/filters/sfz2chLpf4p.hxx"
#include "gen/filters/sfz2chLpf6p.hxx"
#include "gen/filters/sfz2chPink.hxx"
#include "gen/filters/sfz2chLpf2pSv.hxx"
#include "gen/filters/sfz2chHpf2pSv.hxx"
#include "gen/filters/sfz2chBpf2pSv.hxx"
#include "gen/filters/sfz2chBrf2pSv.hxx"
#include "gen/filters/sfz2chLsh.hxx"
#include "gen/filters/sfz2chHsh.hxx"
#include "gen/filters/sfz2chPeq.hxx"
#include "gen/filters/sfz2chEqPeak.hxx"
#include "gen/filters/sfz2chEqLshelf.hxx"
#include "gen/filters/sfz2chEqHshelf.hxx"

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

/**
   Wrapper of the most common kind of resonant filter
   Parameterized by cutoff and Q
 */
template <class F> struct sfzFilter : public F {
    void configureStandard(float cutoff, float q, float pksh) override
    {
        this->setCutoff(cutoff);
        this->setResonance(q);
        (void)pksh;
    }
};

/**
   Wrapper of non resonant filters
   Parameterized by cutoff only
 */
template <class F> struct sfzFilterNoQ : public F {
    void configureStandard(float cutoff, float q, float pksh) override
    {
        this->setCutoff(cutoff);
        (void)q;
        (void)pksh;
    }
};

/**
   Wrapper of fixed filters
   Not parameterized
 */
template <class F> struct sfzFilterNoCutoff : public F {
    void configureStandard(float cutoff, float q, float pksh) override
    {
        (void)cutoff;
        (void)q;
        (void)pksh;
    }
};

/**
   Wrapper of resonant filters with a gain control for peak or shelf
   Parameterized by cutoff, Q, peak/shelf gain
 */
template <class F> struct sfzFilterPkSh : public F {
    void configureStandard(float cutoff, float q, float pksh) override
    {
        this->setCutoff(cutoff);
        this->setResonance(q);
        this->setPeakShelfGain(pksh);
    }
};

/**
   Wrapper of equalizer filters with a bandwidth control
   Parameterized by cutoff, bandwidth, peak/shelf gain
 */
template <class F> struct sfzFilterEq : public F {
    void configureEq(float cutoff, float bw, float pksh) override
    {
        this->setCutoff(cutoff);
        this->setBandwidth(bw);
        this->setPeakShelfGain(pksh);
    }
};

struct sfzLpf1p final : public sfzFilterNoQ<faustLpf1p> {};
struct sfzLpf2p final : public sfzFilter<faustLpf2p> {};
struct sfzLpf4p final : public sfzFilter<faustLpf4p> {};
struct sfzLpf6p final : public sfzFilter<faustLpf6p> {};
struct sfzHpf1p final : public sfzFilterNoQ<faustHpf1p> {};
struct sfzHpf2p final : public sfzFilter<faustHpf2p> {};
struct sfzHpf4p final : public sfzFilter<faustHpf4p> {};
struct sfzHpf6p final : public sfzFilter<faustHpf6p> {};
struct sfzBpf1p final : public sfzFilterNoQ<faustBpf1p> {};
struct sfzBpf2p final : public sfzFilter<faustBpf2p> {};
struct sfzBpf4p final : public sfzFilter<faustBpf4p> {};
struct sfzBpf6p final : public sfzFilter<faustBpf6p> {};
struct sfzApf1p final : public sfzFilterNoQ<faustApf1p> {};
struct sfzBrf1p final : public sfzFilterNoQ<faustBrf1p> {};
struct sfzBrf2p final : public sfzFilter<faustBrf2p> {};
struct sfzPink final : public sfzFilterNoCutoff<faustPink> {};
struct sfzLpf2pSv final : public sfzFilter<faustLpf2pSv> {};
struct sfzHpf2pSv final : public sfzFilter<faustHpf2pSv> {};
struct sfzBpf2pSv final : public sfzFilter<faustBpf2pSv> {};
struct sfzBrf2pSv final : public sfzFilter<faustBrf2pSv> {};
struct sfzLsh final : public sfzFilterPkSh<faustLsh> {};
struct sfzHsh final : public sfzFilterPkSh<faustHsh> {};
struct sfzPeq final : public sfzFilterPkSh<faustPeq> {};
struct sfzEqPeak final : public sfzFilterEq<faustEqPeak> {};
struct sfzEqLshelf final : public sfzFilterEq<faustEqLshelf> {};
struct sfzEqHshelf final : public sfzFilterEq<faustEqHshelf> {};

struct sfz2chLpf1p final : public sfzFilterNoQ<faust2chLpf1p> {};
struct sfz2chLpf2p final : public sfzFilter<faust2chLpf2p> {};
struct sfz2chLpf4p final : public sfzFilter<faust2chLpf4p> {};
struct sfz2chLpf6p final : public sfzFilter<faust2chLpf6p> {};
struct sfz2chHpf1p final : public sfzFilterNoQ<faust2chHpf1p> {};
struct sfz2chHpf2p final : public sfzFilter<faust2chHpf2p> {};
struct sfz2chHpf4p final : public sfzFilter<faust2chHpf4p> {};
struct sfz2chHpf6p final : public sfzFilter<faust2chHpf6p> {};
struct sfz2chBpf1p final : public sfzFilterNoQ<faust2chBpf1p> {};
struct sfz2chBpf2p final : public sfzFilter<faust2chBpf2p> {};
struct sfz2chBpf4p final : public sfzFilter<faust2chBpf4p> {};
struct sfz2chBpf6p final : public sfzFilter<faust2chBpf6p> {};
struct sfz2chApf1p final : public sfzFilterNoQ<faust2chApf1p> {};
struct sfz2chBrf1p final : public sfzFilterNoQ<faust2chBrf1p> {};
struct sfz2chBrf2p final : public sfzFilter<faust2chBrf2p> {};
struct sfz2chPink final : public sfzFilterNoCutoff<faust2chPink> {};
struct sfz2chLpf2pSv final : public sfzFilter<faust2chLpf2pSv> {};
struct sfz2chHpf2pSv final : public sfzFilter<faust2chHpf2pSv> {};
struct sfz2chBpf2pSv final : public sfzFilter<faust2chBpf2pSv> {};
struct sfz2chBrf2pSv final : public sfzFilter<faust2chBrf2pSv> {};
struct sfz2chLsh final : public sfzFilterPkSh<faust2chLsh> {};
struct sfz2chHsh final : public sfzFilterPkSh<faust2chHsh> {};
struct sfz2chPeq final : public sfzFilterPkSh<faust2chPeq> {};
struct sfz2chEqPeak final : public sfzFilterEq<faust2chEqPeak> {};
struct sfz2chEqLshelf final : public sfzFilterEq<faust2chEqLshelf> {};
struct sfz2chEqHshelf final : public sfzFilterEq<faust2chEqHshelf> {};

#define SFZ_EACH_FILTER(F) \
    F(Apf1p) \
    F(Bpf1p) \
    F(Bpf2p) \
    F(Bpf4p) \
    F(Bpf6p) \
    F(Brf1p) \
    F(Brf2p) \
    F(Hpf1p) \
    F(Hpf2p) \
    F(Hpf4p) \
    F(Hpf6p) \
    F(Lpf1p) \
    F(Lpf2p) \
    F(Lpf4p) \
    F(Lpf6p) \
    F(Pink) \
    F(Lpf2pSv) \
    F(Hpf2pSv) \
    F(Bpf2pSv) \
    F(Brf2pSv) \
    F(Lsh) \
    F(Hsh) \
    F(Peq)

#define SFZ_EACH_EQ(F) \
    F(Peak) \
    F(Lshelf) \
    F(Hshelf)
