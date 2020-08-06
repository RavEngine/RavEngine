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

#ifndef __GL3PlusTextureBuffer_H__
#define __GL3PlusTextureBuffer_H__

#include "OgreGL3PlusHardwarePixelBuffer.h"

namespace Ogre {
namespace v1 {
    /** Texture surface.
     */
    class _OgreGL3PlusExport GL3PlusTextureBuffer: public GL3PlusHardwarePixelBuffer
    {
    public:
        /** Texture constructor */
        GL3PlusTextureBuffer(const String &baseName, GLenum target, GLuint id,
                             GLint face, GLint level, Usage usage,
                             bool writeGamma, uint fsaa);
        ~GL3PlusTextureBuffer();

        /// @copydoc HardwarePixelBuffer::bindToFramebuffer
        virtual void bindToFramebuffer(GLenum attachment, uint32 zoffset);

        /// @copydoc HardwarePixelBuffer::getRenderTarget
        RenderTexture* getRenderTarget(size_t);

        /// Upload a box of pixels to this buffer on the card.
        virtual void upload(const PixelBox &data, const Box &dest);

        /// Download a box of pixels from the card.
        virtual void download(const PixelBox &data);

        /// Hardware implementation of blitFromMemory.
        virtual void blitFromMemory(const PixelBox &src_orig, const Box &dstBox);

        /// Notify TextureBuffer of destruction of render target.
        void _clearSliceRTT(size_t zoffset)
        {
            mSliceTRT[zoffset] = 0;
        }

        /// Copy from framebuffer.
        void copyFromFramebuffer(uint32 zoffset);

        /// @copydoc HardwarePixelBuffer::blit
        void blit(const HardwarePixelBufferSharedPtr &src,
                  const Box &srcBox, const Box &dstBox);
        // Blitting implementation
        void blitFromTexture(GL3PlusTextureBuffer *src,
                             const Box &srcBox, const Box &dstBox);

        GLenum getGlTarget(void) const          { return mTarget; }
        GLuint getGlTextureId(void) const       { return mTextureID; }

    protected:
        // In case this is a texture level.
        GLenum mTarget;
        // Same as mTarget in case of GL_TEXTURE_xD, but cubemap face
        // for cubemaps.
        GLenum mFaceTarget;
        GLuint mTextureID;
        GLuint mBufferId;
        GLint mFace;
        GLint mLevel;

        typedef vector<RenderTexture*>::type SliceTRT;
        SliceTRT mSliceTRT;

        void _bindToFramebuffer(GLenum attachment, uint32 zoffset, GLenum which);
    };

}
}

#endif
