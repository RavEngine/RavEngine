/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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

#ifndef __OSXCocoaWindow_H__
#define __OSXCocoaWindow_H__

#include "OgreOSXCocoaContext.h"

#include <AppKit/NSWindow.h>
#include <QuartzCore/CVDisplayLink.h>
#include "OgreOSXCocoaView.h"
#include "OgreOSXCocoaWindowDelegate.h"

@class CocoaWindowDelegate;

@interface OgreGL3PlusWindow : NSWindow

@end

namespace Ogre {
    class _OgreGL3PlusExport CocoaWindow : public RenderWindow
    {
    private:
        NSWindow *mWindow;
        NSView *mView;
        NSOpenGLContext *mGLContext;
        NSOpenGLPixelFormat *mGLPixelFormat;
        CVDisplayLinkRef mDisplayLink;
        NSPoint mWindowOriginPt;
        CocoaWindowDelegate *mWindowDelegate;
        CocoaContext* mContext;

        bool mActive;
        bool mClosed;
        bool mHidden;
        bool mVSync;
        bool mHasResized;
        bool mIsExternal;
        String mWindowTitle;
        bool mUseOgreGLView;
        float mContentScalingFactor;
        
        int _getPixelFromPoint(int viewPt) const;
        void _setWindowParameters(unsigned int widthPt, unsigned int heightPt);
        
    public:
        CocoaWindow();
        ~CocoaWindow();
        
        NSView* ogreView() const { return mView; };
        NSWindow* ogreWindow() const { return mWindow; };
        NSOpenGLContext* nsopenGLContext() const { return mGLContext; };
        void createWithView(OgreGL3PlusView *view);

        /** @copydoc see RenderWindow::getViewPointToPixelScale */
        float getViewPointToPixelScale();
        /** Overridden - see RenderWindow */
        void create(const String& name, unsigned int widthPt, unsigned int heightPt,
                bool fullScreen, const NameValuePairList *miscParams);
        /** Overridden - see RenderWindow */
        void destroy(void);
        /** Overridden - see RenderWindow */
        bool isActive(void) const;
        /** Overridden - see RenderWindow */
        bool isClosed(void) const;
        /** @copydoc see RenderWindow::isHidden */
        bool isHidden(void) const { return mHidden; }
        /** @copydoc see RenderWindow::setHidden */
        void setHidden(bool hidden);
        /** @copydoc see RenderWindow::setVSyncEnabled */
        void setVSyncEnabled(bool vsync);
        /** @copydoc see RenderWindow::isVSyncEnabled */
        bool isVSyncEnabled() const;
        /** Overridden - see RenderWindow */
        void reposition(int leftPt, int topPt);
        /** Overridden - see RenderWindow */
        void resize(unsigned int widthPt, unsigned int heightPt);
        /** Overridden - see RenderWindow */
        void swapBuffers();
        /** Overridden - see RenderTarget */
        virtual void copyContentsToMemory(const Box& src, const PixelBox &dst, FrameBuffer buffer);
        /** Overridden - see RenderWindow */
        virtual void setFullscreen(bool fullScreen, unsigned int widthPt, unsigned int heightPt);
        /** Overridden - see RenderWindow */
        virtual unsigned int getWidth(void) const;
        /** Overridden - see RenderWindow */
        virtual unsigned int getHeight(void) const;
        /** Overridden - see RenderWindow */
        void windowMovedOrResized(void);
        void windowHasResized(void);
        void createNewWindow(unsigned int width, unsigned int height, String title);
        void createWindowFromExternal(NSView *viewRef);

        bool requiresTextureFlipping() const { return false; }      
        void getCustomAttribute( const String& name, void* pData );
    };
}

#endif

