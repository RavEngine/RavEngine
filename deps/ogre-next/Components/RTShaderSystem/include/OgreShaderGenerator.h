/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org

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
#ifndef _ShaderGenerator_
#define _ShaderGenerator_

#include "OgreShaderPrerequisites.h"
#include "OgreSingleton.h"
#include "OgreFileSystemLayer.h"
#include "OgreRenderObjectListener.h"
#include "OgreSceneManager.h"
#include "OgreShaderRenderState.h"
#include "OgreScriptTranslator.h"
#include "OgreShaderScriptTranslator.h"


namespace Ogre {


namespace RTShader {

/** \addtogroup Core
*  @{
*/
/** \addtogroup RTShader
*  @{
*/

/** Shader generator system main interface. This singleton based class
enables automatic generation of shader code based on existing material techniques.
*/
class _OgreRTSSExport ShaderGenerator : public Singleton<ShaderGenerator>, public RTShaderSystemAlloc
{
// Interface.
public:

    /** 
    Initialize the Shader Generator System.
    Return true upon success.
    */
    static bool initialize();

    /** 
    Destroy the Shader Generator instance.
    */
    static void destroy();


    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ShaderGenerator& getSingleton(); 


    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ShaderGenerator* getSingletonPtr();

    /** 
    Add a scene manager to the shader generator scene managers list.
    @param sceneMgr The scene manager to add to the list.
    */
    void addSceneManager(SceneManager* sceneMgr);

    /** 
    Remove a scene manager from the shader generator scene managers list.
    @param sceneMgr The scene manager to remove from the list.
    */
    void removeSceneManager(SceneManager* sceneMgr);

    /** 
    Get the active scene manager that is doint the actual scene rendering.
    This attribute will be update on the call to preFindVisibleObjects. 
    */
    SceneManager* getActiveSceneManager();

    /** 
    Set the active scene manager against which new render states are compiled.
    Note that normally the setting of the active scene manager is updated through the
    preFindVisibleObjects method.
    */

    void _setActiveSceneManager(SceneManager* sceneManager);

    /** 
    Set the target shader language.
    @param shaderLanguage The output shader language to use.    
    @remarks The default shader language is cg.
    */
    void setTargetLanguage(const String& shaderLanguage,const float version = 1.0);

    /** 
    Return if hlsl 4.0 shading language is currently in use.        
    */
    bool IsHlsl4() const { return mShaderLanguage == "hlsl" && mShaderLanguageVersion == 4.0f; }
    /** 
    Return the target shader language currently in use.     
    */
    const String& getTargetLanguage() const { return mShaderLanguage; }

    /** 
    Return the target shader language version currently in use.     
    */
    float getTargetLanguageVersion() const { return mShaderLanguageVersion; }

    /** 
    Set the output vertex shader target profiles.
    @param vertexShaderProfiles The target profiles for the vertex shader.  
    */
    void setVertexShaderProfiles(const String& vertexShaderProfiles);

    /** 
    Get the output vertex shader target profiles.   
    */
    const String& getVertexShaderProfiles() const { return mVertexShaderProfiles; }

    /** 
    Get the output vertex shader target profiles as list of strings.    
    */
    const StringVector& getVertexShaderProfilesList() const { return mVertexShaderProfilesList; }


    /** 
    Set the output fragment shader target profiles.
    @param fragmentShaderProfiles The target profiles for the fragment shader.  
    */
    void setFragmentShaderProfiles(const String& fragmentShaderProfiles);

    /** 
    Get the output fragment shader target profiles. 
    */
    const String& getFragmentShaderProfiles() const { return mFragmentShaderProfiles; }

    /** 
    Get the output fragment shader target profiles as list of strings.
    */
    const StringVector& getFragmentShaderProfilesList() const { return mFragmentShaderProfilesList; }

    /** 
    Set the output shader cache path. Generated shader code will be written to this path.
    In case of empty cache path shaders will be generated directly from system memory.
    @param cachePath The cache path of the shader.  
    The default is empty cache path.
    */
    void setShaderCachePath(const String& cachePath);

    /** 
    Get the output shader cache path.
    */
    const String& getShaderCachePath() const { return mShaderCachePath; }

    /** 
    Flush the shader cache. This operation will cause all active sachems to be invalidated and will
    destroy any CPU/GPU program that created by this shader generator.
    */
    void flushShaderCache();

    /** 
    Return a global render state associated with the given scheme name.
    Modifying this render state will affect all techniques that belongs to that scheme.
    This is the best way to apply global changes to all techniques.
    After altering the render state one should call invalidateScheme method in order to 
    regenerate shaders.
    @param schemeName The destination scheme name.
    */
    RenderState* getRenderState(const String& schemeName);


    typedef std::pair<RenderState*, bool> RenderStateCreateOrRetrieveResult;
    /** 
    Returns a requested render state. If the render state does not exist this function creates it.
    @param schemeName The scheme name to retrieve.
    */
    RenderStateCreateOrRetrieveResult createOrRetrieveRenderState(const String& schemeName);


    /** 
    Tells if a given render state exists
    @param schemeName The scheme name to check.
    */
    bool hasRenderState(const String& schemeName) const;
    

    /** 
    Get render state of specific pass.
    Using this method allows the user to customize the behavior of a specific pass.
    @param schemeName The destination scheme name.
    @param materialName The specific material name.
    @param passIndex The pass index.
    */
    RenderState* getRenderState(const String& schemeName, const String& materialName, unsigned short passIndex);

    /**
     Get render state of specific pass.
     Using this method allows the user to customize the behavior of a specific pass.
     @param schemeName The destination scheme name.
     @param materialName The specific material name.
     @param groupName The specific material name.
     @param passIndex The pass index.
     */
    RenderState* getRenderState(const String& schemeName, const String& materialName, const String& groupName, unsigned short passIndex);

    /** 
    Add sub render state factory. Plugins or 3d party applications may implement sub classes of
    SubRenderState interface. Add the matching factory will allow the application to create instances 
    of these sub classes.
    @param factory The factory to add.
    */
    void addSubRenderStateFactory(SubRenderStateFactory* factory);

    /** 
    Returns the number of existing factories
    */
    size_t getNumSubRenderStateFactories() const;

    /** 
    Returns a sub render state factory by index
    @note index must be lower than the value returned by getNumSubRenderStateFactories()
    */
    SubRenderStateFactory* getSubRenderStateFactory(size_t index);

    /** 
    Returns a sub render state factory by name
    */
    SubRenderStateFactory* getSubRenderStateFactory(const String& type);

    /** 
    Remove sub render state factory. 
    @param factory The factory to remove.
    */
    void removeSubRenderStateFactory(SubRenderStateFactory* factory);

    /** 
    Create an instance of sub render state from a given type. 
    @param type The type of sub render state to create.
    */
    SubRenderState* createSubRenderState(const String& type);

    
    /** 
    Destroy an instance of sub render state. 
    @param subRenderState The instance to destroy.
    */
    void destroySubRenderState(SubRenderState* subRenderState);


    /** 
    Checks if a shader based technique has been created for a given technique. 
    Return true if exist. False if not.
    @param materialName The source material name.
    @param srcTechniqueSchemeName The source technique scheme name.
    @param dstTechniqueSchemeName The destination shader based technique scheme name.
    */
    bool hasShaderBasedTechnique(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName) const;

    /**
     Checks if a shader based technique has been created for a given technique.
     Return true if exist. False if not.
     @param materialName The source material name.
     @param groupName The source group name.
     @param srcTechniqueSchemeName The source technique scheme name.
     @param dstTechniqueSchemeName The destination shader based technique scheme name.
     */
    bool hasShaderBasedTechnique(const String& materialName, const String& groupName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName) const;

    /** 
    Create shader based technique from a given technique. 
    Return true upon success. Failure may occur if the source technique is not FFP pure, or different
    source technique is mapped to the requested destination scheme.
    @param materialName The source material name.
    @param srcTechniqueSchemeName The source technique scheme name.
    @param dstTechniqueSchemeName The destination shader based technique scheme name.
    @param overProgrammable If true a shader will be created even if the material has shaders
    */
    bool createShaderBasedTechnique(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName, bool overProgrammable = false);

    /**
     Create shader based technique from a given technique.
     Return true upon success. Failure may occur if the source technique is not FFP pure, or different
     source technique is mapped to the requested destination scheme.
     @param materialName The source material name.
     @param groupName The source group name.
     @param srcTechniqueSchemeName The source technique scheme name.
     @param dstTechniqueSchemeName The destination shader based technique scheme name.
     @param overProgrammable If true a shader will be created even if the material has shaders
     */
    bool createShaderBasedTechnique(const String& materialName, const String& groupName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName, bool overProgrammable = false);


    /** 
    Remove shader based technique from a given technique. 
    Return true upon success. Failure may occur if the given source technique was not previously
    registered successfully using the createShaderBasedTechnique method.
    @param materialName The source material name.
    @param srcTechniqueSchemeName The source technique scheme name.
    @param dstTechniqueSchemeName The destination shader based technique scheme name.
    */
    bool removeShaderBasedTechnique(const String& materialName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName);

    /**
     Remove shader based technique from a given technique.
     Return true upon success. Failure may occur if the given source technique was not previously
     registered successfully using the createShaderBasedTechnique method.
     @param materialName The source material name.
     @param groupName The source group name.
     @param srcTechniqueSchemeName The source technique scheme name.
     @param dstTechniqueSchemeName The destination shader based technique scheme name.
     */
    bool removeShaderBasedTechnique(const String& materialName, const String& groupName, const String& srcTechniqueSchemeName, const String& dstTechniqueSchemeName);


    /** 
    Remove all shader based techniques of the given material. 
    Return true upon success.
    @param materialName The source material name.   
    @param groupName The source group name. 
    */
    bool removeAllShaderBasedTechniques(const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    /** 
    Clone all shader based techniques from one material to another.
    This function can be used in conjunction with the Material::clone() function to copy 
    both material properties and RTSS state from one material to another.
    @param srcMaterialName The source material name.    
    @param srcGroupName The source group name.  
    @param dstMaterialName The destination material name.   
    @param dstGroupName The destination group name. 
    @return True if successful
    */
    bool cloneShaderBasedTechniques(const String& srcMaterialName, 
        const String& srcGroupName, const String& dstMaterialName, const String& dstGroupName);

    /** 
    Remove all shader based techniques that created by this shader generator.   
    */
    void removeAllShaderBasedTechniques();

    /** 
    Create a scheme.
    @param schemeName The scheme name to create.
    */
    void createScheme(const String& schemeName);

    /** 
    Invalidate a given scheme. This action will lead to shader regeneration of all techniques belongs to the
    given scheme name.
    @param schemeName The scheme to invalidate.
    */
    void invalidateScheme(const String& schemeName);

    /** 
    Validate a given scheme. This action will generate shader programs for all techniques of the
    given scheme name.
    @param schemeName The scheme to validate.
    */
    bool validateScheme(const String& schemeName);
    
    /** 
    Invalidate specific material scheme. This action will lead to shader regeneration of the technique belongs to the
    given scheme name.
    @param schemeName The scheme to invalidate.
    @param materialName The material to invalidate.
    @param groupName The source group name. 
    */
    void invalidateMaterial(const String& schemeName, const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    /** 
    Validate specific material scheme. This action will generate shader programs for the technique of the
    given scheme name.
    @param schemeName The scheme to validate.
    @param materialName The material to validate.
    @param groupName The source group name. 
    */
    bool validateMaterial(const String& schemeName, const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);    

	/**
	Invalidate specific material scheme. This action will lead to shader regeneration of the technique belongs to the
	given scheme name.
	@param schemeName The scheme to invalidate.
	@param materialName The material to invalidate.
	@param groupName The source group name.
	*/
	void invalidateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

	/**
	Validate specific material scheme. This action will generate shader programs illumination passes of the technique of the
	given scheme name.
	@param schemeName The scheme to validate.
	@param materialName The material to validate.
	@param groupName The source group name.
	*/
	bool validateMaterialIlluminationPasses(const String& schemeName, const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

    /** 
    Return custom material Serializer of the shader generator.
    This is useful when you'd like to export certain material that contains shader generator effects.
    I.E - when writing an exporter you may want mark your material as shader generated material 
    so in the next time you will load it by your application it will automatically generate shaders with custom
    attributes you wanted. To do it you'll have to do the following steps:
    1. Create shader based technique for you material via the createShaderBasedTechnique() method.
    2. Create MaterialSerializer instance.
    3. Add the return instance of serializer listener to the MaterialSerializer.
    4. Call one of the export methods of MaterialSerializer.
    */
    SGMaterialSerializerListener* getMaterialSerializerListener();


    /** Return the current number of generated vertex shaders. */
    size_t getVertexShaderCount() const;


    /** Return the current number of generated fragment shaders. */
    size_t getFragmentShaderCount() const;



    /** Set the vertex shader outputs compaction policy. 
    @see VSOutputCompactPolicy.
    @param policy The policy to set.
    */
    void setVertexShaderOutputsCompactPolicy(VSOutputCompactPolicy policy)  { mVSOutputCompactPolicy = policy; }
    
    /** Get the vertex shader outputs compaction policy. 
    @see VSOutputCompactPolicy. 
    */
    VSOutputCompactPolicy getVertexShaderOutputsCompactPolicy() const { return mVSOutputCompactPolicy; }


    /** Sets whether shaders are created for passes with shaders.
    Note that this only refers to when the system parses the materials itself.
    Not for when calling the createShaderBasedTechnique() function directly
    @param value The value to set this attribute pass.  
    */
    void setCreateShaderOverProgrammablePass(bool value) { mCreateShaderOverProgrammablePass = value; }

    /** Returns whether shaders are created for passes with shaders.
    @see setCreateShaderOverProgrammablePass(). 
    */
    bool getCreateShaderOverProgrammablePass() const { return mCreateShaderOverProgrammablePass; }


    /** Returns the amount of schemes used in the for RT shader generation
    */
    size_t getRTShaderSchemeCount() const;

    /** Returns the scheme name used in the for RT shader generation by index
    */
    const String& getRTShaderScheme(size_t index) const;

    /// Default material scheme of the shader generator.
    static String DEFAULT_SCHEME_NAME;

// Protected types.
protected:
    class SGPass;
    class SGTechnique;
    class SGMaterial;
    class SGScheme;

    typedef std::pair<String,String>                MatGroupPair;
    struct MatGroupPair_less
    {
        // ensure we arrange the list first by material name then by group name
        bool operator()(const MatGroupPair& p1, const MatGroupPair& p2) const
        {
            int cmpVal = strcmp(p1.first.c_str(),p2.first.c_str());
            return (cmpVal < 0) || ((cmpVal == 0) && (strcmp(p1.second.c_str(),p2.second.c_str()) < 0));
        }
    };

    typedef vector<SGPass*>::type                   SGPassList;
    typedef SGPassList::iterator                    SGPassIterator;
    typedef SGPassList::const_iterator              SGPassConstIterator;

    typedef vector<SGTechnique*>::type              SGTechniqueList;
    typedef SGTechniqueList::iterator               SGTechniqueIterator;
    typedef SGTechniqueList::const_iterator         SGTechniqueConstIterator;

    typedef map<SGTechnique*, SGTechnique*>::type   SGTechniqueMap;
    typedef SGTechniqueMap::iterator                SGTechniqueMapIterator;
    
    typedef map<MatGroupPair, SGMaterial*, MatGroupPair_less>::type SGMaterialMap;
    typedef SGMaterialMap::iterator                 SGMaterialIterator;
    typedef SGMaterialMap::const_iterator           SGMaterialConstIterator;

    typedef map<String, SGScheme*>::type            SGSchemeMap;
    typedef SGSchemeMap::iterator                   SGSchemeIterator;
    typedef SGSchemeMap::const_iterator             SGSchemeConstIterator;

    typedef map<String, ScriptTranslator*>::type    SGScriptTranslatorMap;
    typedef SGScriptTranslatorMap::iterator         SGScriptTranslatorIterator;
    typedef SGScriptTranslatorMap::const_iterator   SGScriptTranslatorConstIterator;


    
    /** Shader generator pass wrapper class. */
    class _OgreRTSSExport SGPass : public RTShaderSystemAlloc
    {
    public:
		SGPass(SGTechnique* parent, Pass* srcPass, Pass* dstPass, IlluminationStage stage);
        ~SGPass();
    
        /** Build the render state. */
        void buildTargetRenderState();

        /** Acquire the CPU/GPU programs for this pass. */
        void acquirePrograms();

        /** Release the CPU/GPU programs of this pass. */
        void releasePrograms();


        /** Called when a single object is about to be rendered. */
        void notifyRenderSingleObject(Renderable* rend, const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

        /** Get source pass. */
        Pass* getSrcPass() { return mSrcPass; }

        /** Get destination pass. */
        Pass* getDstPass() { return mDstPass; }

		/** Get illumination stage. */
		IlluminationStage getIlluminationStage() { return mStage; }

		/** Get illumination state. */
		bool isIlluminationPass() { return mStage != IS_UNKNOWN; }

        /** Get custom FPP sub state of this pass. */
        SubRenderState* getCustomFFPSubState(int subStateOrder);

        /** Get custom render state of this pass. */
        RenderState* getCustomRenderState() { return mCustomRenderState; }

        /** Set the custom render state of this pass. */
        void setCustomRenderState(RenderState* customRenderState) { mCustomRenderState = customRenderState; }

        /// Key name for associating with a Pass instance.
        static String UserKey;
    
    protected:
        SubRenderState* getCustomFFPSubState(int subStateOrder, const RenderState* renderState);

    protected:
        // Parent technique.
        SGTechnique* mParent;
        // Source pass.
        Pass* mSrcPass;
        // Destination pass.
        Pass* mDstPass;
		// Illumination stage
		IlluminationStage mStage;
        // Custom render state.
        RenderState* mCustomRenderState;
        // The compiled render state.
        TargetRenderState* mTargetRenderState;
    };

    
    /** Shader generator technique wrapper class. */
    class _OgreRTSSExport SGTechnique : public RTShaderSystemAlloc
    {
    public:
        SGTechnique(SGMaterial* parent, Technique* srcTechnique, const String& dstTechniqueSchemeName);     
        ~SGTechnique();
        
        /** Get the parent SGMaterial */
        const SGMaterial* getParent() const { return mParent; }
        
        /** Get the source technique. */
        Technique* getSourceTechnique() { return mSrcTechnique; }

        /** Get the destination technique. */
        Technique* getDestinationTechnique() { return mDstTechnique; }

        /** Get the destination technique scheme name. */
        const String& getDestinationTechniqueSchemeName() const { return mDstTechniqueSchemeName; }
        
        /** Build the render state. */
        void buildTargetRenderState();

        /** Acquire the CPU/GPU programs for this technique. */
        void acquirePrograms();

		/** Build the render state for illumination passes. */
		void buildIlluminationTargetRenderState();

		/** Acquire the CPU/GPU programs for illumination passes of this technique. */
		void acquireIlluminationPrograms();

		/** Destroy the illumination passes entries. */
		void destroyIlluminationSGPasses();

        /** Release the CPU/GPU programs of this technique. */
        void releasePrograms();

        /** Tells the technique that it needs to generate shader code. */
        void setBuildDestinationTechnique(bool buildTechnique)  { mBuildDstTechnique = buildTechnique; }        

        /** Tells if the destination technique should be build. */
        bool getBuildDestinationTechnique() const               { return mBuildDstTechnique; }

        /** Get render state of specific pass.
        @param passIndex The pass index.
        */
        RenderState* getRenderState(unsigned short passIndex);
        /** Tells if a custom render state exists for the given pass. */
        bool hasRenderState(unsigned short passIndex);

        // Key name for associating with a Technique instance.
        static String UserKey;

    protected:
        
        /** Create the passes entries. */
        void createSGPasses();

		/** Create the illumination passes entries. */
		void createIlluminationSGPasses();

        /** Destroy the passes entries. */
        void destroySGPasses();
        
    protected:
        // Auto mutex.
        OGRE_AUTO_MUTEX;
        // Parent material.     
        SGMaterial* mParent;
        // Source technique.
        Technique* mSrcTechnique;
        // Destination technique.
        Technique* mDstTechnique;
		// All passes entries, both normal and illumination.
        SGPassList mPassEntries;
        // The custom render states of all passes.
        RenderStateList mCustomRenderStates;
        // Flag that tells if destination technique should be build.        
        bool mBuildDstTechnique;
        // Scheme name of destination technique.
        String mDstTechniqueSchemeName;
    };

    
    /** Shader generator material wrapper class. */
    class _OgreRTSSExport SGMaterial : public RTShaderSystemAlloc
    {   
    
    public:
        /** Class constructor. */
        SGMaterial(const String& materialName, const String& groupName) : mName(materialName), mGroup(groupName) 
        {

        }

        /** Get the material name. */
        const String& getMaterialName() const   { return mName; }
        
        /** Get the group name. */
        const String& getGroupName() const  { return mGroup; }

        /** Get the const techniques list of this material. */
        const SGTechniqueList& getTechniqueList() const  { return mTechniqueEntries; }

        /** Get the techniques list of this material. */
        SGTechniqueList& getTechniqueList()              { return mTechniqueEntries; }
    
    protected:
        // The material name.
        String mName;
        // The group name.
        String mGroup;
        // All passes entries.
        SGTechniqueList mTechniqueEntries;
    };

    
    /** Shader generator scheme class. */
    class _OgreRTSSExport SGScheme : public RTShaderSystemAlloc
    {   
    public:
        SGScheme(const String& schemeName);
        ~SGScheme();    


        /** Return true if this scheme dose not contains any techniques.
        */
        bool empty() const  { return mTechniqueEntries.empty(); }
        
        /** Invalidate the whole scheme.
        @see ShaderGenerator::invalidateScheme.
        */
        void invalidate();

        /** Validate the whole scheme.
        @see ShaderGenerator::validateScheme.
        */
        void validate();

        /** Invalidate specific material.
        @see ShaderGenerator::invalidateMaterial.
        */
        void invalidate(const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        /** Validate specific material.
        @see ShaderGenerator::validateMaterial.
        */
        bool validate(const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		/** Validate illumination passes of the specific material.
		@see ShaderGenerator::invalidateMaterialIlluminationPasses.
		*/
		void invalidateIlluminationPasses(const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

		/** Validate illumination passes of the specific material.
		@see ShaderGenerator::validateMaterialIlluminationPasses.
		*/
		bool validateIlluminationPasses(const String& materialName, const String& groupName = ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME);

        /** Add a technique to current techniques list. */
        void addTechniqueEntry(SGTechnique* techEntry);

        /** Remove a technique from the current techniques list. */
        void removeTechniqueEntry(SGTechnique* techEntry);


        /** Get global render state of this scheme. 
        @see ShaderGenerator::getRenderState.
        */
        RenderState* getRenderState();

        /** Get specific pass render state. 
        @see ShaderGenerator::getRenderState.
        */
        RenderState* getRenderState(const String& materialName, const String& groupName, unsigned short passIndex);

    protected:
        /** Synchronize the current light settings of this scheme with the current settings of the scene. */
        void synchronizeWithLightSettings();

        /** Synchronize the fog settings of this scheme with the current settings of the scene. */
        void synchronizeWithFogSettings();


    protected:
        // Scheme name.
        String mName;
        // Technique entries.
        SGTechniqueList mTechniqueEntries;
        // Tells if this scheme is out of date.
        bool mOutOfDate;
        // The global render state of this scheme.
        RenderState* mRenderState;
        // Current fog mode.
        FogMode mFogMode;
    };


// Protected types.
protected:
    
    /** Shader generator RenderObjectListener sub class. */
    class _OgreRTSSExport SGRenderObjectListener : public RenderObjectListener, public RTShaderSystemAlloc
    {
    public:
        SGRenderObjectListener(ShaderGenerator* owner)
        {
            mOwner = owner;
        }

        /** 
        Listener overridden function notify the shader generator when rendering single object.
        */
        virtual void notifyRenderSingleObject(Renderable* rend, const Pass* pass,  
            const AutoParamDataSource* source, 
            const LightList* pLightList, bool suppressRenderStateChanges)
        {
            mOwner->notifyRenderSingleObject(rend, pass, source, pLightList, suppressRenderStateChanges);
        }

    protected:
        ShaderGenerator* mOwner;
    };

    /** Shader generator scene manager sub class. */
    class _OgreRTSSExport SGSceneManagerListener : public SceneManager::Listener, public RTShaderSystemAlloc
    {
    public:
        SGSceneManagerListener(ShaderGenerator* owner)
        {
            mOwner = owner;
        }

        /** 
        Listener overridden function notify the shader generator when finding visible objects process started.
        */
        virtual void preFindVisibleObjects(SceneManager* source, 
            SceneManager::IlluminationRenderStage irs, Viewport* v)
        {
            mOwner->preFindVisibleObjects(source, irs, v);
        }

        virtual void postFindVisibleObjects(SceneManager* source, 
            SceneManager::IlluminationRenderStage irs, Viewport* v)
        {

        }

        virtual void shadowTexturesUpdated(size_t numberOfShadowTextures) 
        {

        }

        virtual void shadowTextureCasterPreViewProj(Light* light, 
            Camera* camera, size_t iteration) 
        {

        }

        virtual void shadowTextureReceiverPreViewProj(Light* light, 
            Frustum* frustum)
        {

        }

    protected:
        // The shader generator instance.
        ShaderGenerator* mOwner;
    };

    /** Shader generator ScriptTranslatorManager sub class. */
    class _OgreRTSSExport SGScriptTranslatorManager : public ScriptTranslatorManager
    {
    public:
        SGScriptTranslatorManager(ShaderGenerator* owner)
        {
            mOwner = owner;
        }

        /// Returns the number of translators being managed
        virtual size_t getNumTranslators() const
        {
            return mOwner->getNumTranslators();
        }
        
        /// Returns a manager for the given object abstract node, or null if it is not supported
        virtual ScriptTranslator *getTranslator(const AbstractNodePtr& node)
        {
            return mOwner->getTranslator(node);
        }

    protected:
        // The shader generator instance.
        ShaderGenerator* mOwner;
    };

    //-----------------------------------------------------------------------------
    typedef map<String, SubRenderStateFactory*>::type       SubRenderStateFactoryMap;
    typedef SubRenderStateFactoryMap::iterator              SubRenderStateFactoryIterator;
    typedef SubRenderStateFactoryMap::const_iterator        SubRenderStateFactoryConstIterator;

    //-----------------------------------------------------------------------------
    typedef map<String, SceneManager*>::type                SceneManagerMap;
    typedef SceneManagerMap::iterator                       SceneManagerIterator;
    typedef SceneManagerMap::const_iterator                 SceneManagerConstIterator;

protected:
    /** Class default constructor */
    ShaderGenerator();

    /** Class destructor */
    ~ShaderGenerator();

    /** Initialize the shader generator instance. */
    bool _initialize();
    
    /** Destory the shader generator instance. */
    void _destroy();

    /** Find source technique to generate shader based technique based on it. */
    Technique* findSourceTechnique(const String& materialName, const String& groupName, const String& srcTechniqueSchemeName, bool allowProgrammable);

    /** Checks if a given technique has passes with shaders. */
    bool isProgrammable(Technique* tech) const;
 
    /** Called from the sub class of the RenderObjectLister when single object is rendered. */
    void notifyRenderSingleObject(Renderable* rend, const Pass* pass,  const AutoParamDataSource* source, const LightList* pLightList, bool suppressRenderStateChanges);

    /** Called from the sub class of the SceneManager::Listener when finding visible object process starts. */
    void preFindVisibleObjects(SceneManager* source, SceneManager::IlluminationRenderStage irs, Viewport* v);

    /** Create sub render state core extensions factories */
    void createSubRenderStateExFactories();

    /** Destroy sub render state core extensions factories */
    void destroySubRenderStateExFactories();

    /** Create an instance of the SubRenderState based on script properties using the
    current sub render state factories.
    @see SubRenderStateFactory::createInstance  
    @param compiler The compiler instance.
    @param prop The abstract property node.
    @param pass The pass that is the parent context of this node.
    @param translator The translator for the specific SubRenderState
    */
    SubRenderState* createSubRenderState(ScriptCompiler* compiler, PropertyAbstractNode* prop, Pass* pass, SGScriptTranslator* translator);
    
    /** Create an instance of the SubRenderState based on script properties using the
    current sub render state factories.
    @see SubRenderStateFactory::createInstance  
    @param compiler The compiler instance.
    @param prop The abstract property node.
    @param texState The texture unit state that is the parent context of this node.
    @param translator The translator for the specific SubRenderState
    */
    SubRenderState* createSubRenderState(ScriptCompiler* compiler, PropertyAbstractNode* prop, TextureUnitState* texState, SGScriptTranslator* translator);

    /** 
    Add custom script translator. 
    Return true upon success.
    @param key The key name of the given translator.
    @param translator The translator to associate with the given key.
    */
    bool addCustomScriptTranslator(const String& key, ScriptTranslator* translator);

    /** 
    Remove custom script translator. 
    Return true upon success.
    @param key The key name of the translator to remove.    
    */
    bool removeCustomScriptTranslator(const String& key);

    /** Return number of script translators. */
    size_t getNumTranslators() const;

    /** Return a matching script translator. */
    ScriptTranslator* getTranslator(const AbstractNodePtr& node);


    /** This method called by instance of SGMaterialSerializerListener and 
    serialize a given pass entry attributes.
    @param ser The material serializer.
    @param passEntry The SGPass instance.
    */
    void serializePassAttributes(MaterialSerializer* ser, SGPass* passEntry);

    /** This method called by instance of SGMaterialSerializerListener and 
    serialize a given textureUnitState entry attributes.
    @param ser The material serializer.
    @param passEntry The SGPass instance.
    @param srcTextureUnit The TextureUnitState being serialized.
    */
    void serializeTextureUnitStateAttributes(MaterialSerializer* ser, SGPass* passEntry, const TextureUnitState* srcTextureUnit);

    /** Finds an entry iterator in the mMaterialEntriesMap map.
    This function is able to find materials with group specified as 
    AUTODETECT_RESOURCE_GROUP_NAME 
    */
    SGMaterialIterator findMaterialEntryIt(const String& materialName, const String& groupName);
    SGMaterialConstIterator findMaterialEntryIt(const String& materialName, const String& groupName) const;


    typedef std::pair<SGScheme*, bool> SchemeCreateOrRetrieveResult;
    /** 
    Returns a requested scheme. If the scheme does not exist this function creates it.
    @param schemeName The scheme name to retrieve.
    */
    SchemeCreateOrRetrieveResult createOrRetrieveScheme(const String& schemeName);

    /** Used to check if finalizing */
    bool getIsFinalizing() const;
protected:  
    // Auto mutex.
    OGRE_AUTO_MUTEX;
    // The active scene manager.
    SceneManager* mActiveSceneMgr;
    // A map of all scene managers this generator is bound to.
    SceneManagerMap mSceneManagerMap;
    // Render object listener.
    SGRenderObjectListener* mRenderObjectListener;
    // Scene manager listener.
    SGSceneManagerListener* mSceneManagerListener;
    // Script translator manager.
    SGScriptTranslatorManager* mScriptTranslatorManager;
    // Custom material Serializer listener - allows exporting material that contains shader generated techniques.
    SGMaterialSerializerListener* mMaterialSerializerListener;
    // A map of the registered custom script translators.
    SGScriptTranslatorMap mScriptTranslatorsMap;
    // The core translator of the RT Shader System.
    SGScriptTranslator mCoreScriptTranslator;
    // The target shader language (currently only cg supported).
    String mShaderLanguage;
    // The target shader language version.
    float  mShaderLanguageVersion;
    // The target vertex shader profile. Will be used as argument for program compilation.
    String mVertexShaderProfiles;
    // List of target vertex shader profiles.
    StringVector mVertexShaderProfilesList;
    // The target fragment shader profile. Will be used as argument for program compilation.
    String mFragmentShaderProfiles;
    // List of target fragment shader profiles..
    StringVector mFragmentShaderProfilesList;
    // Path for caching the generated shaders.
    String mShaderCachePath;
    // Shader program manager.
    ProgramManager* mProgramManager;
    // Shader program writer manager.
    ProgramWriterManager* mProgramWriterManager;
    // File system layer manager.
    FileSystemLayer* mFSLayer;
    // Fixed Function Render state builder.
    FFPRenderStateBuilder* mFFPRenderStateBuilder;
    // Material entries map.
    SGMaterialMap mMaterialEntriesMap;
    // Scheme entries map.
    SGSchemeMap mSchemeEntriesMap;
    // All technique entries map.
    SGTechniqueMap mTechniqueEntriesMap;
    // Sub render state registered factories.
    SubRenderStateFactoryMap mSubRenderStateFactories;
    // Sub render state core extension factories.
    SubRenderStateFactoryMap mSubRenderStateExFactories;
    // True if active view port use a valid SGScheme.
    bool mActiveViewportValid;
    // Light count per light type.
    int mLightCount[3];
    // Vertex shader outputs compact policy.
    VSOutputCompactPolicy mVSOutputCompactPolicy;
    // Tells whether shaders are created for passes with shaders
    bool mCreateShaderOverProgrammablePass;
    // A flag to indicate finalizing
    bool mIsFinalizing;
private:
    friend class SGPass;
    friend class FFPRenderStateBuilder;
    friend class SGScriptTranslatorManager;
    friend class SGScriptTranslator;
    friend class SGMaterialSerializerListener;
    
};

/** @} */
/** @} */

}
}

#endif

