/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#include "OgreStableHeaders.h"
#include "OgreHardwareCounterBuffer.h"
#include "OgreHardwareBufferManager.h"
#include "OgreDefaultHardwareBufferManager.h"

namespace Ogre {
namespace v1 {

    HardwareCounterBuffer::HardwareCounterBuffer(HardwareBufferManagerBase* mgr, size_t sizeBytes, 
                                    HardwareBuffer::Usage usage, bool useShadowBuffer, const String& name)
        : HardwareBuffer(usage, false, useShadowBuffer)
        , mMgr(mgr), mName(name)
    {
        // Calculate the size of the vertices
        mSizeInBytes = sizeBytes;

        // Create a shadow buffer if required
        if (mUseShadowBuffer)
        {
            mShadowBuffer = OGRE_NEW DefaultHardwareCounterBuffer(mMgr, sizeBytes, HardwareBuffer::HBU_DYNAMIC, false);
        }
    }
    
    HardwareCounterBuffer::~HardwareCounterBuffer()
    {
        if (mMgr)
        {
            mMgr->_notifyCounterBufferDestroyed(this);
        }
        OGRE_DELETE mShadowBuffer;
    }

    //-----------------------------------------------------------------------------
    HardwareCounterBufferSharedPtr::HardwareCounterBufferSharedPtr(HardwareCounterBuffer* buf)
        : SharedPtr<HardwareCounterBuffer>(buf)
    {

    }
}
}
