// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#include "LabSound/extended/DiodeNode.h"
#include "LabSound/extended/Registry.h"

#include "LabSound/core/AudioSetting.h"
#include "LabSound/core/Macros.h"
#include <mutex>

namespace lab
{

// Based on the DiodeNode found at the BBC Radiophonic Workshop
// http://webaudio.prototyping.bbc.co.uk/ring-modulator/

DiodeNode::DiodeNode(AudioContext & ac)
    : WaveShaperNode(ac)
    , _distortion(std::make_shared<AudioSetting>("distortion", "DSTR", AudioSetting::Type::Float))
    , _vb(std::make_shared<AudioSetting>("vb", "VB  ", AudioSetting::Type::Float))
    , _vl(std::make_shared<AudioSetting>("vl", "VL  ", AudioSetting::Type::Float))
{
    _vb->setFloat(0.2f);
    _vl->setFloat(0.4f);
    setDistortion(1.f);
    m_settings.push_back(_distortion);
    m_settings.push_back(_vb);
    m_settings.push_back(_vl);

    _distortion->setValueChanged([this]() {
        this->_precalc();
    });
    _vb->setValueChanged([this]() {
        this->_precalc();
    });
    _vl->setValueChanged([this]() {
        this->_precalc();
    });

    initialize();
}

void DiodeNode::setDistortion(float distortion)
{
    // We increase the distortion by increasing the gradient of the linear portion of the waveshaper's curve.
    _distortion->setFloat(distortion);
    _precalc();
}

void DiodeNode::_precalc()
{
    // The non-linear waveshaper curve describes the transformation between an input signal and an output signal.
    // Calculate a 1024-point curve following equation (2) from Parker's paper.

    float h = _distortion->valueFloat();

    const int samples = 1024;

    std::vector<float> wsCurve;
    wsCurve.resize(samples);

    float vb_ = _vb->valueFloat();
    float vl_ = _vl->valueFloat();

    float s = float(samples);
    for (int i = 0; i < samples; ++i)
    {
        // convert the index to a voltage of range -1 to 1
        float v = fabsf((float(i) - s / 2.0f) / (s / 2.0f));
        if (v <= vb_)
            v = 0;
        else if ((vb_ < v) && (v <= vl_))
            v = h * powf(v - vb_, 2.0f) / (2 * vl_ - 2.0f * vb_);
        else
            v = h * v - h * vl_ + (h * ((powf(vl_ - vb_, 2.0f)) / (2.0f * vl_ - 2.0f * vb_)));
        wsCurve[i] = v;
    }

    setCurve(wsCurve);
}

}  // lab
