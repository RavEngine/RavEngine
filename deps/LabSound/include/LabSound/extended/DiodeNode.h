// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef DIODE_NODE_H
#define DIODE_NODE_H

#include "LabSound/core/WaveShaperNode.h"

namespace lab
{

// The diode node, used in conjunction with a gain node, will modulate
// a source so that it sounds like audio being over-driven through a vacuum tube diode.
//
//  source -+-> diode ----> gain_level
//          |               |
//          +-------------> gain ---------------> outpuot
//
// params:
// settings: distortion, vb, vl
//
class DiodeNode : public WaveShaperNode
{
    void _precalc();

    std::shared_ptr<WaveShaperNode> waveShaper;
    std::shared_ptr<AudioSetting> _distortion;
    std::shared_ptr<AudioSetting> _vb;  // curve shape control
    std::shared_ptr<AudioSetting> _vl;  // curve shape control

public:
    DiodeNode(AudioContext &);
    virtual ~DiodeNode() = default;

    static const char* static_name() { return "Diode"; }
    virtual const char* name() const override { return static_name(); }

    void setDistortion(float distortion = 1.0);

    std::shared_ptr<AudioSetting> distortion() const { return _distortion; };
    std::shared_ptr<AudioSetting> vb() const { return _vb; };
    std::shared_ptr<AudioSetting> vl() const { return _vl; }
};
}

#endif
