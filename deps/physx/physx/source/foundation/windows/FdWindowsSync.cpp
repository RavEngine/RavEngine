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
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ''AS IS'' AND ANY
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
// Copyright (c) 2008-2025 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.

#include "foundation/windows/PxWindowsInclude.h"
#include "foundation/PxUserAllocated.h"
#include "foundation/PxSync.h"

using namespace physx;

static PX_FORCE_INLINE HANDLE& getSync(PxSyncImpl* impl)
{
	return *reinterpret_cast<HANDLE*>(impl);
}

uint32_t PxSyncImpl::getSize()
{
	return sizeof(HANDLE);
}

PxSyncImpl::PxSyncImpl()
{
	getSync(this) = CreateEvent(0, true, false, 0);
}

PxSyncImpl::~PxSyncImpl()
{
	CloseHandle(getSync(this));
}

void PxSyncImpl::reset()
{
	ResetEvent(getSync(this));
}

void PxSyncImpl::set()
{
	SetEvent(getSync(this));
}

bool PxSyncImpl::wait(uint32_t milliseconds)
{
	if(milliseconds == -1)
		milliseconds = INFINITE;

	return WaitForSingleObject(getSync(this), milliseconds) == WAIT_OBJECT_0;
}

