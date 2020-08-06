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
#ifndef __D3D11MULTIRENDERTARGET_H__
#define __D3D11MULTIRENDERTARGET_H__

#include "OgreD3D11Prerequisites.h"
#include "OgreRenderTexture.h"

namespace Ogre {
    class _OgreD3D11Export D3D11MultiRenderTarget : public MultiRenderTarget
    {
    public:
        D3D11MultiRenderTarget(const String &name);
        ~D3D11MultiRenderTarget();

        virtual void getCustomAttribute( const String& name, void *pData );

        bool requiresTextureFlipping() const { return false; }
    private:
        v1::D3D11HardwarePixelBuffer *targets[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        ID3D11RenderTargetView* mRenderTargetViews[OGRE_MAX_MULTIPLE_RENDER_TARGETS];   // Store views to accelerate bind
        uint mNumberOfViews;                                                            // Store number of views to accelerate bind

        virtual void bindSurfaceImpl(size_t attachment, RenderTexture *target);
        virtual void unbindSurfaceImpl(size_t attachment);

        /** Check surfaces and update RenderTarget extent */
        void checkAndUpdate();

        Ogre::RenderTexture*    mRenderTargets[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
    };
};

#endif
