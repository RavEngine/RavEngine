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

#include "OgreRenderTexture.h"
#include "OgreException.h"
#include "OgreHardwarePixelBuffer.h"

namespace Ogre
{

    //-----------------------------------------------------------------------------
    RenderTexture::RenderTexture(v1::HardwarePixelBuffer *buffer, uint32 zoffset):
        mBuffer(buffer), mZOffset(zoffset)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
        mWidth = mBuffer->getWidth();
        mHeight = mBuffer->getHeight();
        mFormat = mBuffer->getFormat();
    }
    RenderTexture::~RenderTexture()
    {
        mBuffer->_clearSliceRTT(0);
    }

    void RenderTexture::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        if (buffer == FB_AUTO) buffer = FB_FRONT;
        if (buffer != FB_FRONT)
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                        "Invalid buffer.",
                        "RenderTexture::copyContentsToMemory" );
        }

        mBuffer->blitToMemory(src, dst);
    }
    //---------------------------------------------------------------------
    PixelFormat RenderTexture::suggestPixelFormat() const
    {
        return mBuffer->getFormat();
    }
    //-----------------------------------------------------------------------------
    MultiRenderTarget::MultiRenderTarget(const String &name)
    {
        mPriority = OGRE_REND_TO_TEX_RT_GROUP;
        mName = name;
        /// Width and height is unknown with no targets attached
        mWidth = mHeight = 0;
    }
    //-----------------------------------------------------------------------------
    void MultiRenderTarget::copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer)
    {
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                    "Cannot get MultiRenderTargets pixels",
                    "MultiRenderTarget::copyContentsToMemory");
    }
    //-----------------------------------------------------------------------------
    void MultiRenderTarget::setFsaaResolveDirty(void)
    {
        BoundSufaceList::const_iterator itor = mBoundSurfaces.begin();
        BoundSufaceList::const_iterator end  = mBoundSurfaces.end();

        while( itor != end )
        {
            (*itor)->setFsaaResolveDirty();
            ++itor;
        }

        RenderTarget::setFsaaResolveDirty();
    }
    //-----------------------------------------------------------------------------
    void MultiRenderTarget::setFsaaResolved()
    {
        BoundSufaceList::const_iterator itor = mBoundSurfaces.begin();
        BoundSufaceList::const_iterator end  = mBoundSurfaces.end();

        while( itor != end )
        {
            (*itor)->setFsaaResolved();
            ++itor;
        }

        RenderTarget::setFsaaResolved();
    }
    //-----------------------------------------------------------------------------
    void MultiRenderTarget::swapBuffers(void)
    {
        BoundSufaceList::const_iterator itor = mBoundSurfaces.begin();
        BoundSufaceList::const_iterator end  = mBoundSurfaces.end();

        while( itor != end )
        {
            (*itor)->swapBuffers();
            ++itor;
        }

        RenderTarget::swapBuffers();
    }
    //-----------------------------------------------------------------------
    void MultiRenderTarget::getFormatsForPso( PixelFormat outFormats[OGRE_MAX_MULTIPLE_RENDER_TARGETS],
                                              bool outHwGamma[OGRE_MAX_MULTIPLE_RENDER_TARGETS] ) const
    {
        BoundSufaceList::const_iterator itor = mBoundSurfaces.begin();
        BoundSufaceList::const_iterator end  = mBoundSurfaces.end();

        size_t idx = 0;
        while( itor != end )
        {
            outFormats[idx] = (*itor)->getFormat();
            outHwGamma[idx] = (*itor)->isHardwareGammaEnabled();
            ++idx;
            ++itor;
        }

        for( size_t i=mBoundSurfaces.size(); i<OGRE_MAX_MULTIPLE_RENDER_TARGETS; ++i )
        {
            outFormats[i] = PF_NULL;
            outHwGamma[i] = false;
        }
    }
}
