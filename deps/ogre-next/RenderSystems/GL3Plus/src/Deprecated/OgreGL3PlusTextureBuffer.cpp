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

#include "OgreRenderSystem.h"
#include "OgreRoot.h"

#include "OgreGL3PlusHardwareBufferManager.h"
#include "OgreGL3PlusHardwarePixelBuffer.h"
#include "OgreGL3PlusTextureBuffer.h"
#include "OgreGL3PlusPixelFormat.h"
#include "OgreGL3PlusFBORenderTexture.h"

#include "OgreGLSLMonolithicProgram.h"
#include "OgreGLSLMonolithicProgramManager.h"
#include "OgreGLSLSeparableProgram.h"
#include "OgreGLSLSeparableProgramManager.h"

namespace Ogre {
namespace v1 {

    GL3PlusTextureBuffer::GL3PlusTextureBuffer(const String &baseName, GLenum target, GLuint id,
                                               GLint face, GLint level, Usage usage,
                                               bool writeGamma, uint fsaa)
        : GL3PlusHardwarePixelBuffer(0, 0, 0, PF_UNKNOWN, writeGamma, usage),
          mTarget(target), mTextureID(id), mBufferId(0), mFace(face), mLevel(level), mSliceTRT(0)
    {
        // devise mWidth, mHeight and mDepth and mFormat
        GLint value = 0;

        OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));

        // Get face identifier
        mFaceTarget = mTarget;
        if (mTarget == GL_TEXTURE_CUBE_MAP)
            mFaceTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;

        // Get width
        OGRE_CHECK_GL_ERROR(glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_WIDTH, &value));
        mWidth = value;

        // Get height
        if (mTarget == GL_TEXTURE_1D)
            value = 1;  // Height always 1 for 1D textures
        else
            OGRE_CHECK_GL_ERROR(glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_HEIGHT, &value));
        mHeight = value;

        // Get depth
        if (mTarget != GL_TEXTURE_3D && mTarget != GL_TEXTURE_2D_ARRAY)
            value = 1; // Depth always 1 for non-3D textures
        else
            OGRE_CHECK_GL_ERROR(glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_DEPTH, &value));
        mDepth = value;

        // Get format
        OGRE_CHECK_GL_ERROR(glGetTexLevelParameteriv(mFaceTarget, level, GL_TEXTURE_INTERNAL_FORMAT, &value));
        mGLInternalFormat = value;
        mFormat = GL3PlusPixelUtil::getClosestOGREFormat(value);

        // Default
        mRowPitch = mWidth;
        mSlicePitch = mHeight*mWidth;
        mSizeInBytes = PixelUtil::getMemorySize(mWidth, mHeight, mDepth, mFormat);

        // Log a message
        //        std::stringstream str;
        //        str << "GL3PlusHardwarePixelBuffer constructed for texture: " << mTextureID
        //            << " bytes: " << mSizeInBytes
        //            << " face: " << mFace << " level: " << mLevel
        //            << " width: " << mWidth << " height: "<< mHeight << " depth: " << mDepth
        //            << " format: " << PixelUtil::getFormatName(mFormat)
        //            << "(internal 0x" << std::hex << value << ")";
        //        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

        // Set up a pixel box
        mBuffer = PixelBox(mWidth, mHeight, mDepth, mFormat);

        if (mWidth==0 || mHeight==0 || mDepth==0)
            // We are invalid, do not allocate a buffer
            return;

        // Is this a render target?
        if (mUsage & TU_RENDERTARGET)
        {
            // Create render target for each slice
            mSliceTRT.reserve(mDepth);
            for(uint32 zoffset=0; zoffset<mDepth; ++zoffset)
            {
                String name;
                name = "rtt/" + StringConverter::toString((size_t)this) + "/" + baseName;
                GL3PlusSurfaceDesc surface;
                surface.buffer = this;
                surface.zoffset = zoffset;
                RenderTexture *trt = GL3PlusRTTManager::getSingleton().createRenderTexture(name, surface, writeGamma, fsaa);
                mSliceTRT.push_back(trt);
                Root::getSingleton().getRenderSystem()->attachRenderTarget(*mSliceTRT[zoffset]);
            }
        }
    }


    GL3PlusTextureBuffer::~GL3PlusTextureBuffer()
    {
        if (mUsage & TU_RENDERTARGET)
        {
            // Delete all render targets that are not yet deleted via _clearSliceRTT because the rendertarget
            // was deleted by the user.
            for (SliceTRT::const_iterator it = mSliceTRT.begin(); it != mSliceTRT.end(); ++it)
            {
                if( *it )
                    Root::getSingleton().getRenderSystem()->destroyRenderTarget((*it)->getName());
            }
        }
    }


    void GL3PlusTextureBuffer::upload(const PixelBox &data, const Box &dest)
    {
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
        OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));

        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));

        // Use PBO as a texture buffer.
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mBufferId));

        // Calculate size for all mip levels of the texture.
        size_t dataSize = 0;
        if (mTarget == GL_TEXTURE_2D_ARRAY)
        {
            dataSize = PixelUtil::getMemorySize(dest.getWidth(), dest.getHeight(), dest.getDepth(), data.format);
        }
        else
        {
            dataSize = PixelUtil::getMemorySize(data.getWidth(), data.getHeight(), mDepth, data.format);
        }

        //TODO Is this the correct was to set buffer size in this case?
        // Fill buffer with NULL values in order to set the buffer size.
        OGRE_CHECK_GL_ERROR(glBufferData(GL_PIXEL_UNPACK_BUFFER, dataSize, NULL, GL3PlusHardwareBufferManager::getGLUsage(mUsage)));

        // std::stringstream str;
        // str << "GL3PlusHardwarePixelBuffer::upload: " << mTextureID
        // << " pixel buffer: " << mBufferId
        // << " bytes: " << mSizeInBytes
        // << " dest depth: " << dest.getDepth()
        // << " dest front: " << dest.front
        // << " datasize: " << dataSize
        // << " face: " << mFace << " level: " << mLevel
        // << " width: " << mWidth << " height: "<< mHeight << " depth: " << mDepth
        // << " format: " << PixelUtil::getFormatName(mFormat);
        // LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

        void* pBuffer = 0;
        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(
            GL_PIXEL_UNPACK_BUFFER, 0, dataSize,
            GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_RANGE_BIT));

        if (pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Texture Buffer: Out of memory",
                        "GL3PlusTextureBuffer::upload");
        }

        // Copy texture data to destination buffer.
        memcpy(pBuffer, data.data, dataSize);
        GLboolean mapped = false;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER));
        if (!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Buffer data corrupted, please reload",
                        "GL3PlusTextureBuffer::upload");
        }

        if( PixelUtil::isCompressed(data.format) )
        {
            if (data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compressed images must be consecutive and in the designated source format",
                            "GL3PlusTextureBuffer::upload");

            GLenum format = GL3PlusPixelUtil::getClosestGLInternalFormat(mFormat, mHwGamma);
            // Data must be consecutive and at beginning of buffer as
            // PixelStorei not allowed for compressed formats.
            switch(mTarget)
            {
            case GL_TEXTURE_1D:
                OGRE_CHECK_GL_ERROR(glCompressedTexSubImage1D(
                    GL_TEXTURE_1D, mLevel,
                    dest.left,
                    dest.getWidth(),
                    format, data.getConsecutiveSize(),
                    NULL));
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_RECTANGLE:
                OGRE_CHECK_GL_ERROR(glCompressedTexSubImage2D(
                    mFaceTarget, mLevel,
                    dest.left, dest.top,
                    dest.getWidth(), dest.getHeight(),
                    format, data.getConsecutiveSize(),
                    NULL));
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
                OGRE_CHECK_GL_ERROR(glCompressedTexSubImage3D(
                    mTarget, mLevel,
                    dest.left, dest.top, dest.front,
                    dest.getWidth(), dest.getHeight(), dest.getDepth(),
                    format, data.getConsecutiveSize(),
                    NULL));
                break;
            }

        }
        else
        {
            if (data.getWidth() != data.rowPitch)
                OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ROW_LENGTH, data.rowPitch));
            if ((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3)
            {
                // Standard alignment of 4 is not right.
                OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
            }

            GLenum type = GL3PlusPixelUtil::getGLOriginDataType(data.format);

            assert( !PixelUtil::isDepth( data.format ) );

            switch(mTarget)
            {
            case GL_TEXTURE_1D:
                OGRE_CHECK_GL_ERROR(glTexSubImage1D(
                    GL_TEXTURE_1D, mLevel,
                    dest.left,
                    dest.getWidth(),
                    GL3PlusPixelUtil::getGLOriginFormat(data.format),
                    type,
                    NULL));
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP:
            case GL_TEXTURE_RECTANGLE:
                OGRE_CHECK_GL_ERROR(glTexSubImage2D(
                    mFaceTarget, mLevel,
                    dest.left, dest.top,
                    dest.getWidth(), dest.getHeight(),
                    GL3PlusPixelUtil::getGLOriginFormat(data.format),
                    type,
                    NULL));
                break;
            case GL_TEXTURE_3D:
            case GL_TEXTURE_2D_ARRAY:
                OGRE_CHECK_GL_ERROR(glTexSubImage3D(
                    mTarget, mLevel,
                    dest.left, dest.top, dest.front,
                    dest.getWidth(), dest.getHeight(), dest.getDepth(),
                    GL3PlusPixelUtil::getGLOriginFormat(data.format),
                    type,
                    NULL));
                break;
            }
            if (mUsage & TU_AUTOMIPMAP && (mTarget == GL_TEXTURE_2D_ARRAY || mTarget == GL_TEXTURE_3D))
            {
                OGRE_CHECK_GL_ERROR(glGenerateMipmap(mTarget));
            }
        }

        // Delete PBO.
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0));
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
        mBufferId = 0;

        // Restore defaults.
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ROW_LENGTH, 0));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0));
        OGRE_CHECK_GL_ERROR(glPixelStorei(GL_UNPACK_ALIGNMENT, 4));
    }

    void GL3PlusTextureBuffer::download(const PixelBox &data)
    {
        /*if (data.getWidth() != getWidth() ||
            data.getHeight() != getHeight() ||
            data.getDepth() != getDepth())
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "only download of entire buffer is supported by GL",
                        "GL3PlusTextureBuffer::download");*/

        // Upload data to PBO
        OGRE_CHECK_GL_ERROR(glGenBuffers(1, &mBufferId));
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_PIXEL_PACK_BUFFER, mBufferId));

        OGRE_CHECK_GL_ERROR(glBufferData(GL_PIXEL_PACK_BUFFER, mSizeInBytes, NULL,
                                         GL3PlusHardwareBufferManager::getGLUsage(mUsage)));

        //        std::stringstream str;
        //        str << "GL3PlusHardwarePixelBuffer::download: " << mTextureID
        //        << " pixel buffer: " << mBufferId
        //        << " bytes: " << mSizeInBytes
        //        << " face: " << mFace << " level: " << mLevel
        //        << " width: " << mWidth << " height: "<< mHeight << " depth: " << mDepth
        //        << " format: " << PixelUtil::getFormatName(mFormat);
        //        LogManager::getSingleton().logMessage(LML_NORMAL, str.str());

        OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));
        if (PixelUtil::isCompressed(data.format))
        {
            if (data.format != mFormat || !data.isConsecutive())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS,
                            "Compressed images must be consecutive, in the source format",
                            "GL3PlusTextureBuffer::download");
            // Data must be consecutive and at beginning of buffer as PixelStorei not allowed
            // for compressed formate
            OGRE_CHECK_GL_ERROR(glGetCompressedTexImage(mFaceTarget, mLevel, 0));
        }
        else
        {
            if ((data.getWidth()*PixelUtil::getNumElemBytes(data.format)) & 3) {
                // Standard alignment of 4 is not right
                OGRE_CHECK_GL_ERROR(glPixelStorei(GL_PACK_ALIGNMENT, 1));
            }
            // We can only get the entire texture
            OGRE_CHECK_GL_ERROR(glGetTexImage(mFaceTarget, mLevel,
                                              GL3PlusPixelUtil::getGLOriginFormat(data.format),
                                              GL3PlusPixelUtil::getGLOriginDataType(data.format),
                                              0));

            // Restore defaults
            OGRE_CHECK_GL_ERROR(glPixelStorei(GL_PACK_ALIGNMENT, 4));
        }

        GLint offsetInBytes = 0;
        uint32 width = mWidth;
        uint32 height = mHeight;
//        uint32 depth = mDepth;

//        for(GLint i = 0; i < mLevel; i++)
//        {
//            offsetInBytes += PixelUtil::getMemorySize(width, height, depth, data.format);
//            if (width > 1)
//                width >>= 1;
//            if (height > 1)
//                height >>= 1;
//            if (depth > 1)
//                depth >>= 1;
//        }

        offsetInBytes += PixelUtil::getMemorySize( width, height, data.front, mFormat );
        size_t sizeInBytes = PixelUtil::getMemorySize( width, height, data.getDepth(), data.format );

        void* pBuffer;
        OGRE_CHECK_GL_ERROR(pBuffer = glMapBufferRange(GL_PIXEL_PACK_BUFFER, offsetInBytes, sizeInBytes, GL_MAP_READ_BIT));

        if (pBuffer == 0)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Texture Buffer: Out of memory",
                        "GL3PlusTextureBuffer::download");
        }

        // Copy to destination buffer
        if( data.getWidth() == width && data.getHeight() == height )
        {
            memcpy( data.data, pBuffer, data.getConsecutiveSize() );
        }
        else
        {
            uint32 blockWidth  = PixelUtil::getCompressedBlockWidth( mFormat );
            uint32 blockHeight = PixelUtil::getCompressedBlockHeight( mFormat );
            uint32 blockResolution = blockWidth * blockHeight;
            if( !blockWidth || !blockHeight )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                            "Cannot transfer subregions of the image when compressed by format " +
                             PixelUtil::getFormatName( mFormat ) +
                             ". You must download the entire image.",
                            "GL3PlusTextureBuffer::download");
            }

            if( data.left % blockWidth || data.right % blockWidth ||
                data.top % blockHeight || data.bottom % blockHeight )
            {
                OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                            "Image downloads from GPU for the compressed format " +
                             PixelUtil::getFormatName( mFormat ) +
                             " requires subregions to be aligned to " +
                             StringConverter::toString( blockWidth ) + "x" +
                             StringConverter::toString( blockHeight ) +  " blocks",
                             "GL3PlusTextureBuffer::download");
            }

            size_t blockSize = PixelUtil::getMemorySize( blockWidth, blockHeight, 1, mFormat );

            for( size_t z=0; z<data.getDepth(); ++z )
            {
                size_t dstZ = z * ( (data.getWidth() * data.getHeight()) / blockResolution );
                size_t srcZ = z * ( (width * height) / blockResolution );
                for( size_t y=data.top; y<data.bottom; y += blockHeight )
                {
                    size_t dstY = ((y - data.top) * data.getWidth()) / blockResolution;
                    size_t srcY = (y * width) / blockResolution;
                    memcpy( (uint8*)(data.data) + ( (dstZ + dstY) * blockSize ),
                            (uint8*)(pBuffer) + ( (srcZ + srcY + data.left / blockWidth) * blockSize ),
                            (data.getWidth() / blockWidth) * blockSize );
                }
            }
        }

        GLboolean mapped;
        OGRE_CHECK_GL_ERROR(mapped = glUnmapBuffer(GL_PIXEL_PACK_BUFFER));
        if (!mapped)
        {
            OGRE_EXCEPT(Exception::ERR_INTERNAL_ERROR,
                        "Buffer data corrupted, please reload",
                        "GL3PlusTextureBuffer::download");
        }

        // Delete PBO
        OGRE_CHECK_GL_ERROR(glBindBuffer(GL_PIXEL_PACK_BUFFER, 0));
        OGRE_CHECK_GL_ERROR(glDeleteBuffers(1, &mBufferId));
        mBufferId = 0;
    }


    void GL3PlusTextureBuffer::bindToFramebuffer(GLenum attachment, uint32 zoffset)
    {
        // Delegate the framebuffer binding to a more specific function
        // This call retains the original implementation using GL_FRAMEBUFFER (aka GL_DRAW_FRAMEBUFFER)
        _bindToFramebuffer(attachment, zoffset, GL_DRAW_FRAMEBUFFER);
    }


    void GL3PlusTextureBuffer::copyFromFramebuffer(uint32 zoffset)
    {
        OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));
        switch(mTarget)
        {
        case GL_TEXTURE_1D:
            OGRE_CHECK_GL_ERROR(glCopyTexSubImage1D(mFaceTarget, mLevel, 0, 0, 0, mWidth));
            break;
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_RECTANGLE:
            OGRE_CHECK_GL_ERROR(glCopyTexSubImage2D(mFaceTarget, mLevel, 0, 0, 0, 0, mWidth, mHeight));
            break;
        case GL_TEXTURE_3D:
        case GL_TEXTURE_2D_ARRAY:
            OGRE_CHECK_GL_ERROR(glCopyTexSubImage3D(mFaceTarget, mLevel, 0, 0, zoffset, 0, 0, mWidth, mHeight));
            break;
        }
    }


    void GL3PlusTextureBuffer::blit(const HardwarePixelBufferSharedPtr &src, const Box &srcBox, const Box &dstBox)
    {
        GL3PlusTextureBuffer *srct = static_cast<GL3PlusTextureBuffer *>(src.getPointer());
        // Check for FBO support first
        // Destination texture must be 1D, 2D, 3D, or Cube
        // Source texture must be 1D, 2D or 3D

        // This does not seem to work for RTTs after the first update
        // I have no idea why! For the moment, disable
        if ((src->getUsage() & TU_RENDERTARGET) == 0 &&
            (srct->mTarget == GL_TEXTURE_1D || srct->mTarget == GL_TEXTURE_2D
             || srct->mTarget == GL_TEXTURE_RECTANGLE || srct->mTarget == GL_TEXTURE_3D)
            && mTarget != GL_TEXTURE_2D_ARRAY)
        {
            blitFromTexture(srct, srcBox, dstBox);
        }
        else
        {
            GL3PlusHardwarePixelBuffer::blit(src, srcBox, dstBox);
        }
    }


    // Very fast texture-to-texture blitter and hardware bi/trilinear scaling implementation using FBO
    // Destination texture must be 1D, 2D, 3D, or Cube
    // Source texture must be 1D, 2D or 3D
    // Supports compressed formats as both source and destination format, it will use the hardware DXT compressor
    // if available.
    // @author W.J. van der Laan
    void GL3PlusTextureBuffer::blitFromTexture(GL3PlusTextureBuffer *src, const Box &srcBox, const Box &dstBox)
    {
        //        std::cerr << "GL3PlusTextureBuffer::blitFromTexture " <<
        //        src->mTextureID << ":" << srcBox.left << "," << srcBox.top << "," << srcBox.right << "," << srcBox.bottom << " " <<
        //        mTextureID << ":" << dstBox.left << "," << dstBox.top << "," << dstBox.right << "," << dstBox.bottom << std::endl;
        // Store reference to FBO manager
        GL3PlusFBOManager *fboMan = static_cast<GL3PlusFBOManager *>(GL3PlusRTTManager::getSingletonPtr());

        GLenum filtering = GL_LINEAR;

        // Set filtering modes depending on the dimensions and source
        if (srcBox.getWidth()==dstBox.getWidth() &&
            srcBox.getHeight()==dstBox.getHeight() &&
            srcBox.getDepth()==dstBox.getDepth())
        {
            // Dimensions match -- use nearest filtering (fastest and pixel correct)
            filtering = GL_NEAREST;
        }

        // Store old binding so it can be restored later
        GLint oldfb;
        OGRE_CHECK_GL_ERROR(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldfb));

        // Set up temporary FBO
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboMan->getTemporaryFBO(0)));
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMan->getTemporaryFBO(1)));

        GLuint tempTex = 0;
        if (!fboMan->checkFormat(mFormat))
        {
            // If target format not directly supported, create intermediate texture
            GLenum tempFormat = GL3PlusPixelUtil::getClosestGLInternalFormat(fboMan->getSupportedAlternative(mFormat),false);
            OGRE_CHECK_GL_ERROR(glGenTextures(1, &tempTex));
            OGRE_CHECK_GL_ERROR(glBindTexture(GL_TEXTURE_2D, tempTex));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
            OGRE_CHECK_GL_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0));

            // Allocate temporary texture of the size of the destination area
            OGRE_CHECK_GL_ERROR(glTexImage2D(GL_TEXTURE_2D, 0, tempFormat,
                                             dstBox.getWidth(), dstBox.getHeight(),
                                             0, GL_RGBA, GL_UNSIGNED_BYTE, 0));
            OGRE_CHECK_GL_ERROR(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                     tempTex, 0));

            OGRE_CHECK_GL_ERROR(glCheckFramebufferStatus(GL_FRAMEBUFFER));

            // Set viewport to size of destination slice
            OGRE_CHECK_GL_ERROR(glViewport(0, 0, dstBox.getWidth(), dstBox.getHeight()));
        }
        else
        {
            // We are going to bind directly, so set viewport to size and position of destination slice
            OGRE_CHECK_GL_ERROR(glViewport(dstBox.left, dstBox.top, dstBox.getWidth(), dstBox.getHeight()));
        }

        // Process each destination slice
        for(uint32 slice = dstBox.front; slice < dstBox.back; ++slice)
        {
            if (!tempTex)
            {
                assert( !PixelUtil::isDepth( mFormat ) );
                bindToFramebuffer(GL_COLOR_ATTACHMENT0, slice);
            }

            OGRE_CHECK_GL_ERROR(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));

            GLbitfield mask = GL_ZERO;

            assert( !PixelUtil::isDepth( mFormat ) );

            // Bind the appropriate source texture to the read framebuffer
            src->_bindToFramebuffer(GL_COLOR_ATTACHMENT0, slice, GL_READ_FRAMEBUFFER);

            OGRE_CHECK_GL_ERROR(glReadBuffer(GL_COLOR_ATTACHMENT0));

            mask |= GL_COLOR_BUFFER_BIT;

            OGRE_CHECK_GL_ERROR(glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

            assert(mask != GL_ZERO);

            // Perform blit from the source texture bound to read framebuffer to
            // this texture bound to draw framebuffer using the pixel coorinates.
            // Sampling ouside the source box is implicitly handled using GL_CLAMP_TO_EDGE.
            OGRE_CHECK_GL_ERROR(glBlitFramebuffer(srcBox.left, srcBox.top, srcBox.right, srcBox.bottom,
                                                  dstBox.left, dstBox.top, dstBox.right, dstBox.bottom,
                                                  mask, filtering));
        }

        // Finish up
        if (!tempTex)
        {
            // Generate mipmaps
            if (mUsage & TU_AUTOMIPMAP)
            {
                OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));
                OGRE_CHECK_GL_ERROR(glGenerateMipmap(mTarget));
            }
        }

        // Reset source texture to sane state
        OGRE_CHECK_GL_ERROR(glBindTexture(src->mTarget, src->mTextureID));

        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_RENDERBUFFER, 0));
        OGRE_CHECK_GL_ERROR(glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                                      GL_RENDERBUFFER, 0));

        // Reset read buffer/framebuffer
        OGRE_CHECK_GL_ERROR(glReadBuffer(GL_NONE));
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

        // Restore old framebuffer
        OGRE_CHECK_GL_ERROR(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, oldfb));
        OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &tempTex));
    }

    void GL3PlusTextureBuffer::_bindToFramebuffer(GLenum attachment, uint32 zoffset, GLenum which)
    {
        assert(zoffset < mDepth);
        assert(which == GL_READ_FRAMEBUFFER || which == GL_DRAW_FRAMEBUFFER || which == GL_FRAMEBUFFER);

        OGRE_CHECK_GL_ERROR(glBindTexture(mTarget, mTextureID));
        switch(mTarget)
        {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            OGRE_CHECK_GL_ERROR(glFramebufferTexture(which, attachment,
                                                     mTextureID, mLevel));
            break;
        case GL_TEXTURE_CUBE_MAP:
            OGRE_CHECK_GL_ERROR(glFramebufferTexture2D(which, GL_COLOR_ATTACHMENT0,
                                                       mFaceTarget, mTextureID, mLevel));
            break;
        case GL_TEXTURE_3D:
            OGRE_CHECK_GL_ERROR(glFramebufferTexture3D(which, attachment,
                                                       mFaceTarget, mTextureID, mLevel, zoffset));
            break;
        case GL_TEXTURE_2D_ARRAY:
            OGRE_CHECK_GL_ERROR(glFramebufferTextureLayer(which, attachment, mTextureID, mLevel, zoffset));
            break;
        }
    }


    // blitFromMemory doing hardware trilinear scaling
    void GL3PlusTextureBuffer::blitFromMemory(const PixelBox &src_orig, const Box &dstBox)
    {
        // Fall back to normal GLHardwarePixelBuffer::blitFromMemory in case
        // - FBO is not supported
        // - Either source or target is luminance due doesn't looks like supported by hardware
        // - the source dimensions match the destination ones, in which case no scaling is needed
        if (PixelUtil::isLuminance(src_orig.format) ||
            PixelUtil::isLuminance(mFormat) ||
            (src_orig.getWidth() == dstBox.getWidth() &&
             src_orig.getHeight() == dstBox.getHeight() &&
             src_orig.getDepth() == dstBox.getDepth()))
        {
            GL3PlusHardwarePixelBuffer::blitFromMemory(src_orig, dstBox);
            return;
        }
        if (!mBuffer.contains(dstBox))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Destination box out of range",
                        "GL3PlusTextureBuffer::blitFromMemory");
        // For scoped deletion of conversion buffer
        MemoryDataStreamPtr buf;
        PixelBox src;

        // First, convert the srcbox to a OpenGL compatible pixel format
        if (GL3PlusPixelUtil::getGLOriginFormat(src_orig.format) == 0)
        {
            // Convert to buffer internal format
            buf.bind(new MemoryDataStream(PixelUtil::getMemorySize(src_orig.getWidth(), src_orig.getHeight(),
                                                                   src_orig.getDepth(), mFormat)));
            src = PixelBox(src_orig.getWidth(), src_orig.getHeight(), src_orig.getDepth(), mFormat, buf->getPtr());
            PixelUtil::bulkPixelConversion(src_orig, src);
        }
        else
        {
            // No conversion needed
            src = src_orig;
        }

        // Create temporary texture to store source data
        GLuint id;
        GLenum target = (src.getDepth() != 1) ? GL_TEXTURE_3D : GL_TEXTURE_2D;

        // Generate texture name
        OGRE_CHECK_GL_ERROR(glGenTextures(1, &id));

        // Set texture type
        OGRE_CHECK_GL_ERROR(glBindTexture(target, id));

        // Set automatic mipmap generation; nice for minimisation
        OGRE_CHECK_GL_ERROR(glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0));
        OGRE_CHECK_GL_ERROR(glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 1000));

        GLenum internalFormat = GL3PlusPixelUtil::getGLInternalFormat(src.format, mHwGamma);

        GLenum format = GL_RGBA;

        switch (internalFormat)
        {
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
                format = GL_DEPTH_COMPONENT;
                break;

            default:
                break;
        }

        // Allocate texture memory
        if (target == GL_TEXTURE_3D || target == GL_TEXTURE_2D_ARRAY)
        {
            OGRE_CHECK_GL_ERROR(glTexImage3D(target, 0, internalFormat, src.getWidth(), src.getHeight(), src.getDepth(), 0, format, GL_UNSIGNED_BYTE, 0));
        }
        else
        {
            OGRE_CHECK_GL_ERROR(glTexImage2D(target, 0, internalFormat, src.getWidth(), src.getHeight(), 0, format, GL_UNSIGNED_BYTE, 0));
        }

        // GL texture buffer
        GL3PlusTextureBuffer tex(BLANKSTRING, target, id, 0, 0, (Usage)(TU_AUTOMIPMAP|HBU_STATIC_WRITE_ONLY), false, 0);

        // Upload data to 0,0,0 in temporary texture
        Box tempTarget(0, 0, 0, src.getWidth(), src.getHeight(), src.getDepth());
        tex.upload(src, tempTarget);

        // Blit
        blitFromTexture(&tex, tempTarget, dstBox);

        // Delete temp texture
        OGRE_CHECK_GL_ERROR(glDeleteTextures(1, &id));
    }


    RenderTexture *GL3PlusTextureBuffer::getRenderTarget(size_t zoffset)
    {
        assert(mUsage & TU_RENDERTARGET);
        assert(zoffset < mDepth);
        return mSliceTRT[zoffset];
    }
}
}
