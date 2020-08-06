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
#include "OgreTerrainMaterialGeneratorA.h"
#include "OgreRoot.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTextureManager.h"
#include "OgreTexture.h"
#include "OgreTerrain.h"
#include "OgreManualObject.h"
#include "OgreCamera.h"
#include "OgreViewport.h"
#include "OgreRenderSystem.h"
#include "OgreRenderTarget.h"
#include "OgreRenderTexture.h"
#include "OgreSceneNode.h"

#include "Compositor/OgreCompositorManager2.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/Pass/PassScene/OgreCompositorPassSceneDef.h"

#if OGRE_COMPILER == OGRE_COMPILER_MSVC
// we do lots of conversions here, casting them all is tedious & cluttered, we know what we're doing
#   pragma warning (disable : 4244)
#endif


namespace Ogre
{
    //---------------------------------------------------------------------
    TerrainMaterialGenerator::TerrainMaterialGenerator() 
        : mActiveProfile(0)
        , mChangeCounter(0)
        , mDebugLevel(0) 
        , mCompositeMapSM(0)
        , mCompositeMapCam(0)
        , mCompositeMapRTT(0)
        , mWorkspace(0)
        , mCompositeMapPlane(0)
        , mCompositeMapLight(0)
    {

    }
    //---------------------------------------------------------------------
    TerrainMaterialGenerator::~TerrainMaterialGenerator()
    {
        for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
            OGRE_DELETE *i;

        if (mWorkspace && Root::getSingletonPtr())
        {
            CompositorManager2 *compositorManager = Root::getSingleton().getCompositorManager2();
            compositorManager->removeWorkspace( mWorkspace );
            mWorkspace = 0;
        }

        if (mCompositeMapRTT && TextureManager::getSingletonPtr())
        {
            TextureManager::getSingleton().remove(mCompositeMapRTT->getHandle());
            mCompositeMapRTT = 0;
        }

        if (mCompositeMapSM && Root::getSingletonPtr())
        {
            // will also delete cam and objects etc
            Root::getSingleton().destroySceneManager(mCompositeMapSM);
            mCompositeMapSM = 0;
            mCompositeMapCam = 0;
            mCompositeMapPlane = 0;
            mCompositeMapLight = 0;
        }
    }
    //---------------------------------------------------------------------
    void TerrainMaterialGenerator::_renderCompositeMap(size_t size, 
        const Rect& rect, const MaterialPtr& mat, const TexturePtr& destCompositeMap)
    {
        if (!mCompositeMapSM)
        {
#if OGRE_DEBUG_MODE
            // Debugging multithreaded code is a PITA, disable it.
            const size_t numThreads = 1;
            Ogre::InstancingThreadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_SINGLETHREAD;
#else
            //getNumLogicalCores() may return 0 if couldn't detect
            const size_t numThreads = std::max<size_t>( 1, Ogre::PlatformInformation::getNumLogicalCores() );

            Ogre::InstancingThreadedCullingMethod threadedCullingMethod = Ogre::INSTANCING_CULLING_SINGLETHREAD;

            //See doxygen documentation regarding culling methods.
            //In some cases you may still want to use single thread.
            if( numThreads > 1 )
                threadedCullingMethod = Ogre::INSTANCING_CULLING_THREADED;
#endif

            // dedicated SceneManager
            mCompositeMapSM = Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, numThreads, threadedCullingMethod);
            float camDist = 100;
            float halfCamDist = camDist * 0.5f;
            mCompositeMapCam = mCompositeMapSM->createCamera("cam");
            mCompositeMapCam->setPosition(0, 0, camDist);
            mCompositeMapCam->lookAt(Vector3::ZERO);
            mCompositeMapCam->setProjectionType(PT_ORTHOGRAPHIC);
            mCompositeMapCam->setNearClipDistance(10);
            mCompositeMapCam->setFarClipDistance(500);
            mCompositeMapCam->setOrthoWindow(camDist, camDist);

            // Just in case material relies on light auto params
            mCompositeMapLight = mCompositeMapSM->createLight();
            SceneNode *lightNode = mCompositeMapSM->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject( mCompositeMapLight );
            mCompositeMapLight->setType(Light::LT_DIRECTIONAL);

            RenderSystem* rSys = Root::getSingleton().getRenderSystem();
            Real hOffset = rSys->getHorizontalTexelOffset() / (Real)size;
            Real vOffset = rSys->getVerticalTexelOffset() / (Real)size;


            // set up scene
            mCompositeMapPlane = mCompositeMapSM->createManualObject();
            mCompositeMapPlane->begin(mat->getName());
            mCompositeMapPlane->position(-halfCamDist, halfCamDist, 0);
            mCompositeMapPlane->textureCoord(0 - hOffset, 0 - vOffset);
            mCompositeMapPlane->position(-halfCamDist, -halfCamDist, 0);
            mCompositeMapPlane->textureCoord(0 - hOffset, 1 - vOffset);
            mCompositeMapPlane->position(halfCamDist, -halfCamDist, 0);
            mCompositeMapPlane->textureCoord(1 - hOffset, 1 - vOffset);
            mCompositeMapPlane->position(halfCamDist, halfCamDist, 0);
            mCompositeMapPlane->textureCoord(1 - hOffset, 0 - vOffset);
            mCompositeMapPlane->quad(0, 1, 2, 3);
            mCompositeMapPlane->end();
            mCompositeMapSM->getRootSceneNode()->attachObject(mCompositeMapPlane);

        }

        // update
        mCompositeMapPlane->setMaterialName(0, mat->getName());
        TerrainGlobalOptions& globalopts = TerrainGlobalOptions::getSingleton();
        mCompositeMapLight->setDirection(globalopts.getLightMapDirection());
        mCompositeMapLight->setDiffuseColour(globalopts.getCompositeMapDiffuse());
        mCompositeMapSM->setAmbientLight(globalopts.getCompositeMapAmbient());


        // check for size change (allow smaller to be reused)
        if (mCompositeMapRTT && size != mCompositeMapRTT->getWidth())
        {
            TextureManager::getSingleton().remove(mCompositeMapRTT->getHandle());
            mCompositeMapRTT = 0;
        }

        if (!mCompositeMapRTT)
        {
            mCompositeMapRTT = TextureManager::getSingleton().createManual(
                mCompositeMapSM->getName() + "/compRTT", 
                ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, TEX_TYPE_2D, static_cast<uint>(size), static_cast<uint>(size), 0, PF_BYTE_RGBA,
                TU_RENDERTARGET).get();

            CompositorManager2 *compositorManager = Root::getSingleton().getCompositorManager2();

            IdString workspaceName( "Ogre Terrain Material Generator" );
            
            if( !compositorManager->hasWorkspaceDefinition( workspaceName ) )
            {
                CompositorNodeDef *nodeDef = compositorManager->addNodeDefinition(
                                                            "Ogre Terrain Material Generator Node" );

                //Input texture
                nodeDef->addTextureSourceName( "RTT", 0, TextureDefinitionBase::TEXTURE_INPUT );

                nodeDef->setNumTargetPass( 1 );
                {
                    CompositorTargetDef *targetDef = nodeDef->addTargetPass( "RTT" );
                    targetDef->setNumPasses( 2 );
                    {
                        {
                            targetDef->addPass( PASS_CLEAR );
                        }
                        {
                            CompositorPassSceneDef *passScene = static_cast<CompositorPassSceneDef*>
                                                                    ( targetDef->addPass( PASS_SCENE ) );
                            passScene->mIncludeOverlays = false;
                        }
                    }
                }

                CompositorWorkspaceDef *workDef =
                                            compositorManager->addWorkspaceDefinition( workspaceName );
                workDef->connectOutput( nodeDef->getName(), 0 );
            }

            mWorkspace = compositorManager->addWorkspace( mCompositeMapSM,
                                             mCompositeMapRTT->getBuffer()->getRenderTarget(),
                                             mCompositeMapCam, workspaceName, true, 0 );
        }

        // calculate the area we need to update
        Real vpleft = (Real)rect.left / (Real)size;
        Real vptop = (Real)rect.top / (Real)size;
        Real vpright = (Real)rect.right / (Real)size;
        Real vpbottom = (Real)rect.bottom / (Real)size;

        mCompositeMapCam->setWindow(vpleft, vptop, vpright, vpbottom);

        // We have an RTT, we want to copy the results into a regular texture
        // That's because in non-update scenarios we don't want to keep an RTT
        // around. We use a single RTT to serve all terrain pages which is more
        // efficient.
        Box box(static_cast<uint32>(rect.left),
                static_cast<uint32>(rect.top),
                static_cast<uint32>(rect.right),
                static_cast<uint32>(rect.bottom));
        destCompositeMap->getBuffer()->blit(mCompositeMapRTT->getBuffer(), box, box);

        
    }
    //---------------------------------------------------------------------
    //---------------------------------------------------------------------
    void TerrainMaterialGenerator::Profile::updateCompositeMap(const Terrain* terrain, const Rect& rect)
    {
        // convert point-space rect into image space
        size_t compSize = terrain->getCompositeMap()->getWidth();
        Rect imgRect;
        Vector3 inVec, outVec;
        inVec.x = rect.left;
        inVec.y = rect.bottom - 1; // this is 'top' in image space, also make inclusive
        terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
        imgRect.left = outVec.x * compSize;
        imgRect.top = (1.0f - outVec.y) * compSize;
        inVec.x = rect.right - 1;
        inVec.y = rect.top; // this is 'bottom' in image space
        terrain->convertPosition(Terrain::POINT_SPACE, inVec, Terrain::TERRAIN_SPACE, outVec);
        imgRect.right = outVec.x * (Real)compSize + 1; 
        imgRect.bottom = (1.0 - outVec.y) * compSize + 1;

        imgRect.left = std::max(0L, imgRect.left);
        imgRect.top = std::max(0L, imgRect.top);
        imgRect.right = std::min((long)compSize, imgRect.right);
        imgRect.bottom = std::min((long)compSize, imgRect.bottom);
        

        mParent->_renderCompositeMap(
            compSize, imgRect, 
            terrain->getCompositeMapMaterial(), 
            terrain->getCompositeMap());

    }


}

