// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Config.h"
#include "SfzFilter.h"
#include "SfzFilterImpls.hpp"
#include "SIMDHelpers.h"
#include "utility/StringViewHelpers.h"
#include "utility/Debug.h"
#include <cstring>

namespace sfz {

//------------------------------------------------------------------------------
// SFZ v2 multi-mode filter

struct Filter::Impl {
    double fSampleRate = sfz::config::defaultSampleRate;
    FilterType fType = kFilterNone;
    unsigned fChannels = 1;
    enum { maxChannels = 2 };

    union U {
        U() {}
        ~U() {}

        #define DECLARE_MEMBER(type)    \
            sfz##type fDsp##type;       \
            sfz2ch##type fDsp2ch##type;
        SFZ_EACH_FILTER(DECLARE_MEMBER)
        #undef DECLARE_MEMBER
    } u;

    sfzFilterDsp *getDsp(unsigned channels, FilterType type);
    sfzFilterDsp *newDsp(unsigned channels, FilterType type);

    static constexpr uint32_t idDsp(unsigned channels, FilterType type)
    {
        return static_cast<unsigned>(type)|(channels << 16);
    }
};

Filter::Filter()
    : P{new Impl}
{
}

Filter::~Filter()
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->~sfzFilterDsp();
}

void Filter::init(double sampleRate)
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->init(sampleRate);

    P->fSampleRate = sampleRate;
}

void Filter::clear()
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->instanceClear();
}

void Filter::prepare(float cutoff, float q, float pksh)
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (!dsp)
        return;

    // compute a dummy 1-frame cycle with smoothing off

    float buffer[Impl::maxChannels] = {0};
    float *inout[Impl::maxChannels];
    bool en = dsp->isSmoothingEnabled();

    for (unsigned i = 0; i < Impl::maxChannels; ++i)
        inout[i] = &buffer[i];

    dsp->instanceClear();
    dsp->configureStandard(cutoff, q, pksh);
    dsp->setSmoothingEnabled(false);
    dsp->compute(1, inout, inout);
    dsp->setSmoothingEnabled(en);
}

void Filter::process(const float *const in[], float *const out[], float cutoff, float q, float pksh, unsigned nframes)
{
    unsigned channels = P->fChannels;
    sfzFilterDsp *dsp = P->getDsp(channels, P->fType);

    if (!dsp) {
        for (unsigned c = 0; c < channels; ++c)
            copy<float>({ in[c], nframes }, { out[c], nframes });
        return;
    }

    dsp->configureStandard(cutoff, q, pksh);
    dsp->compute(nframes, const_cast<float **>(in), const_cast<float **>(out));
}

void Filter::processModulated(const float *const in[], float *const out[], const float *cutoff, const float *q, const float *pksh, unsigned nframes)
{
    unsigned channels = P->fChannels;
    sfzFilterDsp *dsp = P->getDsp(channels, P->fType);

    if (!dsp) {
        for (unsigned c = 0; c < channels; ++c)
            copy<float>({ in[c], nframes }, { out[c], nframes });
        return;
    }

    unsigned frame = 0;
    while (frame < nframes) {
        unsigned current = nframes - frame;

        if (current > config::filterControlInterval)
            current = config::filterControlInterval;

        const float *current_in[Impl::maxChannels];
        float *current_out[Impl::maxChannels];

        for (unsigned c = 0; c < channels; ++c) {
            current_in[c] = in[c] + frame;
            current_out[c] = out[c] + frame;
        }

        dsp->configureStandard(cutoff[frame], q[frame], pksh[frame]);
        dsp->compute(current, const_cast<float **>(current_in), const_cast<float **>(current_out));

        frame += current;
    }
}

unsigned Filter::channels() const
{
    return P->fChannels;
}

void Filter::setChannels(unsigned channels)
{
    ASSERT(channels <= Impl::maxChannels);
    if (P->fChannels != channels) {
        sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);
        if (dsp)
            dsp->~sfzFilterDsp();

        P->fChannels = channels;

        dsp = P->newDsp(channels, P->fType);
        if (dsp)
            dsp->init(P->fSampleRate);
    }
}

FilterType Filter::type() const
{
    return P->fType;
}

void Filter::setType(FilterType type)
{
    if (P->fType != type) {
        sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);
        if (dsp)
            dsp->~sfzFilterDsp();

        P->fType = type;

        dsp = P->newDsp(P->fChannels, type);
        if (dsp)
            dsp->init(P->fSampleRate);
    }
}

sfzFilterDsp *Filter::Impl::getDsp(unsigned channels, FilterType type)
{
    switch (idDsp(channels, type)) {
    default: return nullptr;

    #define CASE(type)                                         \
        case idDsp(1, kFilter##type): return &u.fDsp##type;    \
        case idDsp(2, kFilter##type): return &u.fDsp2ch##type;
    SFZ_EACH_FILTER(CASE)
    #undef CASE
    }
}

sfzFilterDsp *Filter::Impl::newDsp(unsigned channels, FilterType type)
{
    switch (idDsp(channels, type)) {
    default: return nullptr;

    #define CASE(type)                                                           \
        case idDsp(1, kFilter##type): return new(&u.fDsp##type) sfz##type;       \
        case idDsp(2, kFilter##type): return new(&u.fDsp2ch##type) sfz2ch##type;
    SFZ_EACH_FILTER(CASE)
    #undef CASE
    }
}

//------------------------------------------------------------------------------
// SFZ v1 equalizer filter

struct FilterEq::Impl {
    double fSampleRate = sfz::config::defaultSampleRate;
    EqType fType = kEqNone;
    unsigned fChannels = 1;
    enum { maxChannels = 2 };

    union U {
        U() {}
        ~U() {}

        #define DECLARE_MEMBER(type)      \
            sfzEq##type fDsp##type;       \
            sfz2chEq##type fDsp2ch##type;
        SFZ_EACH_EQ(DECLARE_MEMBER)
        #undef DECLARE_MEMBER
    } u;

    sfzFilterDsp *getDsp(unsigned channels, EqType type);
    sfzFilterDsp *newDsp(unsigned channels, EqType type);

    static constexpr uint32_t idDsp(unsigned channels, EqType type)
    {
        return static_cast<unsigned>(type)|(channels << 16);
    }
};

FilterEq::FilterEq()
    : P{new Impl}
{
}

FilterEq::~FilterEq()
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->~sfzFilterDsp();
}

void FilterEq::init(double sampleRate)
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->init(sampleRate);

    P->fSampleRate = sampleRate;
}

void FilterEq::clear()
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (dsp)
        dsp->instanceClear();
}

void FilterEq::prepare(float cutoff, float bw, float pksh)
{
    sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);

    if (!dsp)
        return;

    // compute a dummy 1-frame cycle with smoothing off

    float buffer[Impl::maxChannels] = {0};
    float *inout[Impl::maxChannels];
    bool en = dsp->isSmoothingEnabled();

    for (unsigned i = 0; i < Impl::maxChannels; ++i)
        inout[i] = &buffer[i];

    dsp->instanceClear();
    dsp->configureEq(cutoff, bw, pksh);
    dsp->setSmoothingEnabled(false);
    dsp->compute(1, inout, inout);
    dsp->setSmoothingEnabled(en);
}

void FilterEq::process(const float *const in[], float *const out[], float cutoff, float bw, float pksh, unsigned nframes)
{
    unsigned channels = P->fChannels;
    sfzFilterDsp *dsp = P->getDsp(channels, P->fType);

    if (!dsp) {
        for (unsigned c = 0; c < channels; ++c)
            copy<float>({ in[c], nframes }, { out[c], nframes });
        return;
    }

    dsp->configureEq(cutoff, bw, pksh);
    dsp->compute(nframes, const_cast<float **>(in), const_cast<float **>(out));
}

void FilterEq::processModulated(const float *const in[], float *const out[], const float *cutoff, const float *bw, const float *pksh, unsigned nframes)
{
    unsigned channels = P->fChannels;
    sfzFilterDsp *dsp = P->getDsp(channels, P->fType);

    if (!dsp) {
        for (unsigned c = 0; c < channels; ++c)
            copy<float>({ in[c], nframes }, { out[c], nframes });
        return;
    }

    unsigned frame = 0;
    while (frame < nframes) {
        unsigned current = nframes - frame;

        if (current > config::filterControlInterval)
            current = config::filterControlInterval;

        const float *current_in[Impl::maxChannels];
        float *current_out[Impl::maxChannels];

        for (unsigned c = 0; c < channels; ++c) {
            current_in[c] = in[c] + frame;
            current_out[c] = out[c] + frame;
        }

        dsp->configureEq(cutoff[frame], bw[frame], pksh[frame]);
        dsp->compute(current, const_cast<float **>(current_in), const_cast<float **>(current_out));

        frame += current;
    }
}


unsigned FilterEq::channels() const
{
    return P->fChannels;
}

void FilterEq::setChannels(unsigned channels)
{
    ASSERT(channels <= Impl::maxChannels);
    if (P->fChannels != channels) {
        sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);
        if (dsp)
            dsp->~sfzFilterDsp();

        P->fChannels = channels;

        dsp = P->newDsp(channels, P->fType);
        if (dsp)
            dsp->init(P->fSampleRate);
    }
}

EqType FilterEq::type() const
{
    return P->fType;
}

void FilterEq::setType(EqType type)
{
    if (P->fType != type) {
        sfzFilterDsp *dsp = P->getDsp(P->fChannels, P->fType);
        if (dsp)
            dsp->~sfzFilterDsp();

        P->fType = type;

        dsp = P->newDsp(P->fChannels, type);
        if (dsp)
            dsp->init(P->fSampleRate);
    }
}

sfzFilterDsp *FilterEq::Impl::getDsp(unsigned channels, EqType type)
{
    switch (idDsp(channels, type)) {
    default: return nullptr;

    #define CASE(type)                                     \
        case idDsp(1, kEq##type): return &u.fDsp##type;    \
        case idDsp(2, kEq##type): return &u.fDsp2ch##type;
    SFZ_EACH_EQ(CASE)
    #undef CASE
    }
}

sfzFilterDsp *FilterEq::Impl::newDsp(unsigned channels, EqType type)
{
    switch (idDsp(channels, type)) {
    default: return nullptr;

    #define CASE(type)                                                          \
        case idDsp(1, kEq##type): return new (&u.fDsp##type) sfzEq##type;       \
        case idDsp(2, kEq##type): return new (&u.fDsp2ch##type) sfz2chEq##type;
    SFZ_EACH_EQ(CASE)
    #undef CASE
    }
}

} // namespace sfz
