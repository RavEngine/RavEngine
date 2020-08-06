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

#include "OgreHlmsTextureManager.h"
#include "OgreHlmsTexturePack.h"
#include "OgreTextureManager.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreRenderSystem.h"
#include "OgreBitwise.h"
#include "OgreLogManager.h"
#include "OgreHlmsDatablock.h"
#include "OgreLwString.h"
#include "OgreProfiler.h"

#if !OGRE_NO_JSON
    #include "rapidjson/document.h"
    #include "rapidjson/error/en.h"
#endif

namespace Ogre
{
    HlmsTextureManager::HlmsTextureManager() : mRenderSystem( 0 ), mTextureId( 0 )
    {
        mDefaultTextureParameters[TEXTURE_TYPE_DIFFUSE].hwGammaCorrection   = true;
        mDefaultTextureParameters[TEXTURE_TYPE_MONOCHROME].pixelFormat      = PF_L8;
        mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat         = PF_BC5_SNORM;
        mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].isNormalMap         = true;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].hwGammaCorrection    = true;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat=PF_BC5_SNORM;
        mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].isNormalMap = true;
        mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].hwGammaCorrection   = true;
        mDefaultTextureParameters[TEXTURE_TYPE_NON_COLOR_DATA].hwGammaCorrection = false;
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::~HlmsTextureManager()
    {
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::_changeRenderSystem( RenderSystem *newRs )
    {
        mRenderSystem = newRs;

        if( mRenderSystem )
        {
            const RenderSystemCapabilities *caps = mRenderSystem->getCapabilities();

            if( caps )
            {
                TextureType textureType = TEX_TYPE_2D;

                if( caps->hasCapability(RSC_TEXTURE_2D_ARRAY) ) //TODO
                {
                    textureType = TEX_TYPE_2D_ARRAY;

                    for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
                    {
                        mDefaultTextureParameters[i].packingMethod = TextureArrays;
                        mDefaultTextureParameters[i].maxTexturesPerArray = 40;
                    }

                    mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 20;
                    if( !caps->hasCapability(RSC_TEXTURE_CUBE_MAP_ARRAY) )
                        mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 1;
                }
                else
                {
                    for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
                    {
                        mDefaultTextureParameters[i].packingMethod = Atlas;
                        mDefaultTextureParameters[i].maxTexturesPerArray = 1;
                    }
                    mDefaultTextureParameters[TEXTURE_TYPE_ENV_MAP].maxTexturesPerArray = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].maxTexturesPerArray  = 1;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].maxTexturesPerArray = 1;
                }

                bool hwGammaCorrection = caps->hasCapability( RSC_HW_GAMMA );
                mDefaultTextureParameters[TEXTURE_TYPE_DIFFUSE].hwGammaCorrection   = hwGammaCorrection;
                mDefaultTextureParameters[TEXTURE_TYPE_DETAIL].hwGammaCorrection    = hwGammaCorrection;

                // BC5 is the best, native (lossy) compressor for normal maps.
                // DXT5 is like BC5, using the "store only in green and alpha channels" method.
                // The last one is lossless, using UV8 to store uncompressed,
                // and retrieve z = sqrt(x²+y²)
                if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_BC4_BC5) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat = PF_BC5_SNORM;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_BC5_SNORM;
                }
                /*else if( caps->hasCapability(RSC_TEXTURE_COMPRESSION_DXT) )
                {
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = PF_DXT5;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = PF_DXT5;
                }*/
                else
                {
                    PixelFormat pf = caps->hasCapability( RSC_TEXTURE_SIGNED_INT ) ? PF_R8G8_SNORM :
                                                                                     PF_BYTE_LA;
                    mDefaultTextureParameters[TEXTURE_TYPE_NORMALS].pixelFormat           = pf;
                    mDefaultTextureParameters[TEXTURE_TYPE_DETAIL_NORMAL_MAP].pixelFormat = pf;
                }

                mBlankTexture = TextureManager::getSingleton().createManual( "Hlms_Blanktexture",
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                textureType, 4, 4, 1, 0, PF_R8G8B8A8, TU_DEFAULT, 0,
                                                false, 0, BLANKSTRING, false );

                v1::HardwarePixelBufferSharedPtr pixelBufferBuf = mBlankTexture->getBuffer(0);
                const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0, 4, 4, 1 ),
                                                                  v1::HardwareBuffer::HBL_DISCARD );
                uint8 *data = reinterpret_cast<uint8*>( currImage.data );
                for( size_t y=0; y<currImage.getHeight(); ++y )
                {
                    for( size_t x=0; x<currImage.getWidth(); ++x )
                    {
                        *data++ = 255;
                        *data++ = 255;
                        *data++ = 255;
                        *data++ = 255;
                    }

                    data += ( currImage.rowPitch - currImage.getWidth() ) * 4;
                }
                pixelBufferBuf->unlock();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    TexturePtr HlmsTextureManager::reservePoolId( uint32 uniqueSpecialId, TextureMapType mapType,
                                                  uint32 width, uint32 height,
                                                  uint16 numSlices, uint8 numMipmaps,
                                                  PixelFormat pixelFormat,
                                                  bool isNormalMap, bool hwGammaCorrection )
    {
        TextureArray textureArray( 1u, numSlices, true,
                                   isNormalMap, true, uniqueSpecialId );

        textureArray.texture = TextureManager::getSingleton().createManual(
                                    "ReservedPoolHlmsTexture/" +
                                    StringConverter::toString( mTextureId++ ),
                                    ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                    TEX_TYPE_2D_ARRAY, width, height, numSlices, numMipmaps,
                                    pixelFormat,
                                    TU_DEFAULT & ~TU_AUTOMIPMAP, 0,
                                    hwGammaCorrection,
                                    0, BLANKSTRING, false );

        mTextureArrays[mapType].push_back( textureArray );

        return textureArray.texture;
    }
    //-----------------------------------------------------------------------------------
    bool HlmsTextureManager::hasPoolId( uint32 uniqueSpecialId, TextureMapType mapType ) const
    {
        TextureArrayVec::const_iterator itor = mTextureArrays[mapType].begin();
        TextureArrayVec::const_iterator end  = mTextureArrays[mapType].end();

        while( itor != end && itor->uniqueSpecialId != uniqueSpecialId )
            ++itor;

        return itor != end;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copyTextureToArray( const Image &srcImage, TexturePtr dst, uint16 entryIdx,
                                                 uint8 srcBaseMip, bool isNormalMap )
    {
        OgreProfileExhaustive( "HlmsTextureManager::copyTextureToArray" );

        //TODO: Deal with mipmaps (& cubemaps & 3D? does it work?). We could have:
        //  * Original image has mipmaps, we use them all
        //  * Original image has mipmaps, we discard them
        //  * Original image has mipmaps, we use them, but we need to create more
        //  * Original image doesn't have mipmaps, but we need to create them
        // The last problem is a subset of the 3rd problem
        //
        //  See Texture::_loadImages
        uint8 minMipmaps = std::min<uint8>( srcImage.getNumMipmaps() - srcBaseMip,
                                            dst->getNumMipmaps() ) + 1;
        for( uint8 j=0; j<minMipmaps; ++j )
        {
            v1::HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0, j);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, entryIdx,
                                                                   pixelBufferBuf->getWidth(),
                                                                   pixelBufferBuf->getHeight(),
                                                                   entryIdx + 1 ),
                                                              v1::HardwareBuffer::HBL_DISCARD );
            if( isNormalMap && srcImage.getFormat() != dst->getFormat() )
                PixelUtil::convertForNormalMapping( srcImage.getPixelBox(0, j + srcBaseMip), currImage );
            else
                PixelUtil::bulkPixelConversion( srcImage.getPixelBox(0, j + srcBaseMip), currImage );
            pixelBufferBuf->unlock();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copyTextureToAtlas( const Image &srcImage, TexturePtr dst,
                                                 uint16 entryIdx, uint16 sqrtMaxTextures,
                                                 uint8 srcBaseMip, bool isNormalMap )
    {
        OgreProfileExhaustive( "HlmsTextureManager::copyTextureToAtlas" );

        //TODO: Deal with mipmaps (& cubemaps & 3D? does it work?).
        size_t xBlock = entryIdx % sqrtMaxTextures;
        size_t yBlock = entryIdx / sqrtMaxTextures;

        size_t nextX = ( entryIdx % sqrtMaxTextures ) + 1;
        size_t nextY = ( entryIdx / sqrtMaxTextures ) + 1;

        /*if( sqrtMaxTextures > 1 && PixelUtil::isCompressed( dst->getFormat() ) )
        {
            HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0,
                                                                   dst->getWidth(),
                                                                   dst->getHeight(),
                                                                   dst->getDepth() ),
                                                              HardwareBuffer::HBL_DISCARD );
                                                              //HardwareBuffer::HBL_NORMAL );
            PixelUtil::bulkCompressedSubregion( srcImage.getPixelBox(0, srcBaseMip), currImage,
                                                Box( xBlock * srcImage.getWidth(),
                                                     yBlock * srcImage.getHeight(),
                                                     0,
                                                     nextX * srcImage.getWidth(),
                                                     nextY * srcImage.getHeight(),
                                                     dst->getDepth() ) );
            pixelBufferBuf->unlock();
        }
        else*/
        uint8 minMipmaps = std::min<uint8>( srcImage.getNumMipmaps() - srcBaseMip,
                                            dst->getNumMipmaps() ) + 1;
        for( uint8 j=0; j<minMipmaps; ++j )
        {
            v1::HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer(0, j);
            const PixelBox &currImage = pixelBufferBuf->lock( Box( xBlock * pixelBufferBuf->getWidth(),
                                                                   yBlock * pixelBufferBuf->getHeight(),
                                                                   0,
                                                                   nextX * pixelBufferBuf->getWidth(),
                                                                   nextY * pixelBufferBuf->getHeight(),
                                                                   dst->getDepth() ),
                                                              v1::HardwareBuffer::HBL_DISCARD );
            if( isNormalMap && srcImage.getFormat() != dst->getFormat() )
                PixelUtil::convertForNormalMapping( srcImage.getPixelBox(0, j + srcBaseMip), currImage );
            else
                PixelUtil::bulkPixelConversion( srcImage.getPixelBox(0, j + srcBaseMip), currImage );
            pixelBufferBuf->unlock();
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::copy3DTexture( const Image &srcImage, TexturePtr dst,
                                            uint16 sliceStart, uint16 sliceEnd, uint8 srcBaseMip )
    {
        OgreProfileExhaustive( "HlmsTextureManager::copy3DTexture" );

        for( uint16 i=sliceStart; i<sliceEnd; ++i )
        {
            uint8 minMipmaps = std::min<uint8>( srcImage.getNumMipmaps() - srcBaseMip,
                                                dst->getNumMipmaps() ) + 1;
            for( uint8 j=0; j<minMipmaps; ++j )
            {
                v1::HardwarePixelBufferSharedPtr pixelBufferBuf = dst->getBuffer( i, j );
                const PixelBox &currImage = pixelBufferBuf->lock( Box( 0, 0, 0,
                                                                       pixelBufferBuf->getWidth(),
                                                                       pixelBufferBuf->getHeight(),
                                                                       1 ),
                                                                  v1::HardwareBuffer::HBL_DISCARD );

                PixelUtil::bulkPixelConversion( srcImage.getPixelBox( i - sliceStart,
                                                                      srcBaseMip + j ),
                                                currImage );
                pixelBufferBuf->unlock();
            }
        }
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureArrayVec::iterator HlmsTextureManager::findSuitableArray(
            TextureMapType mapType, uint32 width, uint32 height,
            uint32 depth, uint32 faces, PixelFormat format, uint8 numMipmaps,
            uint32 uniqueSpecialId, const String &textureName )
    {
        TextureArrayVec::iterator retVal = mTextureArrays[mapType].end();

        //Find an array where we can put it. If there is none, we'll have have to create a new one
        TextureArrayVec::iterator itor = mTextureArrays[mapType].begin();
        TextureArrayVec::iterator end  = mTextureArrays[mapType].end();

        while( itor != end && retVal == end )
        {
            TextureArray &textureArray = *itor;

            uint32 arrayTexWidth = textureArray.texture->getWidth() / textureArray.sqrtMaxTextures;
            uint32 arrayTexHeight= textureArray.texture->getHeight() / textureArray.sqrtMaxTextures;
            if( textureArray.automatic &&
                textureArray.activeEntries < textureArray.maxTextures &&
                arrayTexWidth  == width  &&
                arrayTexHeight == height &&
                    (textureArray.texture->getTextureType() != TEX_TYPE_3D ||
                    textureArray.texture->getDepth()== depth)  &&
                textureArray.texture->getNumFaces() == faces &&
                textureArray.texture->getFormat() == format &&
                textureArray.texture->getNumMipmaps() == numMipmaps )
            {
                if( textureArray.uniqueSpecialId == uniqueSpecialId )
                    retVal = itor;
            }
            else
            {
                if( textureArray.uniqueSpecialId && textureArray.uniqueSpecialId == uniqueSpecialId )
                {
                    LogManager::getSingleton().logMessage(
                                "Texture " + textureName + " was requested with special ID, but it "
                                "cannot be assigned to the array of the special ID! Reason:",
                            LML_CRITICAL );

                    char tmpBuffer[512];
                    LwString errorMsg( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

                    if( textureArray.activeEntries >= textureArray.maxTextures )
                    {
                        errorMsg.clear();
                        errorMsg.a( "Exceeded entry limit: ",
                                    textureArray.activeEntries, "/", textureArray.maxTextures );
                        LogManager::getSingleton().logMessage( errorMsg.c_str(), LML_CRITICAL );
                    }

                    if( textureArray.texture->getFormat() != format )
                    {
                        errorMsg.clear();
                        errorMsg.a( "Different format. Requested: ",
                                    PixelUtil::getFormatName( format ).c_str(), "; should've been: ",
                                    PixelUtil::getFormatName(
                                        textureArray.texture->getFormat() ).c_str() );
                        LogManager::getSingleton().logMessage( errorMsg.c_str(), LML_CRITICAL );
                    }

                    if( textureArray.texture->getNumMipmaps() != numMipmaps )
                    {
                        errorMsg.clear();
                        errorMsg.a( "Different number of mipmaps. Requested: ",
                                    numMipmaps, "; should've been: ",
                                    textureArray.texture->getNumMipmaps() );
                        LogManager::getSingleton().logMessage( errorMsg.c_str(), LML_CRITICAL );
                    }

                    if( arrayTexWidth != width ||
                        arrayTexHeight != height ||
                        (textureArray.texture->getTextureType() == TEX_TYPE_3D &&
                         textureArray.texture->getDepth() != depth)  ||
                        textureArray.texture->getNumFaces() != faces )
                    {
                        errorMsg.clear();
                        errorMsg.a( "Different resolution. Requested: ",
                                    width, "x", height, "x", depth );
                        errorMsg.a( "x", faces );
                        errorMsg.a( "\nvs: ",
                                    arrayTexWidth, "x", arrayTexHeight, "x",
                                    textureArray.texture->getDepth() );
                        errorMsg.a( "x", static_cast<uint32>( textureArray.texture->getNumFaces() ) );
                        LogManager::getSingleton().logMessage( errorMsg.c_str(), LML_CRITICAL );
                    }
                }
            }

            ++itor;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::createOrRetrieveTexture(
                                                                        const String &texName,
                                                                        TextureMapType mapType )
    {
        return createOrRetrieveTexture( texName, texName, mapType );
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::createOrRetrieveTexture(
                                                                        const String &aliasName,
                                                                        const String &texName,
                                                                        TextureMapType mapType,
                                                                        uint32 uniqueSpecialId,
                                                                        Image *imgSource )
    {
        OgreProfileExhaustive( "HlmsTextureManager::createOrRetrieveTexture" );

        MetadataCacheMap::const_iterator itor = mMetadataCache.find( aliasName );
        if( itor != mMetadataCache.end() )
        {
            mapType = itor->second.mapType;
            uniqueSpecialId = itor->second.poolId;
        }

        TextureEntry searchName( aliasName );
        TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(), searchName );

        TextureLocation retVal;

        assert( !aliasName.empty() && "Alias name can't be left empty!" );

        const bool missingFromCache = it == mEntries.end() || it->name != searchName.name;

        try
        {
        if( missingFromCache )
        {
            LogManager::getSingleton().logMessage( "Texture: loading " + texName + " as " + aliasName );

            Image localImageVar;
            Image *image = imgSource;

            if( !imgSource )
            {
                image = &localImageVar;
                image->load( texName, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
            }

            PixelFormat imageFormat = image->getFormat();
            const RenderSystemCapabilities *caps = mRenderSystem->getCapabilities();

            if( mDefaultTextureParameters[mapType].pixelFormat != PF_UNKNOWN )
            {
                //Don't force non-compressed sources to be compressed when we can't do it
                //automatically, but force them to a format we actually understand.
                if( mDefaultTextureParameters[mapType].isNormalMap &&
                    mDefaultTextureParameters[mapType].pixelFormat == PF_BC5_SNORM &&
                    imageFormat != PF_BC5_SNORM )
                {
                    LogManager::getSingleton().logMessage(
                                "WARNING: normal map texture " + texName + " is not BC5S compressed. "
                                "This is encouraged for lower memory usage. If you don't want to see "
                                "this message without compressing to BC5, set "
                                "getDefaultTextureParameters()[TEXTURE_TYPE_NORMALS].pixelFormat to "
                                "PF_R8G8_SNORM (or PF_BYTE_LA if RSC_TEXTURE_SIGNED_INT is not "
                                "supported)", LML_NORMAL);
                    imageFormat = caps->hasCapability( RSC_TEXTURE_SIGNED_INT ) ? PF_R8G8_SNORM :
                                                                                  PF_BYTE_LA;
                }
                else if (mDefaultTextureParameters[mapType].pixelFormat != imageFormat &&
                         (PixelUtil::isCompressed(imageFormat) ||
                          PixelUtil::isCompressed(mDefaultTextureParameters[mapType].pixelFormat)))
                {
                    //Image formats do not match, and one or both of the formats is compressed
                    //and therefore we can not convert it to the desired format.
                    //So we use the src image format instead of the requested image format
                    LogManager::getSingleton().logMessage(
                        "WARNING: The input texture " + texName + " is a " + PixelUtil::getFormatName(imageFormat) + " " +
                        "texture and can not be converted to the requested pixel format of " +
                        PixelUtil::getFormatName(mDefaultTextureParameters[mapType].pixelFormat) + ". " +
                        "This will potentially cause both an increase in memory usage and a decrease in performance. " +
                        "It is highly recommended you convert this texture to the requested format.", LML_NORMAL);
                }
                else
                {	
                    imageFormat = mDefaultTextureParameters[mapType].pixelFormat;
                }
            }

            if( imageFormat == PF_X8R8G8B8 || imageFormat == PF_R8G8B8 ||
                imageFormat == PF_X8B8G8R8 || imageFormat == PF_B8G8R8 ||
                imageFormat == PF_A8R8G8B8 )
            {
#if OGRE_PLATFORM >= OGRE_PLATFORM_ANDROID
                imageFormat = PF_A8B8G8R8;
#else
                imageFormat = PF_A8R8G8B8;
#endif
            }

            uint8 numMipmaps = 0;

            if( mDefaultTextureParameters[mapType].mipmaps )
            {
                uint32 heighestRes = std::max( std::max( image->getWidth(), image->getHeight() ),
                                               std::max<uint32>( image->getDepth(),
                                                                 image->getNumFaces() ) );
#if (ANDROID || (OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1800))
                numMipmaps = static_cast<uint8>( floorf( logf( static_cast<float>(heighestRes) ) /
                                                         logf( 2.0f ) ) );
#else
                numMipmaps = static_cast<uint8>( floorf( log2f( static_cast<float>(heighestRes) ) ) );
#endif
            }

            TextureType texType = TEX_TYPE_2D;
            uint width, height, depth, faces;
            uint8 baseMipLevel = 0;

            width   = image->getWidth();
            height  = image->getHeight();
            depth   = image->getDepth();
            faces   = image->getNumFaces();

            ushort maxResolution = caps->getMaximumResolution2D();

            if( image->hasFlag( IF_3D_TEXTURE ) )
            {
                maxResolution = caps->getMaximumResolution3D();
                texType = TEX_TYPE_3D;
            }
            else
            {
                if( image->hasFlag( IF_CUBEMAP ) )
                {
                    maxResolution = caps->getMaximumResolutionCubemap();
                    //TODO: Cubemap arrays supported since D3D10.1
                    texType = TEX_TYPE_CUBE_MAP;
                }
                else if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                {
                    //2D Texture Arrays
                    texType = TEX_TYPE_2D_ARRAY;
                }
            }

            if( !maxResolution )
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                             "Maximum resolution for this type of texture is 0.\n"
                             "Either a driver bug, or this GPU cannot support 2D/"
                             "Cubemap/3D texture: " + texName,
                             "HlmsTextureManager::createOrRetrieveTexture" );
            }

            //The texture is too big. Take a smaller mip.
            //If the texture doesn't have mipmaps, resize it.
            if( width > maxResolution || height > maxResolution )
            {
                bool resize = true;
                if( image->getNumMipmaps() )
                {
                    resize = false;
                    while( (width > maxResolution || height > maxResolution)
                           && (baseMipLevel <= image->getNumMipmaps()) )
                    {
                        width  >>= 1;
                        height >>= 1;
                        ++baseMipLevel;
                    }

                    if( (width > maxResolution || height > maxResolution) )
                        resize = true;
                }

                if( resize )
                {
                    baseMipLevel = 0;
                    Real aspectRatio = (Real)image->getWidth() / (Real)image->getHeight();
                    if( image->getWidth() >= image->getHeight() )
                    {
                        width  = maxResolution;
                        height = static_cast<uint>( floorf( maxResolution / aspectRatio ) );
                    }
                    else
                    {
                        width  = static_cast<uint>( floorf( maxResolution * aspectRatio ) );
                        height = maxResolution;
                    }

                    image->resize( width, height );
                }
            }

            if (image->getNumMipmaps() - baseMipLevel != (numMipmaps - baseMipLevel))
            {
                if (image->generateMipmaps(mDefaultTextureParameters[mapType].hwGammaCorrection) == false)
                {
                    //unable to generate preferred number of mipmaps, so use mipmaps of the input tex
                    numMipmaps = image->getNumMipmaps();

                    LogManager::getSingleton().logMessage(
                        "WARNING: Could not generate mipmaps for " + texName + ". "
                        "This can negatively impact performance as the HlmsTextureManager "
                        "will create more texture arrays than necessary, and the lower mips "
                        "won't be available. Lack of mipmaps also contribute to aliasing. "
                        "If this is a compressed DDS/PVR file, bake the mipmaps offline.",
                        LML_NORMAL );
                }
            }

            //Find an array where we can put it. If there is none, we'll have have to create a new one
            TextureArrayVec::iterator dstArrayIt = findSuitableArray( mapType, width, height, depth,
                                                                      faces, imageFormat,
                                                                      numMipmaps - baseMipLevel,
                                                                      uniqueSpecialId, aliasName );

            if( dstArrayIt == mTextureArrays[mapType].end() )
            {
                //Create a new array
                uint limit          = mDefaultTextureParameters[mapType].maxTexturesPerArray;
                uint limitSquared   = mDefaultTextureParameters[mapType].maxTexturesPerArray;
                bool packNonPow2    = mDefaultTextureParameters[mapType].packNonPow2;
                float packMaxRatio  = mDefaultTextureParameters[mapType].packMaxRatio;

                if( !packNonPow2 )
                {
                    if( !Bitwise::isPO2( width ) || !Bitwise::isPO2( height ) )
                        limit = limitSquared = 1;
                }

                if( width / (float)height >= packMaxRatio || height / (float)width >= packMaxRatio )
                    limit = limitSquared = 1;

                if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                {
                    limit = 1;

                    //Texture Arrays
                    if( texType == TEX_TYPE_3D || texType == TEX_TYPE_CUBE_MAP )
                    {
                        //APIs don't support arrays + 3D textures
                        //TODO: Cubemap arrays supported since D3D10.1
                        limitSquared = 1;
                    }
                    else if( texType == TEX_TYPE_2D_ARRAY )
                    {
                        size_t textureSizeNoMips = PixelUtil::getMemorySize( width, height, 1,
                                                                             imageFormat );

                        ThresholdVec::const_iterator itThres =  mDefaultTextureParameters[mapType].
                                                                    textureArraysTresholds.begin();
                        ThresholdVec::const_iterator enThres =  mDefaultTextureParameters[mapType].
                                                                    textureArraysTresholds.end();

                        while( itThres != enThres && textureSizeNoMips > itThres->minTextureSize )
                            ++itThres;

                        if( itThres == enThres )
                        {
                            itThres = mDefaultTextureParameters[mapType].
                                        textureArraysTresholds.end() - 1;
                        }

                        limitSquared = std::min<uint16>( limitSquared, itThres->maxTexturesPerArray );
                        depth = limitSquared;
                    }
                }
                else
                {
                    //UV Atlas
                    limit        = static_cast<uint>( ceilf( sqrtf( (Real)limitSquared ) ) );
                    limitSquared = limit * limit;

                    if( texType == TEX_TYPE_3D || texType == TEX_TYPE_CUBE_MAP )
                        limit = 1; //No UV atlas for 3D and Cubemaps

                    uint texWidth  = width  * limit;
                    uint texHeight = height * limit;

                    if( texWidth > maxResolution || texHeight > maxResolution )
                    {
                        limit = maxResolution / width;
                        limit = std::min<uint>( limit, maxResolution / height );

                        width  = width  * limit;
                        height = height * limit;
                    }

                    limitSquared = limit * limit;
                }

                TextureArray textureArray( limit, limitSquared, true,
                                           mDefaultTextureParameters[mapType].isNormalMap,
                                           false, uniqueSpecialId );

                textureArray.texture = TextureManager::getSingleton().createManual(
                                            "HlmsTextureManager/" +
                                            StringConverter::toString( mTextureId++ ),
                                            ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            texType, width, height, depth, numMipmaps - baseMipLevel,
                                            imageFormat,
                                            TU_DEFAULT & ~TU_AUTOMIPMAP, 0,
                                            mDefaultTextureParameters[mapType].hwGammaCorrection,
                                            0, BLANKSTRING, false );

                mTextureArrays[mapType].push_back( textureArray );
                dstArrayIt = mTextureArrays[mapType].end() - 1;
            }

            uint16 entryIdx = dstArrayIt->createEntry();
            uint16 arrayIdx = dstArrayIt - mTextureArrays[mapType].begin();

            dstArrayIt->entries[entryIdx] = TextureArray::NamePair( aliasName, texName );
            it = mEntries.insert( it, TextureEntry( searchName.name, mapType, arrayIdx, entryIdx ) );

            try
            {
                if( texType != TEX_TYPE_3D && texType != TEX_TYPE_CUBE_MAP )
                {
                    if( mDefaultTextureParameters[mapType].packingMethod == TextureArrays )
                    {
                        copyTextureToArray( *image, dstArrayIt->texture, entryIdx,
                                            baseMipLevel, dstArrayIt->isNormalMap );
                    }
                    else
                    {
                        copyTextureToAtlas( *image, dstArrayIt->texture, entryIdx,
                                            dstArrayIt->sqrtMaxTextures, baseMipLevel,
                                            dstArrayIt->isNormalMap );
                    }
                }
                else
                {
                    copy3DTexture( *image, dstArrayIt->texture, 0,
                                   std::max<uint32>( image->getNumFaces(), image->getDepth() ),
                                   baseMipLevel );
                }
            }
            catch( Exception &e )
            {
                destroyTexture(aliasName);
                throw;
            }
        }

        const TextureArray &texArray = mTextureArrays[it->mapType][it->arrayIdx];

        if( !missingFromCache && uniqueSpecialId && uniqueSpecialId == texArray.uniqueSpecialId )
        {
            LogManager::getSingleton().logMessage(
                        "Texture " + aliasName + " was requested with special ID, but it "
                        "was already loaded in a pool with a different ID! You should ensure the "
                        "texture is first loaded with the desired pool ID, or you can use a different "
                        "alias name to clone it and have it in two different pools",
                    LML_CRITICAL );
        }

        retVal.texture = texArray.texture;

        if( !texArray.texture->isTextureTypeArray() )
        {
            retVal.xIdx = it->entryIdx % texArray.sqrtMaxTextures;
            retVal.yIdx = it->entryIdx / texArray.sqrtMaxTextures;
            retVal.divisor= texArray.sqrtMaxTextures;
        }
        else
        {
            retVal.xIdx = it->entryIdx;
            retVal.yIdx = 0;
            retVal.divisor= 1;
        }
        }
        catch( Exception &e )
        {
            LogManager::getSingleton().logMessage( LML_CRITICAL, e.getFullDescription() );

            if( e.getNumber() != Exception::ERR_FILE_NOT_FOUND )
                throw;
            else
            {
                retVal.texture  = mBlankTexture;
                retVal.xIdx     = 0;
                retVal.yIdx     = 0;
                retVal.divisor  = 1;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::destroyTexture( IdString aliasName )
    {
        TextureEntry searchName( aliasName );
        TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(), searchName );

        if( it != mEntries.end() && it->name == searchName.name )
        {
            TextureArrayVec::iterator texArrayIt = mTextureArrays[it->mapType].begin() + it->arrayIdx;
            texArrayIt->destroyEntry( it->entryIdx );

            if( texArrayIt->activeEntries == 0 )
            {
                //The whole array has no actual content. Destroy the texture.
                ResourcePtr texResource = texArrayIt->texture;
                TextureManager::getSingleton().remove( texResource );
                texArrayIt = efficientVectorRemove( mTextureArrays[it->mapType], texArrayIt );

                if( texArrayIt != mTextureArrays[it->mapType].end() )
                {
                    //The last element has now a new index. Update the references in mEntries
                    const size_t newArrayIdx = texArrayIt - mTextureArrays[it->mapType].begin();
                    TextureArray::NamePairVec::const_iterator itor = texArrayIt->entries.begin();
                    TextureArray::NamePairVec::const_iterator end  = texArrayIt->entries.end();

                    while( itor != end )
                    {
                        if( !itor->aliasName.empty() )
                        {
                            searchName.name = itor->aliasName;
                            TextureEntryVec::iterator itEntry = std::lower_bound( mEntries.begin(),
                                                                                  mEntries.end(),
                                                                                  searchName );
                            assert( itEntry != mEntries.end() && itEntry->name == searchName.name );
                            itEntry->arrayIdx = newArrayIdx;
                        }
                        ++itor;
                    }
                }
            }

            mEntries.erase( it );
        }
    }
    //-----------------------------------------------------------------------------------
    const String* HlmsTextureManager::findAliasName( const TextureLocation &textureLocation ) const
    {
        const String *retVal = 0;

        for( size_t i=0; i<NUM_TEXTURE_TYPES && !retVal; ++i )
        {
            TextureArrayVec::const_iterator itor = mTextureArrays[i].begin();
            TextureArrayVec::const_iterator end  = mTextureArrays[i].end();

            while( itor != end && !retVal )
            {
                if( itor->texture == textureLocation.texture )
                {
                    size_t idx = textureLocation.yIdx * itor->sqrtMaxTextures + textureLocation.xIdx;

                    if( idx < itor->entries.size() )
                        retVal = &itor->entries[idx].aliasName;
                }

                ++itor;
            }
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    const String* HlmsTextureManager::findResourceNameFromAlias( IdString aliasName ) const
    {
        uint32 poolId = 0;
        return findResourceNameFromAlias( aliasName, poolId );
    }
    //-----------------------------------------------------------------------------------
    const String* HlmsTextureManager::findResourceNameFromAlias( IdString aliasName,
                                                                 uint32 &outPoolId ) const
    {
        const String *retVal = 0;

        TextureEntry searchName( aliasName );
        TextureEntryVec::const_iterator it = std::lower_bound( mEntries.begin(), mEntries.end(),
                                                               searchName );

        if( it != mEntries.end() && it->name == searchName.name )
        {
            const TextureArray &texArray = mTextureArrays[it->mapType][it->arrayIdx];
            retVal = &texArray.entries[it->entryIdx].resourceName;
            outPoolId = texArray.uniqueSpecialId;
        }

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    bool HlmsTextureManager::getTexturePackParameters( const HlmsTexturePack &pack, uint32 &outWidth,
                                                       uint32 &outHeight, uint32 &outDepth,
                                                       PixelFormat &outPixelFormat ) const
    {
        HlmsTexturePack::TextureEntryVec::const_iterator itor = pack.textureEntry.begin();
        HlmsTexturePack::TextureEntryVec::const_iterator end  = pack.textureEntry.end();

        while( itor != end )
        {
            const HlmsTexturePack::TextureEntry &texInfo = *itor;

            StringVector::const_iterator itPath = texInfo.paths.begin();
            StringVector::const_iterator enPath = texInfo.paths.end();

            while( itPath != enPath )
            {
                try
                {
                    Image image;
                    image.load( *itPath, ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
                    outWidth    = image.getWidth();
                    outHeight   = image.getHeight();
                    outDepth    = std::max<uint32>( image.getDepth(), image.getNumFaces() );
                    outPixelFormat = image.getFormat();

                    return true;
                }
                catch( ... )
                {
                }

                ++itPath;
            }

            ++itor;
        }

        return false;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::createFromTexturePack( const HlmsTexturePack &pack )
    {
        uint32 width = 0, height = 0, depth = 0;
        PixelFormat pixelFormat;
        uint8 numMipmaps = 0;

        if( !getTexturePackParameters( pack, width, height, depth, pixelFormat ) )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Could not derive the texture properties "
                         "for texture pack '" + pack.name + "'",
                         "HlmsTextureManager::createFromTexturePack" );
        }

        if( pack.pixelFormat != PF_UNKNOWN )
        {
            pixelFormat = pack.pixelFormat;
        }
        else
        {
            if( pixelFormat == PF_X8R8G8B8 || pixelFormat == PF_R8G8B8 ||
                pixelFormat == PF_X8B8G8R8 || pixelFormat == PF_B8G8R8 ||
                pixelFormat == PF_A8R8G8B8 )
            {
                pixelFormat = PF_A8B8G8R8;
            }
        }

        if( pack.hasMipmaps )
        {
            uint32 heighestRes = std::max( std::max( width, height ), depth );
#if (ANDROID || (OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1800))
            numMipmaps = static_cast<uint8>( floorf( logf( static_cast<float>(heighestRes) ) / logf( 2.0f ) ) );
#else
            numMipmaps = static_cast<uint8>( floorf( log2f( static_cast<float>(heighestRes) ) ) );
#endif
        }

        if( pack.textureType == TEX_TYPE_CUBE_MAP )
        {
            HlmsTexturePack::TextureEntryVec::const_iterator itor = pack.textureEntry.begin();
            HlmsTexturePack::TextureEntryVec::const_iterator end  = pack.textureEntry.end();

            while( itor != end )
            {
                const HlmsTexturePack::TextureEntry &texInfo = *itor;

                TextureEntry searchName( texInfo.name );
                TextureEntryVec::iterator it = std::lower_bound( mEntries.begin(), mEntries.end(),
                                                                 searchName );

                if( it != mEntries.end() && it->name == searchName.name )
                {
                    LogManager::getSingleton().logMessage( "ERROR: A texture by the name '" +
                                                           texInfo.name  + "' already exists!" );
                    ++itor;
                    continue;
                }

                assert( !texInfo.paths.empty() ) ;

                if( texInfo.paths.size() != 1 )
                {
                    //Multiple files
                    assert( !(texInfo.paths.size() % 6) &&
                            "For cubemaps, the number of files must be multiple of 6!" );

                    Image cubeMap;
                    size_t cubeMapSize = PixelUtil::getMemorySize( width, height, 6, pixelFormat );
                    cubeMap.loadDynamicImage( OGRE_ALLOC_T( uchar, cubeMapSize, MEMCATEGORY_GENERAL ),
                                              width, height, 1, pixelFormat, true, 6 );
                    for( size_t i=0; i<6; ++i )
                    {
                        Image image;
                        image.load( texInfo.paths[i],
                                    ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );

                        if( image.getWidth() != width && image.getHeight() != height )
                        {
                            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, texInfo.paths[i] +
                                         ": All textures in the same pack must have the "
                                         "same resolution!",
                                         "HlmsTextureManager::createFromTexturePack" );
                        }

                        PixelUtil::bulkPixelConversion( image.getPixelBox( 0 ), cubeMap.getPixelBox( i ) );
                    }

                    TextureArray textureArray( 1, 1, false, false, false, 0 );

                    textureArray.texture = TextureManager::getSingleton().createManual(
                                                "HlmsTextureManager/" +
                                                StringConverter::toString( mTextureId++ ),
                                                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                                pack.textureType, width, height, depth, numMipmaps,
                                                pixelFormat, TU_DEFAULT & ~TU_AUTOMIPMAP, 0,
                                                pack.hwGammaCorrection, 0, BLANKSTRING, false );

                    if( pack.hasMipmaps )
                    {
                        if( !cubeMap.generateMipmaps( pack.hwGammaCorrection, Image::FILTER_GAUSSIAN ) )
                        {
                            LogManager::getSingleton().logMessage( "Couldn't generate mipmaps for '" +
                                                                    texInfo.name + "'", LML_CRITICAL );
                        }

                        if( !pack.exportLocation.empty() )
                            cubeMap.save( pack.exportLocation + "/" + texInfo.name + ".dds" );
                    }

                    copy3DTexture( cubeMap, textureArray.texture, 0, 6, 0 );

                    it = mEntries.insert( it, TextureEntry( searchName.name, TEXTURE_TYPE_ENV_MAP,
                                                            mTextureArrays[TEXTURE_TYPE_ENV_MAP].size(), 0 ) );

                    textureArray.entries.push_back( TextureArray::NamePair( texInfo.name,
                                                                            texInfo.name ) );
                    mTextureArrays[TEXTURE_TYPE_ENV_MAP].push_back( textureArray );
                }
                else
                {
                    OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                                 "Oops! Work in Progress, sorry!",
                                 "HlmsTextureManager::createFromTexturePack" );
                    //TODO
                    /*if( image.getNumMipmaps() != numMipmaps )
                    {
                        image.generateMipmaps();
                    }*/
                }

                ++itor;
            }
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Oops! Work in Progress, sorry!",
                         "HlmsTextureManager::createFromTexturePack" );
        }
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::saveTexture( const HlmsTextureManager::TextureLocation &texLocation,
                                          const String &folderPath, set<String>::type &savedTextures,
                                          bool saveOitd, bool saveOriginal,
                                          uint32 slice, uint32 numSlices,
                                          HlmsTextureExportListener *listener )
    {
#if 0
        //Render Targets are... complicated. Let's not, for now.
        if( texLocation.texture->getUsage() & TU_RENDERTARGET )
            return;

        const String *aliasNamePtr = findAliasName( texLocation );
        const String aliasName = aliasNamePtr ? *aliasNamePtr : texLocation.texture->getName();

        if( savedTextures.find( aliasName ) != savedTextures.end() )
            return; //Texture already saved

        DataStreamPtr inFile;
        if( saveOriginal )
        {
            String resourceName;
            if( aliasNamePtr )
            {
                const String *resNamePtr = findResourceNameFromAlias( aliasName );
                if( resNamePtr )
                    resourceName = *resNamePtr;
                else
                    resourceName = aliasName;
            }
            else
                resourceName = aliasName;

            String savingFilename = aliasName;
            if( listener )
                listener->savingChangeTextureNameOriginal( aliasName, resourceName, savingFilename );
            try
            {
                inFile = ResourceGroupManager::getSingleton().openResource(
                             resourceName, texLocation.texture->getGroup() );
            }
            catch( FileNotFoundException &e )
            {
                //Try opening as an absolute path
                std::fstream *ifs = OGRE_NEW_T( std::fstream, MEMCATEGORY_GENERAL )(
                                        resourceName.c_str(),
                                        std::ios::binary|std::ios::in );

                if( ifs->is_open() )
                {
                    inFile = DataStreamPtr( OGRE_NEW FileStreamDataStream( resourceName, ifs, true ) );
                }
                else
                {
                    LogManager::getSingleton().logMessage(
                                "WARNING: Could not find texture file " + aliasName +
                                " (" + resourceName + ") for copying to export location. "
                                "Error: " + e.getFullDescription() );
                }
            }
            catch( Exception &e )
            {
                LogManager::getSingleton().logMessage(
                            "WARNING: Could not find texture file " + aliasName +
                            " (" + resourceName + ") for copying to export location. "
                            "Error: " + e.getFullDescription() );
            }

            if( inFile )
            {
                size_t fileSize = inFile->size();
                vector<uint8>::type fileData;
                fileData.resize( fileSize );
                inFile->read( &fileData[0], fileData.size() );
                std::ofstream outFile( (folderPath + "/" + savingFilename).c_str(),
                                       std::ios::binary | std::ios::out );
                outFile.write( (const char*)&fileData[0], fileData.size() );
                outFile.close();
            }
        }

        if( saveOitd )
        {
            String texName = aliasName;
            if( listener )
                listener->savingChangeTextureNameOitd( aliasName, texName );

            Image image;
            texLocation.texture->convertToImage( image, true, 0u, slice, numSlices );

            image.save( folderPath + "/" + texName + ".oitd" );
        }

        savedTextures.insert( aliasName );
#endif
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::MetadataCacheEntry::MetadataCacheEntry() :
        mapType( TEXTURE_TYPE_DIFFUSE ),
        poolId( 0 )
    {
    }
    //-----------------------------------------------------------------------------------
    const HlmsTextureManager::MetadataCacheEntry* HlmsTextureManager::getMetadataCacheEntry(
            IdString aliasName ) const
    {
        MetadataCacheEntry const *retVal = 0;
        MetadataCacheMap::const_iterator itor = mMetadataCache.find( aliasName );
        if( itor != mMetadataCache.end() )
            retVal = &itor->second;
        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::importTextureMetadataCache( const String &filename, const char *jsonString )
    {
#if !OGRE_NO_JSON
        rapidjson::Document d;
        d.Parse( jsonString );

        if( d.HasParseError() )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                         "HlmsTextureManager::importTextureMetadataCache",
                         "Invalid JSON string in file " + filename + " at line " +
                         StringConverter::toString( d.GetErrorOffset() ) + " Reason: " +
                         rapidjson::GetParseError_En( d.GetParseError() ) );
        }

        rapidjson::Value::ConstMemberIterator itor;
        itor = d.FindMember( "reserved_pool_ids" );

        if( itor != d.MemberEnd() && itor->value.IsArray() )
        {
            const rapidjson::Value &jsonVal = itor->value;
            const rapidjson::SizeType arraySize = jsonVal.Size();
            for( rapidjson::SizeType i=0; i<arraySize; ++i )
            {
                if( jsonVal[i].IsObject() )
                {
                    uint32 poolId = 0;
                    itor = jsonVal[i].FindMember( "poolId" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsUint() )
                        poolId = itor->value.GetUint();

                    TextureMapType mapType = TEXTURE_TYPE_DIFFUSE;
                    itor = jsonVal[i].FindMember( "mapType" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsUint() )
                    {
                        mapType = static_cast<TextureMapType>(
                                      Math::Clamp<uint32>( itor->value.GetUint(),
                                                           0u, NUM_TEXTURE_TYPES ) );
                    }

                    uint32 width = 0, height = 0;
                    uint16 numSlices = 0;
                    itor = jsonVal[i].FindMember( "resolution" );
                    if( itor != jsonVal[i].MemberEnd() &&
                        itor->value.IsArray() && itor->value.Size() >= 3u &&
                        itor->value[0].IsUint() && itor->value[1].IsUint() && itor->value[2].IsUint() )
                    {
                        width = itor->value[0].GetUint();
                        height = itor->value[1].GetUint();
                        numSlices = static_cast<uint16>( itor->value[2].GetUint() );
                    }

                    uint8 mipmaps = 0;
                    itor = jsonVal[i].FindMember( "mipmaps" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsUint() )
                        mipmaps = static_cast<uint8>( itor->value.GetUint() - 1u );

                    PixelFormat pixelFormat = PF_UNKNOWN;
                    itor = jsonVal[i].FindMember( "format" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsString() )
                        pixelFormat = PixelUtil::getFormatFromName( itor->value.GetString() );

                    bool isNormalMap = false;
                    itor = jsonVal[i].FindMember( "normal_map" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsBool() )
                        isNormalMap = itor->value.GetBool();

                    bool hwGamma = false;
                    itor = jsonVal[i].FindMember( "sRGB" );
                    if( itor != jsonVal[i].MemberEnd() && itor->value.IsBool() )
                        hwGamma = itor->value.GetBool();

                    if( width > 0u && height > 0u && numSlices > 0u && pixelFormat != PF_UNKNOWN &&
                        !hasPoolId( poolId, mapType ) )
                    {
                        reservePoolId( poolId, mapType, width, height, numSlices,
                                       mipmaps, pixelFormat, isNormalMap, hwGamma );
                    }
                }
            }
        }

        itor = d.FindMember( "textures" );
        if( itor != d.MemberEnd() && itor->value.IsObject() )
        {
            rapidjson::Value::ConstMemberIterator itTex = itor->value.MemberBegin();
            rapidjson::Value::ConstMemberIterator enTex = itor->value.MemberEnd();

            while( itTex != enTex )
            {
                if( itTex->value.IsObject() )
                {
                    MetadataCacheEntry cacheEntry;
                    IdString aliasName = itTex->name.GetString();

                    itor = itTex->value.FindMember( "type" );
                    if( itor != itTex->value.MemberEnd() && itor->value.IsUint() )
                    {
                        cacheEntry.mapType = static_cast<TextureMapType>(
                                                 Math::Clamp<uint32>( itor->value.GetUint(),
                                                                      0u, NUM_TEXTURE_TYPES ) );
                    }

                    itor = itTex->value.FindMember( "poolId" );
                    if( itor != itTex->value.MemberEnd() && itor->value.IsUint() )
                        cacheEntry.poolId = itor->value.GetUint();

                    mMetadataCache[aliasName] = cacheEntry;
                }

                ++itTex;
            }
        }
#else
        OGRE_EXCEPT( Exception::ERR_INVALID_CALL,
                     "Ogre must be built with JSON support to call this function!",
                     "HlmsTextureManager::importTextureMetadataCache" );
#endif
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::exportTextureMetadataCache( String &outJson )
    {
        char tmpBuffer[4096];
        LwString jsonStr( LwString::FromEmptyPointer( tmpBuffer, sizeof(tmpBuffer) ) );

        jsonStr.a( "{" );
        jsonStr.a( "\n\t\"reserved_pool_ids\" :\n\t[" );

        bool firstIteration = true;
        for( int i=0; i<NUM_TEXTURE_TYPES; ++i )
        {
            TextureArrayVec::const_iterator itor = mTextureArrays[i].begin();
            TextureArrayVec::const_iterator end  = mTextureArrays[i].end();

            while( itor != end )
            {
                const TextureArray &texArray = *itor;
                if( texArray.manuallyReserved )
                {
                    if( !firstIteration )
                        jsonStr.a( "," );
                    jsonStr.a( "\n\t\t{\n\t\t\t\"poolId\" : ", texArray.uniqueSpecialId );
                    jsonStr.a( ",\n\t\t\t\"mapType\" : ", i );
                    jsonStr.a( ",\n\t\t\t\"resolution\" : [",
                               texArray.texture->getWidth(), ", ",
                               texArray.texture->getHeight(), ", ",
                               texArray.texture->getDepth(), "]" );
                    jsonStr.a( ",\n\t\t\t\"mipmaps\" : ", texArray.texture->getNumMipmaps() + 1u );
                    jsonStr.a( ",\n\t\t\t\"format\" : \"",
                               PixelUtil::getFormatName( texArray.texture->getFormat() ).c_str(), "\"" );
                    if( texArray.isNormalMap )
                        jsonStr.a( ",\n\t\t\t\"normal_map\" : true" );
                    if( texArray.texture->isHardwareGammaEnabled() )
                        jsonStr.a( ",\n\t\t\t\"sRGB\" : true" );

                    jsonStr.a( "\n\t\t}" );
                    firstIteration = false;

                    outJson += jsonStr.c_str();
                    jsonStr.clear();
                }
                ++itor;
            }
        }

        jsonStr.a( "\n\t],\n\t\"textures\" :\n\t{" );
        firstIteration = true;
        TextureEntryVec::const_iterator itor = mEntries.begin();
        TextureEntryVec::const_iterator end  = mEntries.end();

        while( itor != end )
        {
            const TextureArray & texArray = mTextureArrays[itor->mapType][itor->arrayIdx];
            if( !firstIteration )
                jsonStr.a( "," );

            jsonStr.a( "\n\t\t\"", texArray.entries[itor->entryIdx].aliasName.c_str(), "\" : \n\t\t{" );
            jsonStr.a( "\n\t\t\t\"type\" : ", itor->mapType );
            jsonStr.a( ",\n\t\t\t\"poolId\" : ", texArray.uniqueSpecialId );
            jsonStr.a( "\n\t\t}" );

            outJson += jsonStr.c_str();
            jsonStr.clear();

            firstIteration = false;

            ++itor;
        }
        jsonStr.a( "\n\t}\n}" );
        outJson += jsonStr.c_str();
        jsonStr.clear();
    }
    //-----------------------------------------------------------------------------------
    HlmsTextureManager::TextureLocation HlmsTextureManager::getBlankTexture(void) const
    {
        TextureLocation retVal;
        retVal.texture  = mBlankTexture;
        retVal.xIdx     = 0;
        retVal.yIdx     = 0;
        retVal.divisor  = 1;

        return retVal;
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::clearTextureMetadataCache(void)
    {
        mMetadataCache.clear();
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::dumpMemoryUsage( Log* log ) const
    {
        const char *typeNames[NUM_TEXTURE_TYPES] =
        {
            "DIFFUSE",
            "MONOCHROME",
            "NORMALS",
            "ENV_MAP",
            "DETAIL",
            "DETAIL_NORMAL_MAP",
			"NON_COLOR_DATA"
        };

        size_t bytesPerCategory[NUM_TEXTURE_TYPES];
        memset( bytesPerCategory, 0, sizeof( bytesPerCategory ) );

        Log* logActual = log == NULL ? LogManager::getSingleton().getDefaultLog() : log;

        logActual->logMessage(
                    "================================"
                    "Start dump of HlmsTextureManager"
                    "================================",
                    LML_CRITICAL );

        logActual->logMessage(
                    "|#|Type|Width|Height|Depth|Format|HW Gamma|Mipmaps|Size in bytes|"
                    "Num. active textures|Total texture capacity|Texture Names",
                    LML_CRITICAL );

        for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
        {
            TextureArrayVec::const_iterator itor = mTextureArrays[i].begin();
            TextureArrayVec::const_iterator end  = mTextureArrays[i].end();

            String row;

            while( itor != end )
            {
                size_t textureSize = 0;

                uint32 width    = itor->texture->getWidth();
                uint32 height   = itor->texture->getHeight();
                uint32 depth    = itor->texture->getDepth();

                for( size_t j=0; j<(size_t)(itor->texture->getNumMipmaps() + 1); ++j )
                {
                    textureSize += PixelUtil::getMemorySize( width, height, depth,
                                                             itor->texture->getFormat() ) *
                                    itor->texture->getNumFaces();

                    width   = std::max<uint32>( width >> 1, 1 );
                    height  = std::max<uint32>( height >> 1, 1 );
                    if( !itor->texture->isTextureTypeArray() )
                        depth = std::max<uint32>( depth >> 1, 1 );
                }

                row += "|";
                row += StringConverter::toString( (size_t)(itor - mTextureArrays[i].begin()) ) + "|";
                row += String( typeNames[i] ) + "|";
                row += StringConverter::toString( itor->texture->getWidth() ) + "|";
                row += StringConverter::toString( itor->texture->getHeight() ) + "|";
                row += StringConverter::toString( itor->texture->getDepth() ) + "|";
                row += PixelUtil::getFormatName( itor->texture->getFormat() ) + "|";
                row += itor->texture->isHardwareGammaEnabled() ? "Yes|" : "No|";
                row += StringConverter::toString( itor->texture->getNumMipmaps() ) + "|";
                row += StringConverter::toString( textureSize ) + "|";
                row += StringConverter::toString( itor->activeEntries ) + "|";
                row += StringConverter::toString( itor->entries.size() );

                TextureArray::NamePairVec::const_iterator itEntry = itor->entries.begin();
                TextureArray::NamePairVec::const_iterator enEntry = itor->entries.end();

                while( itEntry != enEntry )
                {
                    row += "|" + itEntry->aliasName;
                    ++itEntry;
                }

                logActual->logMessage( row, LML_CRITICAL );
                row.clear();

                bytesPerCategory[i] += textureSize;

                ++itor;
            }
        }

        logActual->logMessage( "|Size in MBs per category:", LML_CRITICAL );

        size_t totalBytes = 0;
        for( size_t i=0; i<NUM_TEXTURE_TYPES; ++i )
        {
            logActual->logMessage( "|" + String( typeNames[i] ) + "|" +
                                   StringConverter::toString( bytesPerCategory[i] /
                                                              (1024.0f * 1024.0f) ),
                                   LML_CRITICAL );

            totalBytes += bytesPerCategory[i];
        }

        logActual->logMessage( "|Total MBs used:|" + StringConverter::toString( totalBytes /
                                                                                (1024.0f * 1024.0f) ),
                               LML_CRITICAL );
        logActual->logMessage(
                    "================================"
                    "End dump of HlmsTextureManager"
                    "================================",
                    LML_CRITICAL );
    }
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    //-----------------------------------------------------------------------------------
    uint16 HlmsTextureManager::TextureArray::createEntry(void)
    {
        assert( activeEntries < maxTextures );
        ++activeEntries;

        TextureArray::NamePairVec::const_iterator itor = entries.begin();
        TextureArray::NamePairVec::const_iterator end  = entries.end();

        while( itor != end && !itor->aliasName.empty() )
            ++itor;

        return static_cast<uint16>( itor - entries.begin() );
    }
    //-----------------------------------------------------------------------------------
    void HlmsTextureManager::TextureArray::destroyEntry( uint16 entry )
    {
        assert( activeEntries != 0 );
        --activeEntries;

        entries[entry].aliasName.clear();
        entries[entry].resourceName.clear();
    }
    //-----------------------------------------------------------------------------------
}
