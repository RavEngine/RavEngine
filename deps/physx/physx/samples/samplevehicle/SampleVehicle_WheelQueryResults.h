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

#ifndef SAMPLEVEHICLE_WHEELQUERYRESULTS_H
#define SAMPLEVEHICLE_WHEELQUERYRESULTS_H

#include "vehicle/PxVehicleSDK.h"
#include "vehicle/PxVehicleUpdate.h"

using namespace physx;

//Data structure to store reports for each wheel. 
class SampleVehicleWheelQueryResults
{
public:

	//Allocate a buffer of wheel query results for up to maxNumWheels.
	static SampleVehicleWheelQueryResults* allocate(const PxU32 maxNumWheels);

	//Free allocated buffer.
	void free();

	PxWheelQueryResult* addVehicle(const PxU32 numWheels);

private:

	//One result for each wheel.
	PxWheelQueryResult* mWheelQueryResults;

	//Maximum number of wheels.
	PxU32 mMaxNumWheels;

	//Number of wheels 
	PxU32 mNumWheels;


	SampleVehicleWheelQueryResults()
		: mWheelQueryResults(NULL),mMaxNumWheels(0), mNumWheels(0)
	{
		init();
	}

	~SampleVehicleWheelQueryResults()
	{
	}

	void init()
	{
		mWheelQueryResults=NULL;
		mMaxNumWheels=0;
		mNumWheels=0;
	}
};


#endif //SAMPLEVEHICLE_WHEELQUERYRESULTS_H
