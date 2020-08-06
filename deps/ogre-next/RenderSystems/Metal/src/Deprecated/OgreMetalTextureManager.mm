/*
  -----------------------------------------------------------------------------
  This source file is part of OGRE
  (Object-oriented Graphics Rendering Engine)
  For the latest info, see http://www.ogre3d.org

Copyright (c) 2000-2016 Torus Knot Software Ltd

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

#include "OgreMetalTextureManager.h"
#include "OgreMetalTexture.h"
#include "OgreMetalDepthTexture.h"
#include "OgreMetalNullTexture.h"

namespace Ogre
{
    MetalTextureManager::MetalTextureManager( MetalDevice *device ) :
        TextureManager(),
        mDevice( device )
    {
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }

    MetalTextureManager::~MetalTextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);
    }

    Resource* MetalTextureManager::createImpl(const String& name,
        ResourceHandle handle, const String& group, bool isManual,
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        if( createParams )
        {
            if( createParams->find( "DepthTexture" ) != createParams->end() )
            {
                const bool shareableDepthBuffer = createParams->find( "shareableDepthBuffer" ) !=
                                                                                createParams->end();
                return new MetalDepthTexture( shareableDepthBuffer, this, name, handle, group,
                                              isManual, loader, mDevice );
            }

            NameValuePairList::const_iterator it = createParams->find( "SpecialFormat" );
            if( it != createParams->end() && it->second == "PF_NULL" )
            {
                return new MetalNullTexture( this, name, handle, group,
                                             isManual, loader, mDevice );
            }
        }

        return new MetalTexture( this, name, handle, group, isManual, loader, mDevice );
    }

    PixelFormat MetalTextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        if( format == PF_R8G8B8 )
            return PF_X8R8G8B8;
        if( format == PF_B8G8R8 )
            return PF_X8B8G8R8;

        return format;
    }

    bool MetalTextureManager::isHardwareFilteringSupported( TextureType ttype, PixelFormat format,
                                                            int usage, bool preciseFormatOnly )
    {
        return true;
    }
}
