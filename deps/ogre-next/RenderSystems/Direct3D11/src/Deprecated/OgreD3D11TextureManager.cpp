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
#include "OgreD3D11TextureManager.h"
#include "OgreD3D11Texture.h"
#include "OgreD3D11DepthTexture.h"
#include "OgreD3D11NullTexture.h"
#include "OgreRoot.h"
#include "OgreLogManager.h"
#include "OgreD3D11RenderSystem.h"
#include "OgreD3D11Device.h"
#include "OgreD3D11Mappings.h"

namespace Ogre 
{
    //---------------------------------------------------------------------
    D3D11TextureManager::D3D11TextureManager( D3D11Device & device ) : TextureManager(), mDevice (device)
    {
        if( mDevice.isNull())
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Invalid Direct3DDevice passed", "D3D11TextureManager::D3D11TextureManager" );
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    //---------------------------------------------------------------------
    D3D11TextureManager::~D3D11TextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

    }
    //---------------------------------------------------------------------
    Resource* D3D11TextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        if( createParams )
        {
            if( createParams->find( "DepthTexture" ) != createParams->end() )
            {
                const bool shareableDepthBuffer = createParams->find( "shareableDepthBuffer" ) !=
                                                                                createParams->end();
                return new D3D11DepthTexture( shareableDepthBuffer, this, name, handle, group,
                                              isManual, loader, mDevice );
            }

            NameValuePairList::const_iterator it = createParams->find( "SpecialFormat" );
            if( it != createParams->end() && it->second == "PF_NULL" )
            {
                return new D3D11NullTexture( this, name, handle, group,
                                             isManual, loader, mDevice );
            }
        }

        return new D3D11Texture(this, name, handle, group, isManual, loader, mDevice); 
    }
    //---------------------------------------------------------------------
    PixelFormat D3D11TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Basic filtering
        DXGI_FORMAT d3dPF = D3D11Mappings::_getPF(D3D11Mappings::_getClosestSupportedPF(format));

        return D3D11Mappings::_getPF(d3dPF);
    }
    //---------------------------------------------------------------------
    bool D3D11TextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
        bool preciseFormatOnly)
    {
        if (!preciseFormatOnly)
            format = getNativeFormat(ttype, format, usage);

        D3D11RenderSystem* rs = static_cast<D3D11RenderSystem*>(
            Root::getSingleton().getRenderSystem());

        return rs->_checkTextureFilteringSupported(ttype, format, usage);
    }
    //---------------------------------------------------------------------
}
