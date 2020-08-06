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
#include "OgrePixelFormat.h"
#include "OgreBitwise.h"
#include "OgreColourValue.h"
#include "OgreException.h"
#include "OgrePixelFormatDescriptions.h"
#include "OgreStringConverter.h"

#include "OgrePixelBox.h"

#include "OgreProfiler.h"

namespace {
#include "OgrePixelConversions.h"
}

namespace Ogre {

    //-----------------------------------------------------------------------
    void PixelBox::setConsecutive()
    {
        if( PixelUtil::isCompressed( format ) )
        {
            rowPitch    = PixelUtil::getMemorySize( getWidth(), 1, 1, format );
            slicePitch  = PixelUtil::getMemorySize( getWidth(), getHeight(), 1, format );
        }
        else
        {
            rowPitch = getWidth();
            slicePitch = getWidth()*getHeight();
        }
    }
    //-----------------------------------------------------------------------
    size_t PixelBox::getRowSkip() const
    {
        if( PixelUtil::isCompressed( format ) )
        {
            return rowPitch - PixelUtil::getMemorySize( getWidth(), 1, 1, format );
        }
        else
        {
            return rowPitch - getWidth();
        }
    }
    //-----------------------------------------------------------------------
    size_t PixelBox::rowPitchAlwaysBytes() const
    {
        if( PixelUtil::isCompressed( format ) )
        {
            return rowPitch;
        }
        else
        {
            return rowPitch * PixelUtil::getNumElemBytes( format );
        }
    }
    //-----------------------------------------------------------------------
    size_t PixelBox::slicePitchAlwaysBytes(void) const
    {
        if( PixelUtil::isCompressed( format ) )
        {
            return slicePitch;
        }
        else
        {
            return slicePitch * PixelUtil::getNumElemBytes( format );
        }
    }
    //-----------------------------------------------------------------------
    size_t PixelBox::getSliceSkipAlwaysBytes() const
    {
        if( PixelUtil::isCompressed( format ) )
        {
            return getSliceSkip();
        }
        else
        {
            return getSliceSkip() * PixelUtil::getNumElemBytes( format );
        }
    }
    //-----------------------------------------------------------------------
    bool PixelBox::isConsecutive() const
    {
        if( PixelUtil::isCompressed( format ) )
        {
            return rowPitch == PixelUtil::getMemorySize( getWidth(), 1, 1, format ) &&
                    slicePitch == PixelUtil::getMemorySize( getWidth(), getHeight(), 1, format );
        }
        else
        {
            return rowPitch == getWidth() && slicePitch == getWidth()*getHeight();
        }
    }
    //-----------------------------------------------------------------------
    size_t PixelBox::getConsecutiveSize() const
    {
        return PixelUtil::getMemorySize(getWidth(), getHeight(), getDepth(), format);
    }
    PixelBox PixelBox::getSubVolume(const Box &def, bool resetOrigin /* = true */) const
    {
        if(!contains(def))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bounds out of range", "PixelBox::getSubVolume");

        if(PixelUtil::isCompressed(format) && (def.left != left || def.top != top || def.right != right || def.bottom != bottom))
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Cannot return subvolume of compressed PixelBuffer with less than slice granularity", "PixelBox::getSubVolume");

        // Calculate new pixelbox and optionally reset origin.
        PixelBox rval(def, format, data);
        rval.rowPitch = rowPitch;
        rval.slicePitch = slicePitch;

        if(resetOrigin)
        {
            if(PixelUtil::isCompressed(format))
            {
                if(rval.front > 0)
                {
                    rval.data = (uint8*)rval.data + rval.front * PixelUtil::getMemorySize(getWidth(), getHeight(), 1, format);
                    rval.back -= rval.front;
                    rval.front = 0;
                }
            }
            else
            {
                rval.data = rval.getTopLeftFrontPixelPtr();
                rval.right -= rval.left;
                rval.bottom -= rval.top;
                rval.back -= rval.front;
                rval.front = rval.top = rval.left = 0;
            }
        }

        return rval;
    }
    void* PixelBox::getTopLeftFrontPixelPtr() const
    {
        return (uint8*)data + (left + top * rowPitch + front * slicePitch) * PixelUtil::getNumElemBytes(format);
    }
    //-----------------------------------------------------------------------
    /**
    * Directly get the description record for provided pixel format. For debug builds,
    * this checks the bounds of fmt with an assertion.
    */
    static inline const PixelFormatDescription &getDescriptionFor(const PixelFormat fmt)
    {
        const int ord = (int)fmt;
        assert(ord>=0 && ord<PF_COUNT);

        return _pixelFormats[ord];
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getNumElemBytes( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes;
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::calculateSizeBytes( uint32 width, uint32 height, uint32 depth,
                                          uint32 slices, PixelFormat format, uint8 numMipmaps )
    {
        OGRE_ASSERT_LOW( numMipmaps > 0 );

        size_t totalBytes = 0;
        while( (width > 1u || height > 1u || depth > 1u) && numMipmaps > 0 )
        {
            totalBytes += PixelUtil::getMemorySize( width, height, depth * slices, format );
            width   = std::max( 1u, width  >> 1u );
            height  = std::max( 1u, height >> 1u );
            depth   = std::max( 1u, depth  >> 1u );
            --numMipmaps;
        }

        if( width == 1u && height == 1u && depth == 1u && numMipmaps > 0 )
        {
            //Add 1x1x1 mip.
            totalBytes += PixelUtil::getMemorySize( width, height, depth * slices, format );
            --numMipmaps;
        }

        return totalBytes;
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getMemorySize(uint32 width, uint32 height, uint32 depth, PixelFormat format)
    {
        if(isCompressed(format))
        {
            switch(format)
            {
                // DXT formats work by dividing the image into 4x4 blocks, then encoding each
                // 4x4 block with a certain number of bytes. 
                case PF_DXT1:
                    return ((width+3)/4)*((height+3)/4)*8 * depth;
                case PF_DXT2:
                case PF_DXT3:
                case PF_DXT4:
                case PF_DXT5:
                    return ((width+3)/4)*((height+3)/4)*16 * depth;
                case PF_BC4_SNORM:
                case PF_BC4_UNORM:
                    return ((width+3)/4)*((height+3)/4)*8 * depth;
                case PF_BC5_SNORM:
                case PF_BC5_UNORM:
                case PF_BC6H_SF16:
                case PF_BC6H_UF16:
                case PF_BC7_UNORM:
                case PF_BC7_UNORM_SRGB:
                    return ((width+3)/4)*((height+3)/4)*16 * depth;

                // Size calculations from the PVRTC OpenGL extension spec
                // http://www.khronos.org/registry/gles/extensions/IMG/IMG_texture_compression_pvrtc.txt
                // Basically, 32 bytes is the minimum texture size.  Smaller textures are padded up to 32 bytes
                case PF_PVRTC_RGB2:
                case PF_PVRTC_RGBA2:
                case PF_PVRTC2_2BPP:
                    return (std::max((int)width, 16) * std::max((int)height, 8) * 2 + 7) / 8;
                case PF_PVRTC_RGB4:
                case PF_PVRTC_RGBA4:
                case PF_PVRTC2_4BPP:
                    return (std::max((int)width, 8) * std::max((int)height, 8) * 4 + 7) / 8;

                // Size calculations from the ETC spec
                // https://www.khronos.org/registry/OpenGL/extensions/OES/OES_compressed_ETC1_RGB8_texture.txt
                case PF_ETC1_RGB8:
                case PF_ETC2_RGB8:
                case PF_ETC2_RGBA8:
                case PF_ETC2_RGB8A1:
                    return ((width + 3) / 4) * ((height + 3) / 4) * 8;

                case PF_ATC_RGB:
                    return ((width + 3) / 4) * ((height + 3) / 4) * 8;
                case PF_ATC_RGBA_EXPLICIT_ALPHA:
                case PF_ATC_RGBA_INTERPOLATED_ALPHA:
                    return ((width + 3) / 4) * ((height + 3) / 4) * 16;

                case PF_ASTC_RGBA_4X4_LDR:
                case PF_ASTC_SRGB8A8_4X4_LDR:
                case PF_ASTC_RGBA_5X4_LDR:
                case PF_ASTC_SRGB8A8_5X4_LDR:
                case PF_ASTC_RGBA_5X5_LDR:
                case PF_ASTC_SRGB8A8_5X5_LDR:
                case PF_ASTC_RGBA_6X5_LDR:
                case PF_ASTC_SRGB8A8_6X5_LDR:
                case PF_ASTC_RGBA_6X6_LDR:
                case PF_ASTC_SRGB8A8_6X6_LDR:
                case PF_ASTC_RGBA_8X5_LDR:
                case PF_ASTC_SRGB8A8_8X5_LDR:
                case PF_ASTC_RGBA_8X6_LDR:
                case PF_ASTC_SRGB8A8_8X6_LDR:
                case PF_ASTC_RGBA_8X8_LDR:
                case PF_ASTC_SRGB8A8_8X8_LDR:
                case PF_ASTC_RGBA_10X5_LDR:
                case PF_ASTC_SRGB8A8_10X5_LDR:
                case PF_ASTC_RGBA_10X6_LDR:
                case PF_ASTC_SRGB8A8_10X6_LDR:
                case PF_ASTC_RGBA_10X8_LDR:
                case PF_ASTC_SRGB8A8_10X8_LDR:
                case PF_ASTC_RGBA_10X10_LDR:
                case PF_ASTC_SRGB8A8_10X10_LDR:
                case PF_ASTC_RGBA_12X10_LDR:
                case PF_ASTC_SRGB8A8_12X10_LDR:
                case PF_ASTC_RGBA_12X12_LDR:
                case PF_ASTC_SRGB8A8_12X12_LDR:
                {
                    uint32 blockWidth = getCompressedBlockWidth( format );
                    uint32 blockHeight = getCompressedBlockWidth( format );
                    return  (alignToNextMultiple( width, blockWidth ) / blockWidth) *
                            (alignToNextMultiple( height, blockHeight ) / blockHeight) * depth * 16u;
                }
                default:
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Invalid compressed pixel format",
                    "PixelUtil::getMemorySize");
            }
        }
        else
        {
            return width*height*depth*getNumElemBytes(format);
        }
    }
    //-----------------------------------------------------------------------
    uint32 PixelUtil::getCompressedBlockWidth( PixelFormat format, bool apiStrict )
    {
        switch(format)
        {
            // These formats work by dividing the image into 4x4 blocks, then encoding each
            // 4x4 block with a certain number of bytes.
            case PF_DXT1:
            case PF_DXT2:
            case PF_DXT3:
            case PF_DXT4:
            case PF_DXT5:
            case PF_BC4_SNORM:
            case PF_BC4_UNORM:
            case PF_BC5_SNORM:
            case PF_BC5_UNORM:
            case PF_BC6H_SF16:
            case PF_BC6H_UF16:
            case PF_BC7_UNORM:
            case PF_BC7_UNORM_SRGB:
            case PF_ETC2_RGB8:
            case PF_ETC2_RGBA8:
            case PF_ETC2_RGB8A1:
            case PF_ATC_RGB:
            case PF_ATC_RGBA_EXPLICIT_ALPHA:
            case PF_ATC_RGBA_INTERPOLATED_ALPHA:
                return 4;

            case PF_ETC1_RGB8:
                return apiStrict ? 0 : 4;

            // Size calculations from the PVRTC OpenGL extension spec
            // http://www.khronos.org/registry/gles/extensions/IMG/IMG_texture_compression_pvrtc.txt
            //  "Sub-images are not supportable because the PVRTC
            //  algorithm uses significant adjacency information, so there is
            //  no discrete block of texels that can be decoded as a standalone
            //  sub-unit, and so it follows that no stand alone sub-unit of
            //  data can be loaded without changing the decoding of surrounding
            //  texels."
            // In other words, if the user wants atlas, they can't be automatic
            case PF_PVRTC_RGB2:
            case PF_PVRTC_RGBA2:
            case PF_PVRTC2_2BPP:
            case PF_PVRTC_RGB4:
            case PF_PVRTC_RGBA4:
            case PF_PVRTC2_4BPP:
                return 0;

            case PF_ASTC_RGBA_4X4_LDR:
            case PF_ASTC_SRGB8A8_4X4_LDR:
                return 4u;
            case PF_ASTC_RGBA_5X4_LDR:
            case PF_ASTC_SRGB8A8_5X4_LDR:
            case PF_ASTC_RGBA_5X5_LDR:
            case PF_ASTC_SRGB8A8_5X5_LDR:
                return 5u;
            case PF_ASTC_RGBA_6X5_LDR:
            case PF_ASTC_SRGB8A8_6X5_LDR:
            case PF_ASTC_RGBA_6X6_LDR:
            case PF_ASTC_SRGB8A8_6X6_LDR:
                return 6u;
            case PF_ASTC_RGBA_8X5_LDR:
            case PF_ASTC_SRGB8A8_8X5_LDR:
            case PF_ASTC_RGBA_8X6_LDR:
            case PF_ASTC_SRGB8A8_8X6_LDR:
            case PF_ASTC_RGBA_8X8_LDR:
            case PF_ASTC_SRGB8A8_8X8_LDR:
                return 8u;
            case PF_ASTC_RGBA_10X5_LDR:
            case PF_ASTC_SRGB8A8_10X5_LDR:
            case PF_ASTC_RGBA_10X6_LDR:
            case PF_ASTC_SRGB8A8_10X6_LDR:
            case PF_ASTC_RGBA_10X8_LDR:
            case PF_ASTC_SRGB8A8_10X8_LDR:
            case PF_ASTC_RGBA_10X10_LDR:
            case PF_ASTC_SRGB8A8_10X10_LDR:
                return 10u;
            case PF_ASTC_RGBA_12X10_LDR:
            case PF_ASTC_SRGB8A8_12X10_LDR:
            case PF_ASTC_RGBA_12X12_LDR:
            case PF_ASTC_SRGB8A8_12X12_LDR:
                return 12u;

            default:
                assert( !isCompressed( format ) );
                return 1;
        }
    }
    //-----------------------------------------------------------------------
    uint32 PixelUtil::getCompressedBlockHeight( PixelFormat format, bool apiStrict )
    {
        switch( format )
        {
        case PF_ASTC_RGBA_4X4_LDR:
        case PF_ASTC_SRGB8A8_4X4_LDR:
        case PF_ASTC_RGBA_5X4_LDR:
        case PF_ASTC_SRGB8A8_5X4_LDR:
            return 4u;
        case PF_ASTC_RGBA_5X5_LDR:
        case PF_ASTC_SRGB8A8_5X5_LDR:
        case PF_ASTC_RGBA_6X5_LDR:
        case PF_ASTC_SRGB8A8_6X5_LDR:
        case PF_ASTC_RGBA_8X5_LDR:
        case PF_ASTC_SRGB8A8_8X5_LDR:
        case PF_ASTC_RGBA_10X5_LDR:
        case PF_ASTC_SRGB8A8_10X5_LDR:
            return 5u;
        case PF_ASTC_RGBA_6X6_LDR:
        case PF_ASTC_SRGB8A8_6X6_LDR:
        case PF_ASTC_RGBA_8X6_LDR:
        case PF_ASTC_SRGB8A8_8X6_LDR:
        case PF_ASTC_RGBA_10X6_LDR:
        case PF_ASTC_SRGB8A8_10X6_LDR:
            return 6u;
        case PF_ASTC_RGBA_8X8_LDR:
        case PF_ASTC_SRGB8A8_8X8_LDR:
        case PF_ASTC_RGBA_10X8_LDR:
        case PF_ASTC_SRGB8A8_10X8_LDR:
            return 8u;
        case PF_ASTC_RGBA_10X10_LDR:
        case PF_ASTC_SRGB8A8_10X10_LDR:
        case PF_ASTC_RGBA_12X10_LDR:
        case PF_ASTC_SRGB8A8_12X10_LDR:
            return 10u;

        case PF_ASTC_RGBA_12X12_LDR:
        case PF_ASTC_SRGB8A8_12X12_LDR:
            return 12u;
        default:
            return getCompressedBlockWidth( format, apiStrict );
        }

        return getCompressedBlockWidth( format, apiStrict );
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getNumElemBits( PixelFormat format )
    {
        return getDescriptionFor(format).elemBytes * 8;
    }
    //-----------------------------------------------------------------------
    uint8 PixelUtil::getMaxMipmapCount( uint32 maxResolution )
    {
        if( !maxResolution ) //log( 0 ) is undefined.
            return 0;

        uint8 numMipmaps;
#if (ANDROID || (OGRE_COMPILER == OGRE_COMPILER_MSVC && OGRE_COMP_VER < 1800))
        numMipmaps = static_cast<uint8>( floorf( logf( static_cast<float>(maxResolution) ) /
                                                 logf( 2.0f ) ) );
#else
        numMipmaps = static_cast<uint8>( floorf( log2f( static_cast<float>(maxResolution) ) ) );
#endif
        return numMipmaps;
    }
    //-----------------------------------------------------------------------
    uint8 PixelUtil::getMaxMipmapCount( uint32 width, uint32 height )
    {
        return getMaxMipmapCount( std::max( width, height ) );
    }
    //-----------------------------------------------------------------------
    uint8 PixelUtil::getMaxMipmapCount( uint32 width, uint32 height, uint32 depth )
    {
        return getMaxMipmapCount( std::max( std::max( width, height ), depth ) );
    }
    //-----------------------------------------------------------------------
    unsigned int PixelUtil::getFlags( PixelFormat format )
    {
        return getDescriptionFor(format).flags;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::hasAlpha(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_HASALPHA) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isFloatingPoint(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_FLOAT) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isInteger(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_INTEGER) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isSigned(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_SIGNED) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isCompressed(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_COMPRESSED) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isDepth(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_DEPTH) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isNativeEndian(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_NATIVEENDIAN) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isLuminance(PixelFormat format)
    {
        return (PixelUtil::getFlags(format) & PFF_LUMINANCE) > 0;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isValidExtent(size_t width, size_t height, size_t depth, PixelFormat format)
    {
        if(isCompressed(format))
        {
            switch(format)
            {
                case PF_DXT1:
                case PF_DXT2:
                case PF_DXT3:
                case PF_DXT4:
                case PF_DXT5:
                case PF_BC4_SNORM:
                case PF_BC4_UNORM:
                case PF_BC5_SNORM:
                case PF_BC5_UNORM:
                case PF_BC6H_SF16:
                case PF_BC6H_UF16:
                case PF_BC7_UNORM:
                case PF_BC7_UNORM_SRGB:
                    return ((width&3)==0 && (height&3)==0 && depth==1);
                default:
                    return true;
            }
        }
        else
        {
            return true;
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::getBitDepths(PixelFormat format, int rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rbits;
        rgba[1] = des.gbits;
        rgba[2] = des.bbits;
        rgba[3] = des.abits;
    }
    //-----------------------------------------------------------------------
    void PixelUtil::getBitMasks(PixelFormat format, uint64 rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rmask;
        rgba[1] = des.gmask;
        rgba[2] = des.bmask;
        rgba[3] = des.amask;
    }
    //---------------------------------------------------------------------
    void PixelUtil::getBitShifts(PixelFormat format, unsigned char rgba[4])
    {
        const PixelFormatDescription &des = getDescriptionFor(format);
        rgba[0] = des.rshift;
        rgba[1] = des.gshift;
        rgba[2] = des.bshift;
        rgba[3] = des.ashift;
    }
    //-----------------------------------------------------------------------
    String PixelUtil::getFormatName(PixelFormat srcformat)
    {
        return getDescriptionFor(srcformat).name;
    }
    //-----------------------------------------------------------------------
    bool PixelUtil::isAccessible(PixelFormat srcformat)
    {
        if (srcformat == PF_UNKNOWN)
            return false;
        unsigned int flags = getFlags(srcformat);
        return !((flags & PFF_COMPRESSED) || (flags & PFF_DEPTH));
    }
    //-----------------------------------------------------------------------
    PixelComponentType PixelUtil::getComponentType(PixelFormat fmt)
    {
        const PixelFormatDescription &des = getDescriptionFor(fmt);
        return des.componentType;
    }
    //-----------------------------------------------------------------------
    size_t PixelUtil::getComponentCount(PixelFormat fmt)
    {
        const PixelFormatDescription &des = getDescriptionFor(fmt);
        return des.componentCount;
    }
    //-----------------------------------------------------------------------
    PixelFormat PixelUtil::getFormatFromName(const String& name, bool accessibleOnly, bool caseSensitive)
    {
        String tmp = name;
        if (!caseSensitive)
        {
            // We are stored upper-case format names.
            StringUtil::toUpperCase(tmp);
        }

        for (int i = 0; i < PF_COUNT; ++i)
        {
            PixelFormat pf = static_cast<PixelFormat>(i);
            if (!accessibleOnly || isAccessible(pf))
            {
                if (tmp == getFormatName(pf))
                    return pf;
            }
        }
        return PF_UNKNOWN;
    }
    //-----------------------------------------------------------------------
    String PixelUtil::getBNFExpressionOfPixelFormats(bool accessibleOnly)
    {
        // Collect format names sorted by length, it's required by BNF compiler
        // that similar tokens need longer ones comes first.
        typedef multimap<String::size_type, String>::type FormatNameMap;
        FormatNameMap formatNames;
        for (size_t i = 0; i < PF_COUNT; ++i)
        {
            PixelFormat pf = static_cast<PixelFormat>(i);
            if (!accessibleOnly || isAccessible(pf))
            {
                String formatName = getFormatName(pf);
                formatNames.insert(std::make_pair(formatName.length(), formatName));
            }
        }

        // Populate the BNF expression in reverse order
        String result;
        // Note: Stupid M$ VC7.1 can't dealing operator!= with FormatNameMap::const_reverse_iterator.
        for (FormatNameMap::reverse_iterator j = formatNames.rbegin(); j != formatNames.rend(); ++j)
        {
            if (!result.empty())
                result += " | ";
            result += "'" + j->second + "'";
        }

        return result;
    }
    //-----------------------------------------------------------------------
    PixelFormat PixelUtil::getFormatForBitDepths(PixelFormat fmt, ushort integerBits, ushort floatBits)
    {
        switch (integerBits)
        {
        case 16:
            switch (fmt)
            {
            case PF_R8G8B8:
            case PF_X8R8G8B8:
                return PF_R5G6B5;

            case PF_B8G8R8:
            case PF_X8B8G8R8:
                return PF_B5G6R5;

            case PF_A8R8G8B8:
            case PF_R8G8B8A8:
            case PF_A8B8G8R8:
            case PF_B8G8R8A8:
                return PF_A4R4G4B4;

            case PF_A2R10G10B10:
            case PF_A2B10G10R10:
                return PF_A1R5G5B5;

            default:
                // use original image format
                break;
            }
            break;

        case 32:
            switch (fmt)
            {
            case PF_R5G6B5:
                return PF_X8R8G8B8;

            case PF_B5G6R5:
                return PF_X8B8G8R8;

            case PF_A4R4G4B4:
                return PF_A8R8G8B8;

            case PF_A1R5G5B5:
                return PF_A2R10G10B10;

            default:
                // use original image format
                break;
            }
            break;

        default:
            // use original image format
            break;
        }

        switch (floatBits)
        {
        case 16:
            switch (fmt)
            {
            case PF_FLOAT32_R:
                return PF_FLOAT16_R;

            case PF_FLOAT32_RGB:
                return PF_FLOAT16_RGB;

            case PF_FLOAT32_RGBA:
                return PF_FLOAT16_RGBA;

            default:
                // use original image format
                break;
            }
            break;

        case 32:
            switch (fmt)
            {
            case PF_FLOAT16_R:
                return PF_FLOAT32_R;

            case PF_FLOAT16_RGB:
                return PF_FLOAT32_RGB;

            case PF_FLOAT16_RGBA:
                return PF_FLOAT32_RGBA;

            default:
                // use original image format
                break;
            }
            break;

        default:
            // use original image format
            break;
        }

        return fmt;
    }
    //-----------------------------------------------------------------------
    /*************************************************************************
    * Pixel packing/unpacking utilities
    */
    void PixelUtil::packColour(const ColourValue &colour, const PixelFormat pf,  void* dest)
    {
        packColour(colour.r, colour.g, colour.b, colour.a, pf, dest);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(const uint8 r, const uint8 g, const uint8 b, const uint8 a, const PixelFormat pf,  void* dest)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats packing
            unsigned int value = ((Bitwise::fixedToFixed(r, 8, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::fixedToFixed(g, 8, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::fixedToFixed(b, 8, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::fixedToFixed(a, 8, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);
        } else {
            // Convert to float
            packColour((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,(float)a/255.0f, pf, dest);
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::packColour(const float r, const float g, const float b, const float a, const PixelFormat pf,  void* dest)
    {
        // Catch-it-all here
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Do the packing
            //std::cerr << dest << " " << r << " " << g <<  " " << b << " " << a << std::endl;
            const unsigned int value = ((Bitwise::floatToFixed(r, des.rbits)<<des.rshift) & des.rmask) |
                ((Bitwise::floatToFixed(g, des.gbits)<<des.gshift) & des.gmask) |
                ((Bitwise::floatToFixed(b, des.bbits)<<des.bshift) & des.bmask) |
                ((Bitwise::floatToFixed(a, des.abits)<<des.ashift) & des.amask);
            // And write to memory
            Bitwise::intWrite(dest, des.elemBytes, value);
        } else {
            switch(pf)
            {
            case PF_FLOAT32_R:
                ((float*)dest)[0] = r;
                break;
            case PF_FLOAT32_GR:
                ((float*)dest)[0] = g;
                ((float*)dest)[1] = r;
                break;
            case PF_FLOAT32_RGB:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                break;
            case PF_FLOAT32_RGBA:
                ((float*)dest)[0] = r;
                ((float*)dest)[1] = g;
                ((float*)dest)[2] = b;
                ((float*)dest)[3] = a;
                break;
            case PF_DEPTH_DEPRECATED:
            case PF_FLOAT16_R:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                break;
            case PF_FLOAT16_GR:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(r);
                break;
            case PF_FLOAT16_RGB:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                break;
            case PF_FLOAT16_RGBA:
                ((uint16*)dest)[0] = Bitwise::floatToHalf(r);
                ((uint16*)dest)[1] = Bitwise::floatToHalf(g);
                ((uint16*)dest)[2] = Bitwise::floatToHalf(b);
                ((uint16*)dest)[3] = Bitwise::floatToHalf(a);
                break;
            case PF_SHORT_RGB:
                ((uint16*)dest)[0] = (uint16)Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = (uint16)Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = (uint16)Bitwise::floatToFixed(b, 16);
                break;
            case PF_SHORT_RGBA:
                ((uint16*)dest)[0] = (uint16)Bitwise::floatToFixed(r, 16);
                ((uint16*)dest)[1] = (uint16)Bitwise::floatToFixed(g, 16);
                ((uint16*)dest)[2] = (uint16)Bitwise::floatToFixed(b, 16);
                ((uint16*)dest)[3] = (uint16)Bitwise::floatToFixed(a, 16);
                break;
            case PF_BYTE_LA:
                ((uint8*)dest)[0] = (uint8)Bitwise::floatToFixed(r, 8);
                ((uint8*)dest)[1] = (uint8)Bitwise::floatToFixed(a, 8);
                break;
            case PF_A2B10G10R10:
            {
                const uint16 ir = static_cast<uint16>( Math::saturate( r ) * 1023.0f + 0.5f );
                const uint16 ig = static_cast<uint16>( Math::saturate( g ) * 1023.0f + 0.5f );
                const uint16 ib = static_cast<uint16>( Math::saturate( b ) * 1023.0f + 0.5f );
                const uint16 ia = static_cast<uint16>( Math::saturate( a ) * 3.0f + 0.5f );

                ((uint32*)dest)[0] = (ia << 30u) | (ir << 20u) | (ig << 10u) | (ib);
                break;
            }
            default:
                // Not yet supported
                OGRE_EXCEPT(
                    Exception::ERR_NOT_IMPLEMENTED,
                    "pack to "+getFormatName(pf)+" not implemented",
                    "PixelUtil::packColour");
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(ColourValue *colour, PixelFormat pf,  const void* src)
    {
        unpackColour(&colour->r, &colour->g, &colour->b, &colour->a, pf, src);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(uint8 *r, uint8 *g, uint8 *b, uint8 *a, PixelFormat pf,  const void* src)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            const unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE)
            {
                // Luminance format -- only rbits used
                *r = *g = *b = (uint8)Bitwise::fixedToFixed(
                    (value & des.rmask)>>des.rshift, des.rbits, 8);
            }
            else
            {
                *r = (uint8)Bitwise::fixedToFixed((value & des.rmask)>>des.rshift, des.rbits, 8);
                *g = (uint8)Bitwise::fixedToFixed((value & des.gmask)>>des.gshift, des.gbits, 8);
                *b = (uint8)Bitwise::fixedToFixed((value & des.bmask)>>des.bshift, des.bbits, 8);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = (uint8)Bitwise::fixedToFixed((value & des.amask)>>des.ashift, des.abits, 8);
            }
            else
            {
                *a = 255; // No alpha, default a component to full
            }
        } else {
            // Do the operation with the more generic floating point
            float rr = 0, gg = 0, bb = 0, aa = 0;
            unpackColour(&rr,&gg,&bb,&aa, pf, src);
            *r = (uint8)Bitwise::floatToFixed(rr, 8);
            *g = (uint8)Bitwise::floatToFixed(gg, 8);
            *b = (uint8)Bitwise::floatToFixed(bb, 8);
            *a = (uint8)Bitwise::floatToFixed(aa, 8);
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::unpackColour(float *r, float *g, float *b, float *a,
        PixelFormat pf,  const void* src)
    {
        const PixelFormatDescription &des = getDescriptionFor(pf);
        if(des.flags & PFF_NATIVEENDIAN) {
            // Shortcut for integer formats unpacking
            const unsigned int value = Bitwise::intRead(src, des.elemBytes);
            if(des.flags & PFF_LUMINANCE)
            {
                // Luminance format -- only rbits used
                *r = *g = *b = Bitwise::fixedToFloat(
                    (value & des.rmask)>>des.rshift, des.rbits);
            }
            else
            {
                *r = Bitwise::fixedToFloat((value & des.rmask)>>des.rshift, des.rbits);
                *g = Bitwise::fixedToFloat((value & des.gmask)>>des.gshift, des.gbits);
                *b = Bitwise::fixedToFloat((value & des.bmask)>>des.bshift, des.bbits);
            }
            if(des.flags & PFF_HASALPHA)
            {
                *a = Bitwise::fixedToFloat((value & des.amask)>>des.ashift, des.abits);
            }
            else
            {
                *a = 1.0f; // No alpha, default a component to full
            }
        } else {
            switch(pf)
            {
            case PF_FLOAT32_R:
                *r = *g = *b = ((const float*)src)[0];
                *a = 1.0f;
                break;
            case PF_FLOAT32_GR:
                *g = ((const float*)src)[0];
                *r = *b = ((const float*)src)[1];
                *a = 1.0f;
                break;
            case PF_FLOAT32_RGB:
                *r = ((const float*)src)[0];
                *g = ((const float*)src)[1];
                *b = ((const float*)src)[2];
                *a = 1.0f;
                break;
            case PF_FLOAT32_RGBA:
                *r = ((const float*)src)[0];
                *g = ((const float*)src)[1];
                *b = ((const float*)src)[2];
                *a = ((const float*)src)[3];
                break;
            case PF_DEPTH_DEPRECATED:
            case PF_FLOAT16_R:
                *r = *g = *b = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_GR:
                *g = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *r = *b = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_RGB:
                *r = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((const uint16*)src)[2]);
                *a = 1.0f;
                break;
            case PF_FLOAT16_RGBA:
                *r = Bitwise::halfToFloat(((const uint16*)src)[0]);
                *g = Bitwise::halfToFloat(((const uint16*)src)[1]);
                *b = Bitwise::halfToFloat(((const uint16*)src)[2]);
                *a = Bitwise::halfToFloat(((const uint16*)src)[3]);
                break;
            case PF_SHORT_RGB:
                *r = Bitwise::fixedToFloat(((const uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((const uint16*)src)[1], 16);
                *b = Bitwise::fixedToFloat(((const uint16*)src)[2], 16);
                *a = 1.0f;
                break;
            case PF_SHORT_RGBA:
                *r = Bitwise::fixedToFloat(((const uint16*)src)[0], 16);
                *g = Bitwise::fixedToFloat(((const uint16*)src)[1], 16);
                *b = Bitwise::fixedToFloat(((const uint16*)src)[2], 16);
                *a = Bitwise::fixedToFloat(((const uint16*)src)[3], 16);
                break;
            case PF_BYTE_LA:
                *r = *g = *b = Bitwise::fixedToFloat(((const uint8*)src)[0], 8);
                *a = Bitwise::fixedToFloat(((const uint8*)src)[1], 8);
                break;
            default:
                // Not yet supported
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "unpack from "+getFormatName(pf)+" not implemented",
                    "PixelUtil::unpackColour");
                break;
            }
        }
    }
    //-----------------------------------------------------------------------
    /* Convert pixels from one format to another */
    void PixelUtil::bulkPixelConversion(void *srcp, PixelFormat srcFormat,
        void *destp, PixelFormat dstFormat, unsigned int count)
    {
        PixelBox src(count, 1, 1, srcFormat, srcp),
                 dst(count, 1, 1, dstFormat, destp);

        bulkPixelConversion(src, dst);
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkCompressedSubregion( const PixelBox &src, const PixelBox &dst,
                                             const Box &dstRegion )
    {
        assert(src.getWidth()  == dstRegion.getWidth() &&
               src.getHeight() == dstRegion.getHeight() &&
               src.getDepth()  == dstRegion.getDepth());

        assert( dst.contains( dstRegion ) );
        assert( dst.format == src.format );
        assert( src.isConsecutive() && dst.isConsecutive() );

        if( src.getWidth()  == dst.getWidth() &&
            src.getHeight() == dst.getHeight() &&
            src.getDepth()  == dst.getDepth() )
        {
            bulkPixelConversion( src, dst );
            return;
        }

        uint32 blockWidth  = PixelUtil::getCompressedBlockWidth( dst.format, false );
        uint32 blockHeight = PixelUtil::getCompressedBlockHeight( dst.format, false );
        uint32 blockResolution = blockWidth * blockHeight;
        if( !blockWidth || !blockHeight )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Cannot transfer subregions of the image when compressed by format "
                         + PixelUtil::getFormatName( dst.format ) +
                         ". You must update the entire image.",
                        "PixelUtil::bulkCompressedSubregion");
        }

        if( dstRegion.left % blockWidth || dstRegion.right % blockWidth ||
            dstRegion.top % blockHeight || dstRegion.bottom % blockHeight )
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS,
                        "Image transfers for the compressed format " +
                         PixelUtil::getFormatName( dst.format ) +
                         " requires subregions to be aligned to " +
                         StringConverter::toString( blockWidth ) + "x" +
                         StringConverter::toString( blockHeight ) +  " blocks",
                         "PixelUtil::bulkPixelConversion");
        }

        size_t blockSize = PixelUtil::getMemorySize( blockWidth, blockHeight, 1, dst.format );

        for( size_t z=dstRegion.front; z<dstRegion.back; ++z )
        {
            size_t dstZ = z * ( (dst.getWidth() * dst.getHeight()) / blockResolution );
            size_t srcZ = z * ( (src.getWidth() * src.getHeight()) / blockResolution );
            for( size_t y=dstRegion.top; y<dstRegion.bottom; y += blockHeight )
            {
                size_t dstY = ((y - dst.top) * dst.getWidth()) / blockResolution;
                size_t srcY = (y * src.getWidth()) / blockResolution;
                memcpy( (uint8*)(dst.data) + ( (dstZ + dstY + dstRegion.left / blockWidth) * blockSize ),
                        (uint8*)(src.data) + ( (srcZ + srcY ) * blockSize ),
                        (dstRegion.getWidth() / blockWidth) * blockSize );
            }
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkPixelConversion(const PixelBox &src, const PixelBox &dst)
    {
        assert(src.getWidth() == dst.getWidth() &&
               src.getHeight() == dst.getHeight() &&
               src.getDepth() == dst.getDepth());

        // Check for compressed formats, we don't support decompression, compression or recoding
        if(PixelUtil::isCompressed(src.format) || PixelUtil::isCompressed(dst.format))
        {
            if(src.format == dst.format)
            {
                if(src.isConsecutive() && dst.isConsecutive())
                {
                    // we can copy with slice granularity, useful for Tex2DArray handling
                    size_t bytesPerSlice = getMemorySize(src.getWidth(), src.getHeight(), 1, src.format);
                    memcpy(
                        (uint8*)dst.data + bytesPerSlice * dst.front,
                        (uint8*)src.data + bytesPerSlice * src.front,
                        bytesPerSlice * src.getDepth());
                    return;
                }
                else
                {
                    const size_t rowSize = PixelUtil::getMemorySize( src.getWidth(), 1, 1, src.format );
                    const uint32 blockWidth  = PixelUtil::getCompressedBlockWidth( dst.format, false );
                    const uint32 blockHeight = PixelUtil::getCompressedBlockHeight( dst.format, false );

                    if( blockWidth == 0 || blockHeight == 0 )
                    {
                        OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                            "This format should be consecutive!",
                            "PixelUtil::bulkPixelConversion");
                    }

                    uint8 *srcptr = static_cast<uint8*>(src.data)
                        + (src.left + blockWidth - 1) / blockWidth +
                            (src.top + blockHeight - 1) / blockHeight * src.rowPitch +
                            src.front * src.slicePitch;
                    uint8 *dstptr = static_cast<uint8*>(dst.data)
                        + (dst.left + blockWidth - 1) / blockWidth +
                            (dst.top + blockHeight - 1) / blockHeight * dst.rowPitch +
                            dst.front * dst.slicePitch;

                    // Calculate pitches+skips in bytes
                    const size_t srcRowPitchBytes = src.rowPitch;
                    const size_t srcSliceSkipBytes = src.getSliceSkip();

                    const size_t dstRowPitchBytes   = dst.rowPitch;
                    const size_t dstSliceSkipBytes  = dst.getSliceSkip();

                    const size_t compressedSrcTop = (src.top + blockHeight - 1) / blockHeight;
                    const size_t compressedSrcBottom = (src.bottom + blockHeight - 1) / blockHeight;

                    for( size_t z=src.front; z<src.back; ++z )
                    {
                        for( size_t y=compressedSrcTop; y<compressedSrcBottom; ++y )
                        {
                            memcpy( dstptr, srcptr, rowSize );
                            srcptr += srcRowPitchBytes;
                            dstptr += dstRowPitchBytes;
                        }

                        srcptr += srcSliceSkipBytes;
                        dstptr += dstSliceSkipBytes;
                    }
                }
                return;
            }
            else
            {
                OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                    "This method can not be used to compress or decompress images",
                    "PixelUtil::bulkPixelConversion");
            }
        }

        // The easy case
        if(src.format == dst.format) {
            // Everything consecutive?
            if(src.isConsecutive() && dst.isConsecutive())
            {
                memcpy(dst.getTopLeftFrontPixelPtr(), src.getTopLeftFrontPixelPtr(), src.getConsecutiveSize());
                return;
            }

            const size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
            const size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);
            uint8 *srcptr = static_cast<uint8*>(src.data)
                + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
            uint8 *dstptr = static_cast<uint8*>(dst.data)
                + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;

            // Calculate pitches+skips in bytes
            const size_t srcRowPitchBytes = src.rowPitch*srcPixelSize;
            //const size_t srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
            const size_t srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;

            const size_t dstRowPitchBytes = dst.rowPitch*dstPixelSize;
            //const size_t dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
            const size_t dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

            // Otherwise, copy per row
            const size_t rowSize = src.getWidth()*srcPixelSize;
            for(size_t z=src.front; z<src.back; z++)
            {
                for(size_t y=src.top; y<src.bottom; y++)
                {
                    memcpy(dstptr, srcptr, rowSize);
                    srcptr += srcRowPitchBytes;
                    dstptr += dstRowPitchBytes;
                }
                srcptr += srcSliceSkipBytes;
                dstptr += dstSliceSkipBytes;
            }
            return;
        }
        // Converting to PF_X8R8G8B8 is exactly the same as converting to
        // PF_A8R8G8B8. (same with PF_X8B8G8R8 and PF_A8B8G8R8)
        if(dst.format == PF_X8R8G8B8 || dst.format == PF_X8B8G8R8)
        {
            // Do the same conversion, with PF_A8R8G8B8, which has a lot of
            // optimized conversions
            PixelBox tempdst = dst;
            tempdst.format = dst.format==PF_X8R8G8B8?PF_A8R8G8B8:PF_A8B8G8R8;
            bulkPixelConversion(src, tempdst);
            return;
        }
        // Converting from PF_X8R8G8B8 is exactly the same as converting from
        // PF_A8R8G8B8, given that the destination format does not have alpha.
        if((src.format == PF_X8R8G8B8||src.format == PF_X8B8G8R8) && !hasAlpha(dst.format))
        {
            // Do the same conversion, with PF_A8R8G8B8, which has a lot of
            // optimized conversions
            PixelBox tempsrc = src;
            tempsrc.format = src.format==PF_X8R8G8B8?PF_A8R8G8B8:PF_A8B8G8R8;
            bulkPixelConversion(tempsrc, dst);
            return;
        }

// NB VC6 can't handle the templates required for optimised conversion, tough
#if OGRE_COMPILER != OGRE_COMPILER_MSVC || OGRE_COMP_VER >= 1300
        // Is there a specialized, inlined, conversion?
        if(doOptimizedConversion(src, dst))
        {
            // If so, good
            return;
        }
#endif

        const size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
        const size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);
        uint8 *srcptr = static_cast<uint8*>(src.data)
            + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
        uint8 *dstptr = static_cast<uint8*>(dst.data)
            + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;
        
        // Old way, not taking into account box dimensions
        //uint8 *srcptr = static_cast<uint8*>(src.data), *dstptr = static_cast<uint8*>(dst.data);

        // Calculate pitches+skips in bytes
        const size_t srcRowSkipBytes = src.getRowSkip()*srcPixelSize;
        const size_t srcSliceSkipBytes = src.getSliceSkip()*srcPixelSize;
        const size_t dstRowSkipBytes = dst.getRowSkip()*dstPixelSize;
        const size_t dstSliceSkipBytes = dst.getSliceSkip()*dstPixelSize;

        // The brute force fallback
        float r = 0, g = 0, b = 0, a = 1;
        for(size_t z=src.front; z<src.back; z++)
        {
            for(size_t y=src.top; y<src.bottom; y++)
            {
                for(size_t x=src.left; x<src.right; x++)
                {
                    unpackColour(&r, &g, &b, &a, src.format, srcptr);
                    packColour(r, g, b, a, dst.format, dstptr);
                    srcptr += srcPixelSize;
                    dstptr += dstPixelSize;
                }
                srcptr += srcRowSkipBytes;
                dstptr += dstRowSkipBytes;
            }
            srcptr += srcSliceSkipBytes;
            dstptr += dstSliceSkipBytes;
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::convertForNormalMapping(const PixelBox &src, const PixelBox &dst)
    {
        assert(src.getWidth() == dst.getWidth() &&
               src.getHeight() == dst.getHeight() &&
               src.getDepth() == dst.getDepth() &&
               (dst.format == PF_R8G8_SNORM || dst.format == PF_RG8 || dst.format == PF_BYTE_LA)  );

        OgreProfileExhaustiveAggr( "PixelUtil::convertForNormalMapping" );

        const PixelFormatDescription &srcDesc = getDescriptionFor( src.format );

        const size_t srcPixelSize = PixelUtil::getNumElemBytes(src.format);
        const size_t dstPixelSize = PixelUtil::getNumElemBytes(dst.format);
        uint8 *srcPtr = static_cast<uint8*>(src.data)
            + (src.left + src.top * src.rowPitch + src.front * src.slicePitch) * srcPixelSize;
        int8 *dstPtr = static_cast<int8*>(dst.data)
            + (dst.left + dst.top * dst.rowPitch + dst.front * dst.slicePitch) * dstPixelSize;

        // Calculate pitches+skips in bytes
        const size_t srcRowSkipBytes    = src.getRowSkip() * srcPixelSize;
        const size_t srcSliceSkipBytes  = src.getSliceSkip() * srcPixelSize;
        const size_t dstRowSkipBytes    = dst.getRowSkip() * dstPixelSize;
        const size_t dstSliceSkipBytes  = dst.getSliceSkip() * dstPixelSize;

        uint8 notLuminanceMask  = srcDesc.flags & PFF_LUMINANCE ? 0x00 : 0xFF;
        uint8 luminanceMask     = srcDesc.flags & PFF_LUMINANCE ? 0xFF : 0x00;

        if( srcDesc.flags & PFF_FLOAT )
        {
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Floating point formats for normal maps is not implemented yet",
                         "PixelUtil::convertForNormalMapping" );
        }

        if( srcDesc.flags & PFF_SIGNED )
        {
            OGRE_EXCEPT( Exception::ERR_NOT_IMPLEMENTED,
                         "Signed format origins for normal maps is not tested",
                         "PixelUtil::convertForNormalMapping" );
        }

        uint8 shiftOffset = 0x80;

        if( dst.format == PF_BYTE_LA || dst.format == PF_RG8 )
            shiftOffset = 0x00;

        uint8 r, g, b, a;
        for(size_t z=src.front; z<src.back; z++)
        {
            for(size_t y=src.top; y<src.bottom; y++)
            {
                for(size_t x=src.left; x<src.right; x++)
                {
                    unpackColour( &r, &g, &b, &a, src.format, srcPtr );

                    g = (g & notLuminanceMask) | (a & luminanceMask);

                    *dstPtr++ = r - shiftOffset;
                    *dstPtr++ = g - shiftOffset;

                    srcPtr += srcPixelSize;
                }

                srcPtr += srcRowSkipBytes;
                dstPtr += dstRowSkipBytes;
            }

            srcPtr += srcSliceSkipBytes;
            dstPtr += dstSliceSkipBytes;
        }
    }
    //-----------------------------------------------------------------------
    void PixelUtil::bulkPixelVerticalFlip(const PixelBox &box)
    {
        // Check for compressed formats, we don't support decompression, compression or recoding
        if(PixelUtil::isCompressed(box.format))
        {
            OGRE_EXCEPT(Exception::ERR_NOT_IMPLEMENTED,
                        "This method can not be used for compressed formats",
                        "PixelUtil::bulkPixelVerticalFlip");
        }
        
        const size_t pixelSize = PixelUtil::getNumElemBytes(box.format);
        const size_t copySize = (box.right - box.left) * pixelSize;

        // Calculate pitches in bytes
        const size_t rowPitchBytes = box.rowPitch * pixelSize;
        const size_t slicePitchBytes = box.slicePitch * pixelSize;

        uint8 *basesrcptr = static_cast<uint8*>(box.data)
            + (box.left + box.top * box.rowPitch + box.front * box.slicePitch) * pixelSize;
        uint8 *basedstptr = basesrcptr + (box.bottom - box.top - 1) * rowPitchBytes;
        uint8* tmpptr = (uint8*)OGRE_MALLOC_ALIGN(copySize, MEMCATEGORY_GENERAL, false);
        
        // swap rows
        const size_t halfRowCount = (box.bottom - box.top) >> 1;
        for(size_t z = box.front; z < box.back; z++)
        {
            uint8* srcptr = basesrcptr;
            uint8* dstptr = basedstptr;
            for(size_t y = 0; y < halfRowCount; y++)
            {
                // swap rows
                memcpy(tmpptr, dstptr, copySize);
                memcpy(dstptr, srcptr, copySize);
                memcpy(srcptr, tmpptr, copySize);
                srcptr += rowPitchBytes;
                dstptr -= rowPitchBytes;
            }
            basesrcptr += slicePitchBytes;
            basedstptr += slicePitchBytes;
        }
        
        OGRE_FREE_ALIGN(tmpptr, MEMCATEGORY_GENERAL, false);
    }

    ColourValue PixelBox::getColourAt(size_t x, size_t y, size_t z)
    {
        ColourValue cv;

        unsigned char pixelSize = PixelUtil::getNumElemBytes(format);
        size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
        PixelUtil::unpackColour(&cv, format, (unsigned char *)data + pixelOffset);

        return cv;
    }

    void PixelBox::setColourAt(ColourValue const &cv, size_t x, size_t y, size_t z)
    {
        unsigned char pixelSize = PixelUtil::getNumElemBytes(format);
        size_t pixelOffset = pixelSize * (z * slicePitch + y * rowPitch + x);
        PixelUtil::packColour(cv, format, (unsigned char *)data + pixelOffset);
    }

}
