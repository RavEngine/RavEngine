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
#include "OgreShaderExIntegratedPSSM3.h"
#ifdef RTSHADER_SYSTEM_BUILD_EXT_SHADERS
#include "OgreShaderFFPRenderState.h"
#include "OgreShaderFFPTexturing.h"
#include "OgreShaderProgram.h"
#include "OgreShaderParameter.h"
#include "OgreAutoParamDataSource.h"
#include "OgreShaderProgramSet.h"
#include "OgrePass.h"
#include "OgreHardwarePixelBuffer.h"
#include "OgreTechnique.h"
#include "OgreShaderGenerator.h"

namespace Ogre {
namespace RTShader {

/************************************************************************/
/*                                                                      */
/************************************************************************/
String IntegratedPSSM3::Type = "SGX_IntegratedPSSM3";

//-----------------------------------------------------------------------
IntegratedPSSM3::IntegratedPSSM3()
{   
    
}

//-----------------------------------------------------------------------
const String& IntegratedPSSM3::getType() const
{
    return Type;
}


//-----------------------------------------------------------------------
int IntegratedPSSM3::getExecutionOrder() const
{
    return FFP_TEXTURING + 1;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::updateGpuProgramsParams(Renderable* rend, Pass* pass, 
                                             const AutoParamDataSource* source, 
                                             const LightList* pLightList)
{
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();
    size_t shadowIndex = 0;

    while(it != mShadowTextureParamsList.end())
    {                       
        it->mWorldViewProjMatrix->setGpuParameter(source->getTextureWorldViewProjMatrix(shadowIndex));              
        it->mInvTextureSize->setGpuParameter(source->getInverseTextureSize(it->mTextureSamplerIndex));
        
        ++it;
        ++shadowIndex;
    }

    Vector4 vSplitPoints;
	//get the split points from the auto params 
	const vector<Real>::type &pssmSplitPoints = source->getPssmSplits(0);
	vSplitPoints.x = pssmSplitPoints[1];
	vSplitPoints.y = pssmSplitPoints[2];
    vSplitPoints.z = 0.0;
    vSplitPoints.w = 0.0;

    mPSSplitPoints->setGpuParameter(vSplitPoints);

}

//-----------------------------------------------------------------------
void IntegratedPSSM3::copyFrom(const SubRenderState& rhs)
{
    const IntegratedPSSM3& rhsPssm= static_cast<const IntegratedPSSM3&>(rhs);

    mShadowTextureParamsList.resize(rhsPssm.mShadowTextureParamsList.size());

    ShadowTextureParamsConstIterator itSrc = rhsPssm.mShadowTextureParamsList.begin();
    ShadowTextureParamsIterator itDst = mShadowTextureParamsList.begin();

    while(itDst != mShadowTextureParamsList.end())
    {
        itDst->mMaxRange = itSrc->mMaxRange;        
        ++itSrc;
        ++itDst;
    }
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::preAddToRenderState(const RenderState* renderState, 
                                         Pass* srcPass, Pass* dstPass)
{
    if (srcPass->getLightingEnabled() == false ||
        srcPass->getParent()->getParent()->getReceiveShadows() == false)
        return false;

    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();

    while(it != mShadowTextureParamsList.end())
    {
        TextureUnitState* curShadowTexture = dstPass->createTextureUnitState();
            
        curShadowTexture->setContentType(TextureUnitState::CONTENT_SHADOW);
        curShadowTexture->setTextureAddressingMode(TextureUnitState::TAM_BORDER);
        curShadowTexture->setTextureBorderColour(ColourValue::White);
        it->mTextureSamplerIndex = dstPass->getNumTextureUnitStates() - 1;
        ++it;
    }

    

    return true;
}

//-----------------------------------------------------------------------
void IntegratedPSSM3::setSplitPoints(const SplitPointList& newSplitPoints)
{
    if (newSplitPoints.size() != 4)
    {
        OGRE_EXCEPT(Exception::ERR_DUPLICATE_ITEM,
            "IntegratedPSSM3 sub render state supports only 4 split points",
            "IntegratedPSSM3::setSplitPoints");
    }

    mShadowTextureParamsList.resize(newSplitPoints.size() - 1);

    for (unsigned int i=1; i < newSplitPoints.size(); ++i)
    {
        ShadowTextureParams& curParams = mShadowTextureParamsList[i-1];

        curParams.mMaxRange             = newSplitPoints[i];        
    }
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveParameters(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuVertexProgram();
    Program* psProgram = programSet->getCpuFragmentProgram();
    Function* vsMain = vsProgram->getEntryPointFunction();
    Function* psMain = psProgram->getEntryPointFunction();
    
    // Get input position parameter.
    mVSInPos = vsMain->getParameterBySemantic(vsMain->getInputParameters(), Parameter::SPS_POSITION, 0);
    
    // Get output position parameter.
    mVSOutPos = vsMain->getParameterBySemantic(vsMain->getOutputParameters(), Parameter::SPS_POSITION, 0);
    
    // Resolve vertex shader output depth.      
    mVSOutDepth = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1, 
        Parameter::SPC_DEPTH_VIEW_SPACE,
        GCT_FLOAT1);
    
    // Resolve input depth parameter.
    mPSInDepth = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, mVSOutDepth->getIndex(), 
        mVSOutDepth->getContent(),
        GCT_FLOAT1);
    
    // Get in/local diffuse parameter.
    mPSDiffuse = psMain->getParameterBySemantic(psMain->getInputParameters(), Parameter::SPS_COLOR, 0);
    if (mPSDiffuse.get() == NULL)   
    {
        mPSDiffuse = psMain->getParameterBySemantic(psMain->getLocalParameters(), Parameter::SPS_COLOR, 0);
    }
    
    // Resolve output diffuse parameter.
    mPSOutDiffuse = psMain->resolveOutputParameter(Parameter::SPS_COLOR, 0, Parameter::SPC_COLOR_DIFFUSE, GCT_FLOAT4);
    
    // Get in/local specular parameter.
    mPSSpecualr = psMain->getParameterBySemantic(psMain->getInputParameters(), Parameter::SPS_COLOR, 1);
    if (mPSSpecualr.get() == NULL)  
    {
        mPSSpecualr = psMain->getParameterBySemantic(psMain->getLocalParameters(), Parameter::SPS_COLOR, 1);
    }
    
    // Resolve computed local shadow colour parameter.
    mPSLocalShadowFactor = psMain->resolveLocalParameter(Parameter::SPS_UNKNOWN, 0, "lShadowFactor", GCT_FLOAT1);

    // Resolve computed local shadow colour parameter.
    mPSSplitPoints = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "pssm_split_points");

    // Get derived scene colour.
    mPSDerivedSceneColour = psProgram->resolveAutoParameterInt(GpuProgramParameters::ACT_DERIVED_SCENE_COLOUR, 0);
    
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();
    int lightIndex = 0;

    while(it != mShadowTextureParamsList.end())
    {
        it->mWorldViewProjMatrix = vsProgram->resolveParameter(GCT_MATRIX_4X4, -1, (uint16)GPV_PER_OBJECT, "world_texture_view_proj");      

        it->mVSOutLightPosition = vsMain->resolveOutputParameter(Parameter::SPS_TEXTURE_COORDINATES, -1,
            Parameter::Content(Parameter::SPC_POSITION_LIGHT_SPACE0 + lightIndex),
            GCT_FLOAT4);        

        it->mPSInLightPosition = psMain->resolveInputParameter(Parameter::SPS_TEXTURE_COORDINATES, 
            it->mVSOutLightPosition->getIndex(),
            it->mVSOutLightPosition->getContent(),
            GCT_FLOAT4);    

        it->mTextureSampler = psProgram->resolveParameter(GCT_SAMPLER2D, it->mTextureSamplerIndex, (uint16)GPV_GLOBAL, "shadow_map");  

		if (Ogre::RTShader::ShaderGenerator::getSingletonPtr()->IsHlsl4())
			it->mTextureSamplerState = psProgram->resolveParameter(GCT_SAMPLER_STATE, it->mTextureSamplerIndex, (uint16)GPV_GLOBAL, "shadow_map_sampler");

        it->mInvTextureSize = psProgram->resolveParameter(GCT_FLOAT4, -1, (uint16)GPV_GLOBAL, "inv_shadow_texture_size");       

        if (!(it->mInvTextureSize.get()) || !(it->mTextureSampler.get()) || !(it->mPSInLightPosition.get()) ||
            !(it->mVSOutLightPosition.get()) || !(it->mWorldViewProjMatrix.get()))
        {
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "IntegratedPSSM3::resolveParameters" );
        }

        ++lightIndex;
        ++it;
    }

    if (!(mVSInPos.get()) || !(mVSOutPos.get()) || !(mVSOutDepth.get()) || !(mPSInDepth.get()) || !(mPSDiffuse.get()) || !(mPSOutDiffuse.get()) || 
        !(mPSSpecualr.get()) || !(mPSLocalShadowFactor.get()) || !(mPSSplitPoints.get()) || !(mPSDerivedSceneColour.get()))
    {
        OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, 
                "Not all parameters could be constructed for the sub-render state.",
                "IntegratedPSSM3::resolveParameters" );
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::resolveDependencies(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuVertexProgram();
    Program* psProgram = programSet->getCpuFragmentProgram();

    vsProgram->addDependency(FFP_LIB_COMMON);
    
    psProgram->addDependency(FFP_LIB_COMMON);
    psProgram->addDependency(SGX_LIB_INTEGRATEDPSSM);

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addFunctionInvocations(ProgramSet* programSet)
{
    Program* vsProgram = programSet->getCpuVertexProgram(); 
    Function* vsMain = vsProgram->getEntryPointFunction();  
    Program* psProgram = programSet->getCpuFragmentProgram();
    int internalCounter;

    // Add vertex shader invocations.
    internalCounter = 0;
    if (false == addVSInvocation(vsMain, FFP_VS_TEXTURING + 1, internalCounter))
        return false;

    // Add pixel shader invocations.
    internalCounter = 0;
    if (false == addPSInvocation(psProgram, FFP_PS_COLOUR_BEGIN + 2, internalCounter))
        return false;

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addVSInvocation(Function* vsMain, const int groupOrder, int& internalCounter)
{
    FunctionInvocation* curFuncInvocation;

    // Output the vertex depth in camera space.
    curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);                                
    curFuncInvocation->pushOperand(mVSOutPos, Operand::OPS_IN, Operand::OPM_Z); 
    curFuncInvocation->pushOperand(mVSOutDepth, Operand::OPS_OUT);  
    vsMain->addAtomInstance(curFuncInvocation); 


    // Compute world space position.    
    ShadowTextureParamsIterator it = mShadowTextureParamsList.begin();

    while(it != mShadowTextureParamsList.end())
    {
        curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_TRANSFORM, groupOrder, internalCounter++);                                 
        curFuncInvocation->pushOperand(it->mWorldViewProjMatrix, Operand::OPS_IN);  
        curFuncInvocation->pushOperand(mVSInPos, Operand::OPS_IN);
        curFuncInvocation->pushOperand(it->mVSOutLightPosition, Operand::OPS_OUT);  
        vsMain->addAtomInstance(curFuncInvocation); 

        ++it;
    }

    return true;
}

//-----------------------------------------------------------------------
bool IntegratedPSSM3::addPSInvocation(Program* psProgram, const int groupOrder, int& internalCounter)
{
    Function* psMain = psProgram->getEntryPointFunction();
    FunctionInvocation* curFuncInvocation;

    ShadowTextureParams& splitParams0 = mShadowTextureParamsList[0];
    ShadowTextureParams& splitParams1 = mShadowTextureParamsList[1];
    ShadowTextureParams& splitParams2 = mShadowTextureParamsList[2];

	if (Ogre::RTShader::ShaderGenerator::getSingletonPtr()->IsHlsl4())
	{
		FFPTexturing::AddTextureSampleWrapperInvocation(splitParams0.mTextureSampler, splitParams0.mTextureSamplerState, GCT_SAMPLER2D, psMain, groupOrder, internalCounter);
		FFPTexturing::AddTextureSampleWrapperInvocation(splitParams1.mTextureSampler, splitParams1.mTextureSamplerState, GCT_SAMPLER2D, psMain, groupOrder, internalCounter);
		FFPTexturing::AddTextureSampleWrapperInvocation(splitParams2.mTextureSampler, splitParams2.mTextureSamplerState, GCT_SAMPLER2D, psMain, groupOrder, internalCounter);
	}
    // Compute shadow factor.
    curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_COMPUTE_SHADOW_COLOUR3, groupOrder, internalCounter++);                                
    curFuncInvocation->pushOperand(mPSInDepth, Operand::OPS_IN);    
    curFuncInvocation->pushOperand(mPSSplitPoints, Operand::OPS_IN);        
    curFuncInvocation->pushOperand(splitParams0.mPSInLightPosition, Operand::OPS_IN);   
    curFuncInvocation->pushOperand(splitParams1.mPSInLightPosition, Operand::OPS_IN);   
    curFuncInvocation->pushOperand(splitParams2.mPSInLightPosition, Operand::OPS_IN);   

	if (Ogre::RTShader::ShaderGenerator::getSingletonPtr()->IsHlsl4())
	{
		curFuncInvocation->pushOperand(FFPTexturing::GetSamplerWrapperParam(splitParams0.mTextureSampler, psMain), Operand::OPS_IN);
		curFuncInvocation->pushOperand(FFPTexturing::GetSamplerWrapperParam(splitParams1.mTextureSampler, psMain), Operand::OPS_IN);
		curFuncInvocation->pushOperand(FFPTexturing::GetSamplerWrapperParam(splitParams2.mTextureSampler, psMain), Operand::OPS_IN);
	}
	else
	{
		curFuncInvocation->pushOperand(splitParams0.mTextureSampler, Operand::OPS_IN);
		curFuncInvocation->pushOperand(splitParams1.mTextureSampler, Operand::OPS_IN);
		curFuncInvocation->pushOperand(splitParams2.mTextureSampler, Operand::OPS_IN);
	}
    curFuncInvocation->pushOperand(splitParams0.mInvTextureSize, Operand::OPS_IN);  
    curFuncInvocation->pushOperand(splitParams1.mInvTextureSize, Operand::OPS_IN);  
    curFuncInvocation->pushOperand(splitParams2.mInvTextureSize, Operand::OPS_IN);
    curFuncInvocation->pushOperand(mPSLocalShadowFactor, Operand::OPS_OUT); 

    psMain->addAtomInstance(curFuncInvocation); 
    
    // Apply shadow factor on diffuse colour.
    curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_APPLYSHADOWFACTOR_DIFFUSE, groupOrder, internalCounter++);                                 
    curFuncInvocation->pushOperand(mPSDerivedSceneColour, Operand::OPS_IN);     
    curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);    
    curFuncInvocation->pushOperand(mPSLocalShadowFactor, Operand::OPS_IN);  
    curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_OUT);   
    psMain->addAtomInstance(curFuncInvocation); 

    // Apply shadow factor on specular colour.
    curFuncInvocation = OGRE_NEW FunctionInvocation(SGX_FUNC_MODULATE_SCALAR, groupOrder, internalCounter++);                               
    curFuncInvocation->pushOperand(mPSLocalShadowFactor, Operand::OPS_IN);      
    curFuncInvocation->pushOperand(mPSSpecualr, Operand::OPS_IN);   
    curFuncInvocation->pushOperand(mPSSpecualr, Operand::OPS_OUT);      
    psMain->addAtomInstance(curFuncInvocation);

    // Assign the local diffuse to output diffuse.
    curFuncInvocation = OGRE_NEW FunctionInvocation(FFP_FUNC_ASSIGN, groupOrder, internalCounter++);                                
    curFuncInvocation->pushOperand(mPSDiffuse, Operand::OPS_IN);    
    curFuncInvocation->pushOperand(mPSOutDiffuse, Operand::OPS_OUT);    
    psMain->addAtomInstance(curFuncInvocation);

    return true;
}



//-----------------------------------------------------------------------
const String& IntegratedPSSM3Factory::getType() const
{
    return IntegratedPSSM3::Type;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstance(ScriptCompiler* compiler, 
                                                      PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator)
{
    if (prop->name == "integrated_pssm4")
    {       
        if (prop->values.size() != 4)
        {
            compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
        }
        else
        {
            IntegratedPSSM3::SplitPointList splitPointList; 

            AbstractNodeList::const_iterator it = prop->values.begin();
            AbstractNodeList::const_iterator itEnd = prop->values.end();

            while(it != itEnd)
            {
                Real curSplitValue;
                
                if (false == SGScriptTranslator::getReal(*it, &curSplitValue))
                {
                    compiler->addError(ScriptCompiler::CE_INVALIDPARAMETERS, prop->file, prop->line);
                    break;
                }

                splitPointList.push_back(curSplitValue);

                ++it;
            }

            if (splitPointList.size() == 4)
            {
                SubRenderState* subRenderState = createOrRetrieveInstance(translator);
                IntegratedPSSM3* pssmSubRenderState = static_cast<IntegratedPSSM3*>(subRenderState);

                pssmSubRenderState->setSplitPoints(splitPointList);

                return pssmSubRenderState;
            }
        }       
    }

    return NULL;
}

//-----------------------------------------------------------------------
SubRenderState* IntegratedPSSM3Factory::createInstanceImpl()
{
    return OGRE_NEW IntegratedPSSM3;
}

}
}

#endif
