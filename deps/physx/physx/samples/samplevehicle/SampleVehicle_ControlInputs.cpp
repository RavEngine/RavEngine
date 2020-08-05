//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
//  * Neither the name of NVIDIA CORPORATION nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright (c) 2008-2019 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.  

#include "SampleVehicle_ControlInputs.h"
#include "PhysXSampleApplication.h"

using namespace SampleRenderer;
using namespace SampleFramework;

///////////////////////////////////////////////////////////////////////////////

SampleVehicle_ControlInputs::SampleVehicle_ControlInputs()
:	mCameraRotateInputY			(0.0f),
	mCameraRotateInputZ			(0.0f),
	mAccelKeyPressed			(false),
	mGearUpKeyPressed			(false),
	mGearDownKeyPressed			(false),
	mBrakeKeyPressed			(false),
	mHandbrakeKeyPressed		(false),
	mSteerLeftKeyPressed		(false),
	mSteerRightKeyPressed		(false),
	mBrakeLeftKeyPressed		(false),
	mBrakeRightKeyPressed		(false),
	mThrustLeftKeyPressed		(false),
	mThrustRightKeyPressed		(false),
	mAccel						(0.0f),
	mGearup						(false),
	mGeardown					(false),
	mBrake						(0.0f),
	mSteer						(0.0f),
	mHandbrake					(false),
	mThrustLeft					(0.0f),
	mThrustRight				(0.0f),
	mBrakeLeft					(0.0f),
	mBrakeRight					(0.0f)
{
}

SampleVehicle_ControlInputs::~SampleVehicle_ControlInputs()
{
}

