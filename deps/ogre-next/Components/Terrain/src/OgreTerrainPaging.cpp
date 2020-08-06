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
#include "OgreTerrainPaging.h"
#include "OgreTerrainPagedWorldSection.h"
#include "OgreTerrainGroup.h"
#include "OgrePagedWorld.h"
#include "OgrePageManager.h"

namespace Ogre
{
    //---------------------------------------------------------------------
    TerrainPaging::TerrainPaging(PageManager* pageMgr)
        : mManager(pageMgr)
    {
        mManager->addWorldSectionFactory(&mSectionFactory);
    }
    //---------------------------------------------------------------------
    TerrainPaging::~TerrainPaging()
    {
        mManager->removeWorldSectionFactory(&mSectionFactory);
    }
    //---------------------------------------------------------------------
    TerrainPagedWorldSection* TerrainPaging::createWorldSection(
        PagedWorld* world, TerrainGroup* terrainGroup, 
        Real loadRadius, Real holdRadius, int32 minX, int32 minY, int32 maxX, int32 maxY, 
        const String& sectionName, uint32 loadingIntervalMs)
    {
        TerrainPagedWorldSection* ret = static_cast<TerrainPagedWorldSection*>(
            world->createSection(terrainGroup->getSceneManager(), SectionFactory::FACTORY_NAME, sectionName));

        ret->init(terrainGroup);
        ret->setLoadRadius(loadRadius);
        ret->setHoldRadius(holdRadius);
        ret->setPageRange(minX, minY, maxX, maxY);
        ret->setLoadingIntervalMs(loadingIntervalMs);

        return ret;

    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    const String TerrainPaging::SectionFactory::FACTORY_NAME("Terrain");

    const String& TerrainPaging::SectionFactory::getName() const
    {
        return FACTORY_NAME;
    }
    //---------------------------------------------------------------------
    PagedWorldSection* TerrainPaging::SectionFactory::createInstance(const String& name, PagedWorld* parent, SceneManager* sm)
    {
        return OGRE_NEW TerrainPagedWorldSection(name, parent, sm);
    }
    //---------------------------------------------------------------------
    void TerrainPaging::SectionFactory::destroyInstance(PagedWorldSection* s)
    {
        OGRE_DELETE s;
    }


}

