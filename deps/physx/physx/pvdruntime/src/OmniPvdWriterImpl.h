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

#ifndef OMNI_PVD_WRITER_IMPL_H
#define OMNI_PVD_WRITER_IMPL_H

#include "OmniPvdWriter.h"
#include "OmniPvdCommands.h"
#include "OmniPvdDefinesInternal.h"
#include "OmniPvdLog.h"

class OmniPvdWriterImpl : public OmniPvdWriter {
public:
	OmniPvdWriterImpl();
	~OmniPvdWriterImpl();
	void OMNI_PVD_CALL setLogFunction(OmniPvdLogFunction logFunction);
	void setVersionHelper();
	void setVersion(OmniPvdVersionType majorVersion, OmniPvdVersionType minorVersion, OmniPvdVersionType patch);
	void OMNI_PVD_CALL setWriteStream(OmniPvdWriteStream& stream);
	OmniPvdWriteStream* OMNI_PVD_CALL getWriteStream();

	OmniPvdClassHandle OMNI_PVD_CALL registerClass(const char* className, OmniPvdClassHandle baseClass);
	OmniPvdAttributeHandle OMNI_PVD_CALL registerEnumValue(OmniPvdClassHandle classHandle, const char* attributeName, OmniPvdEnumValueType value);
	OmniPvdAttributeHandle OMNI_PVD_CALL registerAttribute(OmniPvdClassHandle classHandle, const char* attributeName, OmniPvdDataType::Enum attributeDataType, uint32_t nbElements);
	OmniPvdAttributeHandle OMNI_PVD_CALL registerFlagsAttribute(OmniPvdClassHandle classHandle, const char* attributeName, OmniPvdClassHandle enumClassHandle);
	OmniPvdAttributeHandle OMNI_PVD_CALL registerClassAttribute(OmniPvdClassHandle classHandle, const char* attributeName, OmniPvdClassHandle classAttributeHandle);
	OmniPvdAttributeHandle OMNI_PVD_CALL registerUniqueListAttribute(OmniPvdClassHandle classHandle, const char* attributeName, OmniPvdDataType::Enum attributeDataType);
	void OMNI_PVD_CALL setAttribute(OmniPvdContextHandle contextHandle, OmniPvdObjectHandle objectHandle, const OmniPvdAttributeHandle* attributeHandles, uint8_t nbAttributeHandles, const uint8_t* data, uint32_t nbrBytes);
	
	void OMNI_PVD_CALL addToUniqueListAttribute(OmniPvdContextHandle contextHandle, OmniPvdObjectHandle objectHandle, const OmniPvdAttributeHandle* attributeHandles, uint8_t nbAttributeHandles, const uint8_t* data, uint32_t nbrBytes);

	void OMNI_PVD_CALL removeFromUniqueListAttribute(OmniPvdContextHandle contextHandle, OmniPvdObjectHandle objectHandle, const OmniPvdAttributeHandle* attributeHandles, uint8_t nbAttributeHandles, const uint8_t* data, uint32_t nbrBytes);

	void OMNI_PVD_CALL createObject(OmniPvdContextHandle contextHandle, OmniPvdClassHandle classHandle, OmniPvdObjectHandle objectHandle, const char* objectName);
	void OMNI_PVD_CALL destroyObject(OmniPvdContextHandle contextHandle, OmniPvdObjectHandle objectHandle);
	void OMNI_PVD_CALL startFrame(OmniPvdContextHandle contextHandle, uint64_t timeStamp);
	void OMNI_PVD_CALL stopFrame(OmniPvdContextHandle contextHandle, uint64_t timeStamp);

	void OMNI_PVD_CALL recordMessage(OmniPvdContextHandle contextHandle, const char* message, const char* file, uint32_t line, uint32_t type, OmniPvdClassHandle handle) override;

	uint32_t OMNI_PVD_CALL getStatus();
	void OMNI_PVD_CALL clearStatus();

	void resetParams();

	bool isFlagOn(OmniPvdWriterStatusFlag::Enum flagBitMask)
	{
		return mStatusFlags & uint32_t(flagBitMask);
	}
	
	void setFlagOn(OmniPvdWriterStatusFlag::Enum flagBitMask)
	{
		mStatusFlags = mStatusFlags | uint32_t(flagBitMask);
	}

	void setFlagOff(OmniPvdWriterStatusFlag::Enum flagBitMask)
	{
		mStatusFlags = mStatusFlags & ~uint32_t(flagBitMask);
	}

	void setFlagVal(OmniPvdWriterStatusFlag::Enum flagBitMask, bool value)
	{
		if (value) 
		{
			setFlagOn(flagBitMask);
		}
		else
		{
			setFlagOff(flagBitMask);
		}
	}

	void writeWithStatus(const uint8_t* writePtr, uint64_t nbrBytesToWrite) 
	{
		if (! (mStream && writePtr && (nbrBytesToWrite > 0)) ) return;
		uint64_t nbrBytesWritten = mStream->writeBytes(writePtr, nbrBytesToWrite);
		const bool writeFailure = nbrBytesWritten != nbrBytesToWrite;
		if (writeFailure) {
			setFlagOn(OmniPvdWriterStatusFlag::eSTREAM_WRITE_FAILURE);
		}
	}
	
	void writeDataType(OmniPvdDataType::Enum attributeDataType)
	{
		const OmniPvdDataTypeStorageType dataType = static_cast<OmniPvdDataTypeStorageType>(attributeDataType);
		writeWithStatus((const uint8_t*)&dataType, sizeof(OmniPvdDataTypeStorageType));
	}
	
	void writeCommand(OmniPvdCommand::Enum command)
	{
		const OmniPvdCommandStorageType commandTmp = static_cast<OmniPvdCommandStorageType>(command);
		writeWithStatus((const uint8_t*)&commandTmp, sizeof(OmniPvdCommandStorageType));
	}

	bool mIsFirstWrite;
	OmniPvdLog mLog;
	OmniPvdWriteStream* mStream;
	int mLastClassHandle;
	int mLastAttributeHandle;

	uint32_t mStatusFlags;
};

#endif
