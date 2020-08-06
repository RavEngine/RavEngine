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
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"
#include "OgreD3D9Mappings.h"
#include "OgreRoot.h"
#include "OgreD3D9RenderSystem.h"

namespace Ogre 
{
    D3D9TextureManager::D3D9TextureManager() : TextureManager()
    {       
        // register with group manager
        ResourceGroupManager::getSingleton()._registerResourceManager(mResourceType, this);
    }
    
    D3D9TextureManager::~D3D9TextureManager()
    {
        // unregister with group manager
        ResourceGroupManager::getSingleton()._unregisterResourceManager(mResourceType);

    }

    Resource* D3D9TextureManager::createImpl(const String& name, 
        ResourceHandle handle, const String& group, bool isManual, 
        ManualResourceLoader* loader, const NameValuePairList* createParams)
    {
        D3D9Texture* ret = OGRE_NEW D3D9Texture(this, name, handle, group, isManual, loader);       
        return ret;
    }

    PixelFormat D3D9TextureManager::getNativeFormat(TextureType ttype, PixelFormat format, int usage)
    {
        // Basic filtering
        D3DFORMAT d3dPF = D3D9Mappings::_getPF(D3D9Mappings::_getClosestSupportedPF(format));

        // Calculate usage
        DWORD d3dusage = 0;
        D3DPOOL pool = D3DPOOL_MANAGED;
        if (D3D9RenderSystem::isDirectX9Ex())
        {
            pool = D3DPOOL_DEFAULT;
        }
        if (usage & TU_RENDERTARGET) 
        {
            d3dusage |= D3DUSAGE_RENDERTARGET;
            pool = D3DPOOL_DEFAULT;
        }
        if (usage & TU_DYNAMIC)
        {
            d3dusage |= D3DUSAGE_DYNAMIC;
            pool = D3DPOOL_DEFAULT;
        }

        IDirect3DDevice9* pCurDevice = D3D9RenderSystem::getActiveD3D9Device();

        // Use D3DX to adjust pixel format
        switch(ttype)
        {
        case TEX_TYPE_1D:
        case TEX_TYPE_2D:
            D3DXCheckTextureRequirements(pCurDevice, NULL, NULL, NULL, d3dusage, &d3dPF, pool);
            break;
        case TEX_TYPE_3D:
            D3DXCheckVolumeTextureRequirements(pCurDevice, NULL, NULL, NULL, NULL, d3dusage, &d3dPF, pool);
            break;
        case TEX_TYPE_CUBE_MAP:
            D3DXCheckCubeTextureRequirements(pCurDevice, NULL, NULL, d3dusage, &d3dPF, pool);
            break;
        };

        return D3D9Mappings::_getPF(d3dPF);
    }

    bool D3D9TextureManager::isHardwareFilteringSupported(TextureType ttype, PixelFormat format, int usage,
        bool preciseFormatOnly)
    {
        if (!preciseFormatOnly)
            format = getNativeFormat(ttype, format, usage);

        D3D9RenderSystem* rs = static_cast<D3D9RenderSystem*>(
            Root::getSingleton().getRenderSystem());

        return rs->_checkTextureFilteringSupported(ttype, format, usage);
    }
}
