// License: BSD 2 Clause
// Copyright (C) 2018, The LabSound Authors. All rights reserved.

#include "LabSound/core/AudioListener.h"

namespace lab
{
AudioListener::AudioListener()
    : m_dopplerFactor(std::make_shared<AudioParam>("dopplerFactor", "DPLR", 1.f, 0.01f, 100.f))
    , m_speedOfSound(std::make_shared<AudioParam>("speedOfSound", "SPED", 343.f, 1.f, 10000.f))
    , m_forwardX(std::make_shared<AudioParam>("forwardX", "FWDX", 0.f, -1.e6f, 1.e6f))
    , m_forwardY(std::make_shared<AudioParam>("forwardY", "FWDY", 0.f, -1.e6f, 1.e6f))
    , m_forwardZ(std::make_shared<AudioParam>("forwardZ", "FWDZ", -1.f, -1.e6f, 1.e6f))
    , m_upX(std::make_shared<AudioParam>("upX", "UP X", 0.f, -1.f, 1.f))
    , m_upY(std::make_shared<AudioParam>("upY", "UP Y", 1.f, -1.f, 1.f))
    , m_upZ(std::make_shared<AudioParam>("upZ", "UP Z", 0.f, -1.f, 1.f))
    , m_velocityX(std::make_shared<AudioParam>("velocityX", "VELX", 0.f, -1000.f, 1000.f))
    , m_velocityY(std::make_shared<AudioParam>("velocityY", "VELY", 0.f, -1000.f, 1000.f))
    , m_velocityZ(std::make_shared<AudioParam>("velocityZ", "VELZ", 0.f, -1000.f, 1000.f))
    , m_positionX(std::make_shared<AudioParam>("positionX", "POSX", 0.f, -1.e6f, 1.e6f))
    , m_positionY(std::make_shared<AudioParam>("positionY", "POSY", 0.f, -1.e6f, 1.e6f))
    , m_positionZ(std::make_shared<AudioParam>("positionZ", "POSZ", 0.f, -1.e6f, 1.e6f))
{
}

void AudioListener::setForward(const FloatPoint3D & fwd)
{
    m_forwardX->setValue(fwd.x);
    m_forwardY->setValue(fwd.y);
    m_forwardZ->setValue(fwd.z);
}

void AudioListener::setUpVector(const FloatPoint3D & upVector)
{
    m_upX->setValue(upVector.x);
    m_upY->setValue(upVector.y);
    m_upZ->setValue(upVector.z);
}

void AudioListener::setPosition(const FloatPoint3D & position)
{
    m_positionX->setValue(position.x);
    m_positionY->setValue(position.y);
    m_positionZ->setValue(position.z);
}

void AudioListener::setVelocity(const FloatPoint3D & velocity)
{
    m_velocityX->setValue(velocity.x);
    m_velocityY->setValue(velocity.y);
    m_velocityZ->setValue(velocity.z);
}
}
