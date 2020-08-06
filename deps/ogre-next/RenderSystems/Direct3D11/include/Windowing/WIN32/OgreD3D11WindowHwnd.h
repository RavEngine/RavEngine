/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2017 Torus Knot Software Ltd

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

#ifndef _OgreD3D11WindowHwnd_H_
#define _OgreD3D11WindowHwnd_H_

#include "OgreD3D11Window.h"

namespace Ogre
{
#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

    class D3D11WindowHwnd : public D3D11WindowSwapChainBased
    {
    protected:
        HWND    mHwnd;                          // Win32 window handle
        DWORD   mWindowedWinStyle;              // Windowed mode window style flags.
        DWORD   mFullscreenWinStyle;            // Fullscreen mode window style flags.
        // the last value of the switching fullscreen counter when we switched
        int     mLastSwitchingFullscreenCounter;

        static bool mClassRegistered;

    protected:
        virtual PixelFormatGpu _getRenderFormat() { return mHwGamma ? PFG_RGBA8_UNORM_SRGB : PFG_RGBA8_UNORM; } // DXGI 1.0 compatible

        DWORD getWindowStyle( bool fullScreen ) const;

        static BOOL CALLBACK createMonitorsInfoEnumProc( HMONITOR hMonitor, HDC hdcMonitor,
                                                         LPRECT lprcMonitor, LPARAM dwData );

        void updateWindowRect(void);
        void adjustWindow( uint32 clientWidth, uint32 clientHeight,
                           uint32 *outDrawableWidth, uint32 *outDrawableHeight );

        template <typename T>
        void setCommonSwapChain( T &sd );
        virtual HRESULT _createSwapChainImpl();


        void create( bool fullscreenMode, const NameValuePairList *miscParams );

    public:
        D3D11WindowHwnd( const String &title, uint32 width, uint32 height,
                         bool fullscreenMode, PixelFormatGpu depthStencilFormat,
                         const NameValuePairList *miscParams,
                         D3D11Device &device, D3D11RenderSystem *renderSystem );
        virtual ~D3D11WindowHwnd();

        virtual void _initialize( TextureGpuManager *textureGpuManager );
        virtual void destroy(void);

        virtual void reposition( int32 left, int32 top );
        virtual void requestResolution( uint32 width, uint32 height );
        virtual void requestFullscreenSwitch( bool goFullscreen, bool borderless, uint32 monitorIdx,
                                              uint32 width, uint32 height,
                                              uint32 frequencyNumerator, uint32 frequencyDenominator );
        virtual void windowMovedOrResized(void);

        virtual bool isVisible(void) const;
        virtual void setHidden( bool hidden );

        virtual void getCustomAttribute( IdString name, void* pData );
    };
#endif
}

#endif
