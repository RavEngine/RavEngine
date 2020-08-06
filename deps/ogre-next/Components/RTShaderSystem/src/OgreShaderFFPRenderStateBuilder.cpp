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
#include "OgreShaderFFPRenderStateBuilder.h"
#ifdef RTSHADER_SYSTEM_BUILD_CORE_SHADERS
#include "OgreShaderGenerator.h"
#include "OgreShaderRenderState.h"
#include "OgreShaderFFPTransform.h"
#include "OgreShaderFFPLighting.h"
#include "OgreShaderFFPColour.h"
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderFFPFog.h"
#include "OgrePass.h"
#include "OgreLogManager.h"
#include "OgreShaderFFPRenderState.h"
#include "OgreTechnique.h"
#include "OgreShaderFFPAlphaTest.h"
#include "OgreCommon.h"
#include "OgreRenderSystem.h"
#include "OgreRoot.h"

namespace Ogre {

//-----------------------------------------------------------------------
template<> 
RTShader::FFPRenderStateBuilder* Singleton<RTShader::FFPRenderStateBuilder>::msSingleton = 0;

namespace RTShader {


//-----------------------------------------------------------------------
FFPRenderStateBuilder* FFPRenderStateBuilder::getSingletonPtr()
{
    return msSingleton;
}

//-----------------------------------------------------------------------
FFPRenderStateBuilder& FFPRenderStateBuilder::getSingleton()
{
    assert( msSingleton );  
    return ( *msSingleton );
}

//-----------------------------------------------------------------------------
FFPRenderStateBuilder::FFPRenderStateBuilder()
{
    

}

//-----------------------------------------------------------------------------
FFPRenderStateBuilder::~FFPRenderStateBuilder()
{
    

}

//-----------------------------------------------------------------------------
bool FFPRenderStateBuilder::initialize()
{
    SubRenderStateFactory* curFactory;

    curFactory = OGRE_NEW FFPTransformFactory;  
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mFFPSubRenderStateFactoryList.push_back(curFactory);

    curFactory = OGRE_NEW FFPColourFactory; 
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mFFPSubRenderStateFactoryList.push_back(curFactory);

    curFactory = OGRE_NEW FFPLightingFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mFFPSubRenderStateFactoryList.push_back(curFactory);

    curFactory = OGRE_NEW FFPTexturingFactory;
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mFFPSubRenderStateFactoryList.push_back(curFactory);

    curFactory = OGRE_NEW FFPFogFactory;    
    ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
    mFFPSubRenderStateFactoryList.push_back(curFactory);

	curFactory = OGRE_NEW FFPAlphaTestFactory;	
	ShaderGenerator::getSingleton().addSubRenderStateFactory(curFactory);
	mFFPSubRenderStateFactoryList.push_back(curFactory);
	
    return true;
}

//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::destroy()
{
    SubRenderStateFactoryIterator it;

    for (it = mFFPSubRenderStateFactoryList.begin(); it != mFFPSubRenderStateFactoryList.end(); ++it)
    {
        ShaderGenerator::getSingleton().removeSubRenderStateFactory(*it);       
        OGRE_DELETE *it;        
    }
    mFFPSubRenderStateFactoryList.clear();
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildRenderState(ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState)
{
    renderState->reset();

    // Build transformation sub state.
    buildFFPSubRenderState(FFP_TRANSFORM, FFPTransform::Type, sgPass, renderState); 

    // Build colour sub state.
    buildFFPSubRenderState(FFP_COLOUR, FFPColour::Type, sgPass, renderState);

    // Build lighting sub state.
    buildFFPSubRenderState(FFP_LIGHTING, FFPLighting::Type, sgPass, renderState);

    // Build texturing sub state.
    buildFFPSubRenderState(FFP_TEXTURING, FFPTexturing::Type, sgPass, renderState); 
    
    // Build fog sub state.
    buildFFPSubRenderState(FFP_FOG, FFPFog::Type, sgPass, renderState);
	
	RenderSystem* rs = Root::getSingleton().getRenderSystem();
	if (rs->getName().find("Direct3D11") != String::npos && 
		sgPass->getSrcPass()->getAlphaRejectFunction() != CMPF_ALWAYS_PASS)
	{
		buildFFPSubRenderState(FFP_ALPHA_TEST, FFPAlphaTest::Type, sgPass, renderState);
	}
	
    // Resolve colour stage flags.
    resolveColourStageFlags(sgPass, renderState);

}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::buildFFPSubRenderState(int subRenderStateOrder, const String& subRenderStateType,
                                                ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState)
{
    SubRenderState* subRenderState;

    subRenderState = sgPass->getCustomFFPSubState(subRenderStateOrder);

    if (subRenderState == NULL) 
    {
        subRenderState = ShaderGenerator::getSingleton().createSubRenderState(subRenderStateType);      
    }

    if (subRenderState->preAddToRenderState(renderState, sgPass->getSrcPass(), sgPass->getDstPass()))
    {
        renderState->addSubRenderStateInstance(subRenderState);
    }
    else
    {       
        ShaderGenerator::getSingleton().destroySubRenderState(subRenderState);              
    }
}


//-----------------------------------------------------------------------------
void FFPRenderStateBuilder::resolveColourStageFlags( ShaderGenerator::SGPass* sgPass, TargetRenderState* renderState )
{
    const SubRenderStateList& subRenderStateList = renderState->getTemplateSubRenderStateList();
    FFPColour* colourSubState = NULL;

    // Find the colour sub state.
    for (SubRenderStateListConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
    {
        SubRenderState* curSubRenderState = *it;

        if (curSubRenderState->getType() == FFPColour::Type)
        {
            colourSubState = static_cast<FFPColour*>(curSubRenderState);
            break;
        }
    }
    
    for (SubRenderStateListConstIterator it=subRenderStateList.begin(); it != subRenderStateList.end(); ++it)
    {
        SubRenderState* curSubRenderState = *it;

        // Add vertex shader specular lighting output in case of specular enabled.
        if (curSubRenderState->getType() == FFPLighting::Type && colourSubState != NULL)
        {
            colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_DIFFUSE);

            Pass* srcPass = sgPass->getSrcPass();

            if (srcPass->getShininess() > 0.0 &&
                srcPass->getSpecular() != ColourValue::Black)
            {
                colourSubState->addResolveStageMask(FFPColour::SF_VS_OUTPUT_SPECULAR);              
            }   
            break;
        }
    }
}



}
}

#endif
