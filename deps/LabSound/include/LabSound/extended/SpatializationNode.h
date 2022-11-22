// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#pragma once

#ifndef SPATIALIZATION_NODE_H
#define SPATIALIZATION_NODE_H

#include "LabSound/core/PannerNode.h"
#include <map>

namespace lab
{

struct Occluder
{
    float x, y, z;
    float innerRadius;
    float outerRadius;
    float maxAttenuation;

    Occluder() { }

    Occluder(float x, float y, float z, float radius)
        : x(x)
        , y(y)
        , z(z)
        , innerRadius(radius * 0.75f)
        , outerRadius(radius)
        , maxAttenuation(0)
    {
    }

    Occluder(const Occluder & rhs)
        : x(rhs.x)
        , y(rhs.y)
        , z(rhs.z)
        , innerRadius(rhs.innerRadius)
        , outerRadius(rhs.outerRadius)
        , maxAttenuation(rhs.maxAttenuation)
    {
    }

    Occluder & operator=(const Occluder & rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        innerRadius = rhs.innerRadius;
        outerRadius = rhs.outerRadius;
        maxAttenuation = rhs.maxAttenuation;
        return *this;
    }
};

class Occluders
{
    std::map<int, Occluder> occluders;

public:
    void setOccluder(int id, float x, float y, float z, float radius);
    void removeOccluder(int id);
    float occlusion(const FloatPoint3D & sourcePos, const FloatPoint3D & listenerPos) const;
};

typedef std::shared_ptr<Occluders> OccludersPtr;

class SpatializationNode : public PannerNode
{
    virtual float distanceConeGain(ContextRenderLock & r);
    std::shared_ptr<Occluders> occluders;

public:
    SpatializationNode(AudioContext & ac);
    virtual ~SpatializationNode() = default;
    void setOccluders(OccludersPtr ptr);
};
}

#endif
