// SPDX-License-Identifier: BSD-2-Clause
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef PannerNode_h
#define PannerNode_h

#include "LabSound/core/AudioListener.h"
#include "LabSound/core/AudioNode.h"
#include "LabSound/core/AudioParam.h"
#include "LabSound/core/FloatPoint3D.h"
#include "LabSound/core/Macros.h"

namespace lab
{

// PannerNode is an AudioNode with one input and one output.
// It positions a sound in 3D space, with the exact effect dependent on the panning model.
// It has a position and an orientation in 3D space which is relative to the position and orientation of the context's AudioListener.
// A distance effect will attenuate the gain as the position moves away from the listener.
// A cone effect will attenuate the gain as the orientation moves away from the listener.
// All of these effects follow the OpenAL specification very closely.

class AudioBus;
class ConeEffect;
class DistanceEffect;
class HRTFDatabaseLoader;
class Panner;

// params: orientation[XYZ], velocity[XYZ], position[XYZ]
// settings: distanceModel, refDistance, maxDistance, rolloffFactor,
//           coneKInnerAngle, coneOuterAngle, panningMode
//
class PannerNode : public AudioNode
{
    std::shared_ptr<AudioParam> m_orientationX;
    std::shared_ptr<AudioParam> m_orientationY;
    std::shared_ptr<AudioParam> m_orientationZ;
    std::shared_ptr<AudioParam> m_velocityX;
    std::shared_ptr<AudioParam> m_velocityY;
    std::shared_ptr<AudioParam> m_velocityZ;
    std::shared_ptr<AudioParam> m_positionX;
    std::shared_ptr<AudioParam> m_positionY;
    std::shared_ptr<AudioParam> m_positionZ;

    std::shared_ptr<AudioParam> m_distanceGain;
    std::shared_ptr<AudioParam> m_coneGain;

    std::shared_ptr<AudioSetting> m_distanceModel;
    std::shared_ptr<AudioSetting> m_refDistance;
    std::shared_ptr<AudioSetting> m_maxDistance;
    std::shared_ptr<AudioSetting> m_rolloffFactor;
    std::shared_ptr<AudioSetting> m_coneInnerAngle;
    std::shared_ptr<AudioSetting> m_coneOuterAngle;
    std::shared_ptr<AudioSetting> m_panningModel;

public:
    enum DistanceModel
    {
        LINEAR_DISTANCE = 0,
        INVERSE_DISTANCE = 1,
        EXPONENTIAL_DISTANCE = 2,
    };

    PannerNode(AudioContext & ac, const std::string & hrtf_root_dir_path = "");
    virtual ~PannerNode();

    static const char* static_name() { return "Panner"; }
    virtual const char* name() const override { return static_name(); }

    // AudioNode
    virtual void process(ContextRenderLock &, int bufferSize) override;
    virtual void reset(ContextRenderLock &) override;
    virtual void initialize() override;
    virtual void uninitialize() override;

    // Panning model
    PanningMode panningModel() const;
    void setPanningModel(PanningMode m);

    // Position
    void setPosition(float x, float y, float z) { setPosition(FloatPoint3D(x, y, z)); }
    void setPosition(const FloatPoint3D & position);

    std::shared_ptr<AudioParam> positionX() const { return m_positionX; }
    std::shared_ptr<AudioParam> positionY() const { return m_positionY; }
    std::shared_ptr<AudioParam> positionZ() const { return m_positionZ; }

    // The orientation property indicates the X component of the direction in
    // which the audio source is facing, in cartesian space. The complete
    // vector is defined by the position of the audio source, and the direction
    // in which it is facing.
    void setOrientation(const FloatPoint3D & fwd);
    std::shared_ptr<AudioParam> orientationX() const { return m_orientationX; }
    std::shared_ptr<AudioParam> orientationY() const { return m_orientationY; }
    std::shared_ptr<AudioParam> orientationZ() const { return m_orientationZ; }

    // Velocity
    void setVelocity(float x, float y, float z) { setVelocity(FloatPoint3D(x, y, z)); }
    void setVelocity(const FloatPoint3D & velocity);
    std::shared_ptr<AudioParam> velocityX() const { return m_velocityX; }
    std::shared_ptr<AudioParam> velocityY() const { return m_velocityY; }
    std::shared_ptr<AudioParam> velocityZ() const { return m_velocityZ; }

    // Distance parameters
    DistanceModel distanceModel();
    void setDistanceModel(DistanceModel);

    float refDistance();
    void setRefDistance(float refDistance);

    float maxDistance();
    void setMaxDistance(float maxDistance);

    float rolloffFactor();
    void setRolloffFactor(float rolloffFactor);

    // Sound cones - angles in degrees
    float coneInnerAngle() const;
    void setConeInnerAngle(float angle);

    float coneOuterAngle() const;
    void setConeOuterAngle(float angle);

    float coneOuterGain() const;
    void setConeOuterGain(float angle);

    void getAzimuthElevation(ContextRenderLock & r, double * outAzimuth, double * outElevation);
    float dopplerRate(ContextRenderLock & r);

    // Accessors for dynamically calculated gain values.
    std::shared_ptr<AudioParam> distanceGain() { return m_distanceGain; }
    std::shared_ptr<AudioParam> coneGain() { return m_coneGain; }

    virtual double tailTime(ContextRenderLock & r) const override;
    virtual double latencyTime(ContextRenderLock & r) const override;

protected:
    std::shared_ptr<HRTFDatabaseLoader> m_hrtfDatabaseLoader;

    // Returns the combined distance and cone gain attenuation.
    virtual float distanceConeGain(ContextRenderLock & r);

    // Notifies any SampledAudioNodes connected to us either directly or indirectly about our existence.
    // This is in order to handle the pitch change necessary for the doppler shift.
    // @tofix - broken?
    void notifyAudioSourcesConnectedToNode(ContextRenderLock & r, AudioNode *);

    std::unique_ptr<Panner> m_panner;
    std::unique_ptr<DistanceEffect> m_distanceEffect;
    std::unique_ptr<ConeEffect> m_coneEffect;

    float m_lastGain = -1.0f;
    float m_sampleRate;
};

}  // namespace lab

#endif  // PannerNode_h
