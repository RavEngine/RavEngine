// License: BSD 2 Clause
// Copyright (C) 2018, The LabSound Authors. All rights reserved.

#ifndef AudioListener_h
#define AudioListener_h

#include "LabSound/core/AudioParam.h"
#include "LabSound/core/FloatPoint3D.h"
#include <memory>

namespace lab
{
class AudioParam;

// AudioListener maintains the state of the listener in the audio scene as defined in the OpenAL specification.
class AudioListener
{
    std::shared_ptr<AudioParam> m_dopplerFactor;
    std::shared_ptr<AudioParam> m_speedOfSound;
    std::shared_ptr<AudioParam> m_forwardX;
    std::shared_ptr<AudioParam> m_forwardY;
    std::shared_ptr<AudioParam> m_forwardZ;
    std::shared_ptr<AudioParam> m_upX;
    std::shared_ptr<AudioParam> m_upY;
    std::shared_ptr<AudioParam> m_upZ;
    std::shared_ptr<AudioParam> m_velocityX;
    std::shared_ptr<AudioParam> m_velocityY;
    std::shared_ptr<AudioParam> m_velocityZ;
    std::shared_ptr<AudioParam> m_positionX;
    std::shared_ptr<AudioParam> m_positionY;
    std::shared_ptr<AudioParam> m_positionZ;

public:
    AudioListener();
    ~AudioListener() = default;

    // Position
    void setPosition(float x, float y, float z) { setPosition({x, y, z}); }
    void setPosition(const FloatPoint3D & position);

    std::shared_ptr<AudioParam> positionX() const { return m_positionX; }
    std::shared_ptr<AudioParam> positionY() const { return m_positionY; }
    std::shared_ptr<AudioParam> positionZ() const { return m_positionZ; }

    // Orientation
    void setOrientation(float x, float y, float z, float upX, float upY, float upZ)
    {
        setForward(FloatPoint3D(x, y, z));
        setUpVector(FloatPoint3D(upX, upY, upZ));
    }

    // Forward represents the horizontal position of the listener's forward
    // direction in the same cartesian coordinate sytem as the position
    // values. The forward and up values are linearly independent of each other.
    void setForward(const FloatPoint3D & fwd);
    std::shared_ptr<AudioParam> forwardX() const { return m_forwardX; }
    std::shared_ptr<AudioParam> forwardY() const { return m_forwardY; }
    std::shared_ptr<AudioParam> forwardZ() const { return m_forwardZ; }

    // Up-vector
    void setUpVector(const FloatPoint3D & upVector);
    std::shared_ptr<AudioParam> upX() const { return m_upX; }
    std::shared_ptr<AudioParam> upY() const { return m_upY; }
    std::shared_ptr<AudioParam> upZ() const { return m_upZ; }

    // Velocity
    void setVelocity(float x, float y, float z) { setVelocity(FloatPoint3D(x, y, z)); }
    void setVelocity(const FloatPoint3D & velocity);
    std::shared_ptr<AudioParam> velocityX() const { return m_velocityX; }
    std::shared_ptr<AudioParam> velocityY() const { return m_velocityY; }
    std::shared_ptr<AudioParam> velocityZ() const { return m_velocityZ; }

    // Doppler factor
    void setDopplerFactor(float dopplerFactor) { m_dopplerFactor->setValue(dopplerFactor); }
    std::shared_ptr<AudioParam> dopplerFactor() const { return m_dopplerFactor; }

    // Speed of sound
    void setSpeedOfSound(float speedOfSound) { m_speedOfSound->setValue(speedOfSound); }
    std::shared_ptr<AudioParam> speedOfSound() const { return m_speedOfSound; }
};

}  // lab

#endif  // AudioListener_h
