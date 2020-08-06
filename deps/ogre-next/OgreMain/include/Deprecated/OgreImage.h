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
#ifndef _Image_H__
#define _Image_H__

#include "OgrePrerequisites.h"
#include "OgreCommon.h"
#include "OgrePixelFormat.h"

namespace Ogre {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Image
    *  @{
    */

    enum ImageFlags
    {
        IF_COMPRESSED = 0x00000001,
        IF_CUBEMAP    = 0x00000002,
        IF_3D_TEXTURE = 0x00000004
    };
    /** Class representing an image file.
        @remarks
            The Image class usually holds uncompressed image data and is the
            only object that can be loaded in a texture. Image  objects handle 
            image data decoding themselves by the means of locating the correct 
            Codec object for each data type.
        @par
            Typically, you would want to use an Image object to load a texture
            when extra processing needs to be done on an image before it is
            loaded or when you want to blit to an existing texture.
    */
    class _OgreExport Image : public ImageAlloc
    {
    public:
        typedef Ogre::Box Box;
        typedef Ogre::Rect Rect;
    public:
        /** Standard constructor.
        */
        Image();
        /** Copy-constructor - copies all the data from the target image.
        */
        Image( const Image &img );

        /** Standard destructor.
        */
        virtual ~Image();

        /** Assignment operator - copies all the data from the target image.
        */
        Image & operator = ( const Image & img );

        /** Flips (mirrors) the image around the Y-axis. 
            @remarks
                An example of an original and flipped image:
                <pre>                
                originalimg
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                ------------> flip axis
                00000000000
                00000000000
                00000000000
                00000000000
                00000000000
                originalimg
                </pre>
        */
        Image & flipAroundY();

        /** Flips (mirrors) the image around the X-axis.
            @remarks
                An example of an original and flipped image:
                <pre>
                        flip axis
                            |
                originalimg|gmilanigiro
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                00000000000|00000000000
                </pre>
        */                 
        Image & flipAroundX();

        /** Stores a pointer to raw data in memory. The pixel format has to be specified.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @note
                Whilst typically your image is likely to be a simple 2D image,
                you can define complex images including cube maps, volume maps,
                and images including custom mip levels. The layout of the 
                internal memory should be:
                <ul><li>face 0, mip 0 (top), width x height (x depth)</li>
                <li>face 0, mip 1, width/2 x height/2 (x depth/2)</li>
                <li>face 0, mip 2, width/4 x height/4 (x depth/4)</li>
                <li>.. remaining mips for face 0 .. </li>
                <li>face 1, mip 0 (top), width x height (x depth)</li
                <li>.. and so on. </li>
                </ul>
                Of course, you will never have multiple faces (cube map) and
                depth too.
            @param data
                The data pointer
            @param width
                Width of image
            @param height
                Height of image
            @param depth
                Image Depth (in 3d images, numbers of layers, otherwise 1)
            @param format
                Pixel Format
            @param autoDelete
                If memory associated with this buffer is to be destroyed
                with the Image object. Note: it's important that if you set
                this option to true, that you allocated the memory using OGRE_ALLOC_T
                with a category of MEMCATEGORY_GENERAL to ensure the freeing of memory 
                matches up.
            @param numFaces
                The number of faces the image data has inside (6 for cubemaps, 1 otherwise)
            @param numMipMaps
                The number of mipmaps the image data has inside
            @note
                 The memory associated with this buffer is NOT destroyed with the
                 Image object, unless autoDelete is set to true.
            @remarks 
                The size of the buffer must be numFaces*PixelUtil::getMemorySize(width, height, depth, format)
         */
        Image& loadDynamicImage( uchar* data, uint32 width, uint32 height,
                            uint32 depth,
                             PixelFormat format, bool autoDelete = false, 
                             size_t numFaces = 1, uint8 numMipMaps = 0);
        
        /** Stores a pointer to raw data in memory. The pixel format has to be specified.
            @remarks
                This method loads an image into memory held in the object. The 
                pixel format will be either greyscale or RGB with an optional
                Alpha component.
                The type can be determined by calling getFormat().             
            @note
                Whilst typically your image is likely to be a simple 2D image,
                you can define complex images including cube maps
                and images including custom mip levels. The layout of the 
                internal memory should be:
                <ul><li>face 0, mip 0 (top), width x height</li>
                <li>face 0, mip 1, width/2 x height/2 </li>
                <li>face 0, mip 2, width/4 x height/4 </li>
                <li>.. remaining mips for face 0 .. </li>
                <li>face 1, mip 0 (top), width x height (x depth)</li
                <li>.. and so on. </li>
                </ul>
                Of course, you will never have multiple faces (cube map) and
                depth too.
            @param data
                The data pointer
            @param width
                Width of image
            @param height
                Height of image
            @param format
                Pixel Format
            @note
                 The memory associated with this buffer is NOT destroyed with the
                 Image object.
            @remarks This function is deprecated; one should really use the
                Image::loadDynamicImage(data, width, height, depth, format, ...) to be compatible
                with future Ogre versions.
         */
        Image& loadDynamicImage( uchar* data, uint32 width,
                                 uint32 height, PixelFormat format)
        {
            return loadDynamicImage(data, width, height, 1, format);
        }
        /** Loads raw data from a stream. See the function
            loadDynamicImage for a description of the parameters.
            @remarks 
                The size of the buffer must be numFaces*PixelUtil::getMemorySize(width, height, depth, format)
            @note
                Whilst typically your image is likely to be a simple 2D image,
                you can define complex images including cube maps
                and images including custom mip levels. The layout of the 
                internal memory should be:
                <ul><li>face 0, mip 0 (top), width x height (x depth)</li>
                <li>face 0, mip 1, width/2 x height/2 (x depth/2)</li>
                <li>face 0, mip 2, width/4 x height/4 (x depth/4)</li>
                <li>.. remaining mips for face 0 .. </li>
                <li>face 1, mip 0 (top), width x height (x depth)</li
                <li>.. and so on. </li>
                </ul>
                Of course, you will never have multiple faces (cube map) and
                depth too.
        */
        Image & loadRawData( 
            DataStreamPtr& stream, 
            uint32 width, uint32 height, uint32 depth,
            PixelFormat format,
            size_t numFaces = 1, size_t numMipMaps = 0);
        /** Loads raw data from a stream. The pixel format has to be specified. 
            @remarks This function is deprecated; one should really use the
                Image::loadRawData(stream, width, height, depth, format, ...) to be compatible
                with future Ogre versions.
            @note
                Whilst typically your image is likely to be a simple 2D image,
                you can define complex images including cube maps
                and images including custom mip levels. The layout of the 
                internal memory should be:
                <ul><li>face 0, mip 0 (top), width x height</li>
                <li>face 0, mip 1, width/2 x height/2 </li>
                <li>face 0, mip 2, width/4 x height/4 </li>
                <li>.. remaining mips for face 0 .. </li>
                <li>face 1, mip 0 (top), width x height (x depth)</li
                <li>.. and so on. </li>
                </ul>
                Of course, you will never have multiple faces (cube map) and
                depth too.
        */
        Image & loadRawData( 
            DataStreamPtr& stream, 
            uint32 width, uint32 height,
            PixelFormat format )
        {
            return loadRawData(stream, width, height, 1, format);
        }

        /** Loads an image file.
            @remarks
                This method loads an image into memory. Any format for which 
                an associated ImageCodec is registered can be loaded. 
                This can include complex formats like DDS with embedded custom 
                mipmaps, cube faces and volume textures.
                The type can be determined by calling getFormat().             
            @param
                filename Name of an image file to load.
            @param
                groupName Name of the resource group to search for the image
            @note
                The memory associated with this buffer is destroyed with the
                Image object.
        */
        Image & load( const String& filename, const String& groupName );

        /** Loads an image file from a stream.
            @remarks
                This method works in the same way as the filename-based load 
                method except it loads the image from a DataStream object. 
                This DataStream is expected to contain the 
                encoded data as it would be held in a file. 
                Any format for which an associated ImageCodec is registered 
                can be loaded. 
                This can include complex formats like DDS with embedded custom 
                mipmaps, cube faces and volume textures.
                The type can be determined by calling getFormat().             
            @param
                stream The source data.
            @param
                type The type of the image. Used to decide what decompression
                codec to use. Can be left blank if the stream data includes
                a header to identify the data.
            @see
                Image::load( const String& filename )
        */
        Image & load(DataStreamPtr& stream, const String& type = BLANKSTRING );

        /** Utility method to combine 2 separate images into this one, with the first
        image source supplying the RGB channels, and the second image supplying the 
        alpha channel (as luminance or separate alpha). 
        @param rgbFilename Filename of image supplying the RGB channels (any alpha is ignored)
        @param alphaFilename Filename of image supplying the alpha channel. If a luminance image the
            single channel is used directly, if an RGB image then the values are
            converted to greyscale.
        @param groupName The resource group from which to load the images
        @param format The destination format
        */
        Image & loadTwoImagesAsRGBA(const String& rgbFilename, const String& alphaFilename,
            const String& groupName, PixelFormat format = PF_BYTE_RGBA);

        /** Utility method to combine 2 separate images into this one, with the first
        image source supplying the RGB channels, and the second image supplying the 
        alpha channel (as luminance or separate alpha). 
        @param rgbStream Stream of image supplying the RGB channels (any alpha is ignored)
        @param alphaStream Stream of image supplying the alpha channel. If a luminance image the
            single channel is used directly, if an RGB image then the values are
            converted to greyscale.
        @param format The destination format
        @param rgbType The type of the RGB image. Used to decide what decompression
            codec to use. Can be left blank if the stream data includes
            a header to identify the data.
        @param alphaType The type of the alpha image. Used to decide what decompression
            codec to use. Can be left blank if the stream data includes
            a header to identify the data.
        */
        Image & loadTwoImagesAsRGBA(DataStreamPtr& rgbStream, DataStreamPtr& alphaStream, PixelFormat format = PF_BYTE_RGBA,
            const String& rgbType = BLANKSTRING, const String& alphaType = BLANKSTRING);

        /** Utility method to combine 2 separate images into this one, with the first
            image source supplying the RGB channels, and the second image supplying the 
            alpha channel (as luminance or separate alpha). 
        @param rgb Image supplying the RGB channels (any alpha is ignored)
        @param alpha Image supplying the alpha channel. If a luminance image the
            single channel is used directly, if an RGB image then the values are
            converted to greyscale.
        @param format The destination format
        */
        Image & combineTwoImagesAsRGBA(const Image& rgb, const Image& alpha, PixelFormat format = PF_BYTE_RGBA);

        
        /** Save the image as a file. 
        @remarks
            Saving and loading are implemented by back end (sometimes third 
            party) codecs.  Implemented saving functionality is more limited
            than loading in some cases. Particularly DDS file format support 
            is currently limited to true colour or single channel float32, 
            square, power of two textures with no mipmaps.  Volumetric support
            is currently limited to DDS files.
        */
        void save(const String& filename);

        /** Encode the image and return a stream to the data. 
            @param formatextension An extension to identify the image format
                to encode into, e.g. "jpg" or "png"
        */
        DataStreamPtr encode(const String& formatextension);

        /** Returns a pointer to the internal image buffer.
        @remarks
            Be careful with this method. You will almost certainly
            prefer to use getPixelBox, especially with complex images
            which include many faces or custom mipmaps.
        */
        uchar* getData(void);

        /** Returns a const pointer to the internal image buffer.
        @remarks
            Be careful with this method. You will almost certainly
            prefer to use getPixelBox, especially with complex images
            which include many faces or custom mipmaps.
        */
        const uchar * getData() const;       

        /** Returns the size of the data buffer.
        */
        size_t getSize() const;

        /** Returns the number of mipmaps contained in the image.
        */
        uint8 getNumMipmaps() const;

        /** Returns true if the image has the appropriate flag set.
        */
        bool hasFlag(const ImageFlags imgFlag) const;

        /** Gets the width of the image in pixels.
        */
        uint32 getWidth(void) const;

        /** Gets the height of the image in pixels.
        */
        uint32 getHeight(void) const;

        /** Gets the depth of the image.
        */
        uint32 getDepth(void) const;
        
        /** Get the number of faces of the image. This is usually 6 for a cubemap, and
            1 for a normal image.
        */
        size_t getNumFaces(void) const;

        /** Gets the physical width in bytes of each row of pixels.
        */
        size_t getRowSpan(void) const;

        /** Returns the image format.
        */
        PixelFormat getFormat() const;

        /** Returns the number of bits per pixel.
        */
        uchar getBPP() const;

        /** Returns true if the image has an alpha component.
        */
        bool getHasAlpha() const;
        
        /** Does gamma adjustment.
            @note
                Basic algo taken from Titan Engine, copyright (c) 2000 Ignacio 
                Castano Iguado
        */
        static void applyGamma( uchar *buffer, Real gamma, size_t size, uchar bpp );

        /**
         * Get colour value from a certain location in the image. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        ColourValue getColourAt(size_t x, size_t y, size_t z) const;
        
        /**
         * Set colour value at a certain location in the image. The z coordinate
         * is only valid for cubemaps and volume textures. This uses the first (largest)
         * mipmap.
         */
        void setColourAt(ColourValue const &cv, size_t x, size_t y, size_t z);

        /**
         * Get a PixelBox encapsulating the image data of a mipmap
         */
        PixelBox getPixelBox(size_t face = 0, size_t mipmap = 0) const;

        /// Delete all the memory held by this image, if owned by this image (not dynamic)
        void freeMemory();

        enum Filter
        {
            FILTER_NEAREST,
            FILTER_LINEAR,
            FILTER_BILINEAR,
            FILTER_BOX,
            FILTER_TRIANGLE,
            FILTER_BICUBIC,
            /// Applies gaussian filter over the image, then a point sampling reduction
            /// This is done at the same time (i.e. we don't blur pixels we ignore).
            FILTER_GAUSSIAN,
            /// Applies gaussian filter over the image, then bilinear downsamples.
            /// This prevents certain artifacts for some images when using FILTER_GAUSSIAN,
            /// like biasing towards certain direction. Not supported by cubemaps.
            FILTER_GAUSSIAN_HIGH
        };
        /** Scale a 1D, 2D or 3D image volume. 
            @param  src         PixelBox containing the source pointer, dimensions and format
            @param  dst         PixelBox containing the destination pointer, dimensions and format
            @param  filter      Which filter to use
            @remarks    This function can do pixel format conversion in the process.
            @note   dst and src can point to the same PixelBox object without any problem
        */
        static void scale(const PixelBox &src, const PixelBox &dst, Filter filter = FILTER_BILINEAR);
        
        /** Resize a 2D image, applying the appropriate filter. */
        void resize(ushort width, ushort height, Filter filter = FILTER_BILINEAR);

        /** Generates the mipmaps for this image. For Cubemaps, the filtering is seamless; and a
            gaussian filter is recommended although it's slow.
        @remarks
            Cannot handle compressed formats.
            Gaussian filter is implemented with a generic 1-pass convolution matrix, which in
            turn means it is O( N^N ) instead of a 2-pass filter which is O( 2^N ); where
            N is the number of taps. The Gaussian filter is 5x5
        @param gammaCorrected
            True if the filter should be applied in linear space.
        @param filter
            The type of filter to use.
        @return
            False if failed to generate and mipmaps properties won't be changed. True on success.
        */
        bool generateMipmaps( bool gammaCorrected, Filter filter = FILTER_BILINEAR );
        
        /// Static function to calculate size in bytes from the number of mipmaps, faces and the dimensions
        static size_t calculateSize(size_t mipmaps, size_t faces, uint32 width, uint32 height, uint32 depth, PixelFormat format);

        /// Static function to get an image type string from a stream via magic numbers
        static String getFileExtFromMagic(DataStreamPtr stream);

    protected:
        /// The width of the image in pixels
        uint32 mWidth;
        /// The height of the image in pixels
        uint32 mHeight;
        /// The depth of the image
        uint32 mDepth;
        /// The size of the image buffer
        size_t mBufSize;
        /// The number of mipmaps the image contains
        uint8 mNumMipmaps;
        /// Image specific flags.
        int mFlags;

        /// The pixel format of the image
        PixelFormat mFormat;

        /// The number of bytes per pixel
        uchar mPixelSize;
        uchar* mBuffer;

        /// A bool to determine if we delete the buffer or the calling app does
        bool mAutoDelete;
    };

    typedef vector<Image*>::type ImagePtrList;
    typedef vector<const Image*>::type ConstImagePtrList;

    /** @} */
    /** @} */

} // namespace

#endif
