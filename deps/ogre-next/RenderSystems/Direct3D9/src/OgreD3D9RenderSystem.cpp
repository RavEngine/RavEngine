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
#include "OgreD3D9RenderSystem.h"
#include "OgreD3D9Prerequisites.h"
#include "OgreD3D9DriverList.h"
#include "OgreD3D9Driver.h"
#include "OgreD3D9VideoModeList.h"
#include "OgreD3D9VideoMode.h"
#include "OgreD3D9RenderWindow.h"
#include "OgreD3D9TextureManager.h"
#include "OgreD3D9Texture.h"
#include "OgreLogManager.h"
#include "OgreLight.h"
#include "OgreMath.h"
#include "OgreViewport.h"
#include "OgreD3D9HardwareBufferManager.h"
#include "OgreD3D9HardwareIndexBuffer.h"
#include "OgreD3D9HardwareVertexBuffer.h"
#include "OgreD3D9VertexDeclaration.h"
#include "OgreD3D9GpuProgram.h"
#include "OgreD3D9GpuProgramManager.h"
#include "OgreD3D9HLSLProgramFactory.h"
#include "OgreHighLevelGpuProgramManager.h"
#include "OgreD3D9HardwareOcclusionQuery.h"
#include "OgreFrustum.h"
#include "OgreD3D9MultiRenderTarget.h"
#include "OgreD3D9DeviceManager.h"
#include "OgreD3D9ResourceManager.h"
#include "OgreD3D9DepthBuffer.h"
#include "OgreRenderOperation.h"
#include "OgreHlmsDatablock.h"

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
#include "OgreD3D9StereoDriverBridge.h"
#endif

#define FLOAT2DWORD(f) *((DWORD*)&f)

namespace Ogre 
{
    D3D9RenderSystem* D3D9RenderSystem::msD3D9RenderSystem = NULL;

    //---------------------------------------------------------------------
    D3D9RenderSystem::D3D9RenderSystem( HINSTANCE hInstance ) :
        mMultiheadUse(mutAuto)
        ,mAllowDirectX9Ex(false)
        ,mIsDirectX9Ex(false)
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        ,mStereoDriver (NULL)
#endif
    {
        LogManager::getSingleton().logMessage( "D3D9 : " + getName() + " created." );

        // update singleton access pointer.
        msD3D9RenderSystem = this;

        // set the instance being passed 
        mhInstance = hInstance;

        // set pointers to NULL
        mD3D = NULL;        
        mDriverList = NULL;
        mActiveD3DDriver = NULL;
        mTextureManager = NULL;
        mHardwareBufferManager = NULL;
        mGpuProgramManager = NULL;          
        mUseNVPerfHUD = false;
        mHLSLProgramFactory = NULL;     
        mDeviceManager = NULL;  
        mPerStageConstantSupport = false;

		for(int i = 0 ; i < OGRE_MAX_TEXTURE_LAYERS ; i++)
		{
			for(int j = 0 ; j < 2 ; j++)
			{
				mManualBlendColours[i][j] = ColourValue::ZERO;
			}

		}

        // Create the resource manager.
        mResourceManager = OGRE_NEW D3D9ResourceManager();

        
        // init lights
        for(int i = 0; i < MAX_LIGHTS; i++ )
            mLights[i] = 0;

        // Create our Direct3D object
        if( NULL == (mD3D = Direct3DCreate9(D3D_SDK_VERSION)) )
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Failed to create Direct3D9 object", "D3D9RenderSystem::D3D9RenderSystem" );

        // set config options defaults
        initConfigOptions();

        // fsaa options
        mFSAAHint = "";
        mFSAASamples = 0;
        
        // set stages desc. to defaults
        for (size_t n = 0; n < OGRE_MAX_TEXTURE_LAYERS; n++)
        {
            mTexStageDesc[n].autoTexCoordType = TEXCALC_NONE;
            mTexStageDesc[n].coordIndex = 0;
            mTexStageDesc[n].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
            mTexStageDesc[n].pTex = 0;
            mTexStageDesc[n].pVertexTex = 0;
        }

        mLastVertexSourceCount = 0;
        
        mCurrentLights.clear();

        // Enumerate events
        mEventNames.push_back("DeviceLost");
        mEventNames.push_back("DeviceRestored");            
    }
    //---------------------------------------------------------------------
    D3D9RenderSystem::~D3D9RenderSystem()
    {       
        shutdown();

        // Deleting the HLSL program factory
        if (mHLSLProgramFactory)
        {
            // Remove from manager safely
            if (HighLevelGpuProgramManager::getSingletonPtr())
                HighLevelGpuProgramManager::getSingleton().removeFactory(mHLSLProgramFactory);
            OGRE_DELETE mHLSLProgramFactory;
            mHLSLProgramFactory = 0;
        }
        
        SAFE_RELEASE( mD3D );
        
        if (mResourceManager != NULL)
        {
            OGRE_DELETE mResourceManager;
            mResourceManager = NULL;
        }
        
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        OGRE_DELETE mStereoDriver;
        mStereoDriver = NULL;
#endif

        LogManager::getSingleton().logMessage( "D3D9 : " + getName() + " destroyed." );

        msD3D9RenderSystem = NULL;
    }
    //---------------------------------------------------------------------
    const String& D3D9RenderSystem::getName() const
    {
        static String strName( "Direct3D9 Rendering Subsystem");
        return strName;
    }
    //---------------------------------------------------------------------
	const String& D3D9RenderSystem::getFriendlyName(void) const
	{
		static String strName = mIsDirectX9Ex ? "Direct3D 9Ex" : "Direct3D 9";
		return strName;
	}
	
    D3D9DriverList* D3D9RenderSystem::getDirect3DDrivers()
    {
        if( !mDriverList )
            mDriverList = OGRE_NEW D3D9DriverList();

        return mDriverList;
    }
    //---------------------------------------------------------------------
    bool D3D9RenderSystem::_checkMultiSampleQuality(D3DMULTISAMPLE_TYPE type, DWORD *outQuality, D3DFORMAT format, UINT adapterNum, D3DDEVTYPE deviceType, BOOL fullScreen)
    {
        HRESULT hr;
        hr = mD3D->CheckDeviceMultiSampleType( 
            adapterNum, 
            deviceType, 
            format, 
            fullScreen, 
            type, 
            outQuality);

        if (SUCCEEDED(hr))
            return true;
        else
            return false;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::initConfigOptions()
    {
        D3D9DriverList* driverList;
        D3D9Driver* driver;

        ConfigOption optDevice;
        ConfigOption optAllowDirectX9Ex;
        ConfigOption optVideoMode;
        ConfigOption optFullScreen;
        ConfigOption optMultihead;
        ConfigOption optVSync;
        ConfigOption optVSyncInterval;
		ConfigOption optBackBufferCount;
        ConfigOption optAA;
        ConfigOption optFPUMode;
        ConfigOption optNVPerfHUD;
        ConfigOption optSRGB;
        ConfigOption optResourceCeationPolicy;
        ConfigOption optMultiDeviceMemHint;
        ConfigOption optEnableFixedPipeline;
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        ConfigOption optStereoMode;
#endif

        driverList = this->getDirect3DDrivers();

        optDevice.name = "Rendering Device";
        optDevice.currentValue.clear();
        optDevice.possibleValues.clear();
        optDevice.immutable = false;

        optAllowDirectX9Ex.name = "Allow DirectX9Ex";
        optAllowDirectX9Ex.possibleValues.push_back( "Yes" );
        optAllowDirectX9Ex.possibleValues.push_back( "No" );
        optAllowDirectX9Ex.currentValue = "No";
        optAllowDirectX9Ex.immutable = false;

        optVideoMode.name = "Video Mode";
        optVideoMode.currentValue = "800 x 600 @ 32-bit colour";
        optVideoMode.immutable = false;

        optFullScreen.name = "Full Screen";
        optFullScreen.possibleValues.push_back( "Yes" );
        optFullScreen.possibleValues.push_back( "No" );
        optFullScreen.currentValue = "Yes";
        optFullScreen.immutable = false;

        optMultihead.name = "Use Multihead";
        optMultihead.possibleValues.push_back( "Auto" );
        optMultihead.possibleValues.push_back( "Yes" );
        optMultihead.possibleValues.push_back( "No" );
        optMultihead.currentValue = "Auto";
        optMultihead.immutable = false;

        optResourceCeationPolicy.name = "Resource Creation Policy";     
        optResourceCeationPolicy.possibleValues.push_back( "Create on all devices" );
        optResourceCeationPolicy.possibleValues.push_back( "Create on active device" );

        if (mResourceManager->getCreationPolicy() == RCP_CREATE_ON_ACTIVE_DEVICE)
            optResourceCeationPolicy.currentValue = "Create on active device";          
        else if (mResourceManager->getCreationPolicy() == RCP_CREATE_ON_ALL_DEVICES)
            optResourceCeationPolicy.currentValue = "Create on all devices";
        else
            optResourceCeationPolicy.currentValue = "N/A";
        optResourceCeationPolicy.immutable = false;

        for( unsigned j=0; j < driverList->count(); j++ )
        {
            driver = driverList->item(j);
            optDevice.possibleValues.push_back( driver->DriverDescription() );
            // Make first one default
            if( j==0 )
                optDevice.currentValue = driver->DriverDescription();
        }

        optVSync.name = "VSync";
        optVSync.immutable = false;
        optVSync.possibleValues.push_back( "Yes" );
        optVSync.possibleValues.push_back( "No" );
        optVSync.currentValue = "No";

        optVSyncInterval.name = "VSync Interval";
        optVSyncInterval.immutable = false;
        optVSyncInterval.possibleValues.push_back( "1" );
        optVSyncInterval.possibleValues.push_back( "2" );
        optVSyncInterval.possibleValues.push_back( "3" );
        optVSyncInterval.possibleValues.push_back( "4" );
        optVSyncInterval.currentValue = "1";

		optBackBufferCount.name = "Backbuffer Count";
		optBackBufferCount.immutable = false;
		optBackBufferCount.possibleValues.push_back( "Auto" );
		optBackBufferCount.possibleValues.push_back( "1" );
		optBackBufferCount.possibleValues.push_back( "2" );
		optBackBufferCount.currentValue = "Auto";

        optAA.name = "FSAA";
        optAA.immutable = false;
        optAA.possibleValues.push_back( "None" );
        optAA.currentValue = "None";

        optFPUMode.name = "Floating-point mode";
#if OGRE_DOUBLE_PRECISION
        optFPUMode.currentValue = "Consistent";
#else
        optFPUMode.currentValue = "Fastest";
#endif
        optFPUMode.possibleValues.clear();
        optFPUMode.possibleValues.push_back("Fastest");
        optFPUMode.possibleValues.push_back("Consistent");
        optFPUMode.immutable = false;

        optNVPerfHUD.currentValue = "No";
        optNVPerfHUD.immutable = false;
        optNVPerfHUD.name = "Allow NVPerfHUD";
        optNVPerfHUD.possibleValues.push_back( "Yes" );
        optNVPerfHUD.possibleValues.push_back( "No" );


        // SRGB on auto window
        optSRGB.name = "sRGB Gamma Conversion";
        optSRGB.possibleValues.push_back("Yes");
        optSRGB.possibleValues.push_back("No");
        optSRGB.currentValue = "No";
        optSRGB.immutable = false;

        // Multiple device memory usage hint.
        optMultiDeviceMemHint.name = "Multi device memory hint";
        optMultiDeviceMemHint.possibleValues.push_back("Use minimum system memory");
        optMultiDeviceMemHint.possibleValues.push_back("Auto hardware buffers management");
        optMultiDeviceMemHint.currentValue = "Use minimum system memory";
        optMultiDeviceMemHint.immutable = false;

        optEnableFixedPipeline.name = "Fixed Pipeline Enabled";
        optEnableFixedPipeline.possibleValues.push_back( "Yes" );
        optEnableFixedPipeline.possibleValues.push_back( "No" );
        optEnableFixedPipeline.currentValue = "Yes";
        optEnableFixedPipeline.immutable = false;

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        optStereoMode.name = "Stereo Mode";
        optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_NONE));
        optStereoMode.possibleValues.push_back(StringConverter::toString(SMT_FRAME_SEQUENTIAL));
        optStereoMode.currentValue = optStereoMode.possibleValues[0];
        optStereoMode.immutable = false;

        mOptions[optStereoMode.name] = optStereoMode;
#endif

        mOptions[optDevice.name] = optDevice;
        mOptions[optAllowDirectX9Ex.name] = optAllowDirectX9Ex;
        mOptions[optVideoMode.name] = optVideoMode;
        mOptions[optFullScreen.name] = optFullScreen;
        mOptions[optMultihead.name] = optMultihead;
        mOptions[optVSync.name] = optVSync;
        mOptions[optVSyncInterval.name] = optVSyncInterval;
		mOptions[optBackBufferCount.name] = optBackBufferCount;
        mOptions[optAA.name] = optAA;
        mOptions[optFPUMode.name] = optFPUMode;
        mOptions[optNVPerfHUD.name] = optNVPerfHUD;
        mOptions[optSRGB.name] = optSRGB;
        mOptions[optResourceCeationPolicy.name] = optResourceCeationPolicy;
        mOptions[optMultiDeviceMemHint.name] = optMultiDeviceMemHint;
        mOptions[optEnableFixedPipeline.name] = optEnableFixedPipeline;

        refreshD3DSettings();

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::refreshD3DSettings()
    {
        ConfigOption* optVideoMode;
        D3D9Driver* driver = 0;
        D3D9VideoMode* videoMode;

        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        if( opt != mOptions.end() )
        {
            for( unsigned j=0; j < getDirect3DDrivers()->count(); j++ )
            {
                D3D9Driver* curDriver = getDirect3DDrivers()->item(j);
                if( curDriver->DriverDescription() == opt->second.currentValue )
                {
                    driver = curDriver;
                    break;
                }
            }

            if (driver)
            {
                opt = mOptions.find( "Video Mode" );
                optVideoMode = &opt->second;
                optVideoMode->possibleValues.clear();
                // get vide modes for this device
                for( unsigned k=0; k < driver->getVideoModeList()->count(); k++ )
                {
                    videoMode = driver->getVideoModeList()->item( k );
                    optVideoMode->possibleValues.push_back( videoMode->getDescription() );
                }

                // Reset video mode to default if previous doesn't avail in new possible values
                StringVector::const_iterator itValue =
                    std::find(optVideoMode->possibleValues.begin(),
                    optVideoMode->possibleValues.end(),
                    optVideoMode->currentValue);
                if (itValue == optVideoMode->possibleValues.end())
                {
                    optVideoMode->currentValue = "800 x 600 @ 32-bit colour";
                }

                // Also refresh FSAA options
                refreshFSAAOptions();
            }
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setConfigOption( const String &name, const String &value )
    {

        LogManager::getSingleton().stream()
            << "D3D9 : RenderSystem Option: " << name << " = " << value;

        bool viewModeChanged = false;

        // Find option
        ConfigOptionMap::iterator it = mOptions.find( name );

        // Update
        if( it != mOptions.end() )
            it->second.currentValue = value;
        else
        {
            StringStream str;
            str << "Option named '" << name << "' does not exist.";
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, str.str(), "D3D9RenderSystem::setConfigOption" );
        }

        // Refresh other options if D3DDriver changed
        if( name == "Rendering Device" )
            refreshD3DSettings();

        if ( name == "Allow DirectX9Ex" )
        {
            if (value == "Yes")
                mAllowDirectX9Ex = true;
            else mAllowDirectX9Ex = false;

            // Create our Direct3D object
            if (mAllowDirectX9Ex && !mIsDirectX9Ex)
            {
                SAFE_RELEASE(mD3D);
                HMODULE hD3D = LoadLibrary(TEXT("d3d9.dll"));
                if (hD3D)
                {
                    typedef HRESULT (WINAPI *DIRECT3DCREATE9EXFUNCTION)(UINT, IDirect3D9Ex**);
                    DIRECT3DCREATE9EXFUNCTION pfnCreate9Ex = (DIRECT3DCREATE9EXFUNCTION)GetProcAddress(hD3D, "Direct3DCreate9Ex");
                    if (pfnCreate9Ex)
                    {
                        IDirect3D9Ex* d3dEx = NULL;
                        (*pfnCreate9Ex)(D3D_SDK_VERSION, &d3dEx);
                        d3dEx->QueryInterface(__uuidof(IDirect3D9), reinterpret_cast<void **>(&mD3D));
                        mIsDirectX9Ex = true;
                    }
                    FreeLibrary(hD3D);
                }
            }
            if ((mD3D == NULL) || (!mAllowDirectX9Ex && mIsDirectX9Ex))
            {
                if ( NULL == (mD3D = Direct3DCreate9(D3D_SDK_VERSION)) )
                    OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Failed to create Direct3D9 object", "D3D9RenderSystem::D3D9RenderSystem" );
            }
        }

        if( name == "Full Screen" )
        {
            // Video mode is applicable
            it = mOptions.find( "Video Mode" );
            if (it->second.currentValue.empty())
            {
                it->second.currentValue = "800 x 600 @ 32-bit colour";
                viewModeChanged = true;
            }
        }

        if( name == "Use Multihead" )
        {
            if (value == "Yes")
                mMultiheadUse = mutYes;
            else if (value == "No")
                mMultiheadUse = mutNo;
            else mMultiheadUse = mutAuto;
        }

		if (name == "VSync Interval")
		{
			mVSyncInterval = StringConverter::parseUnsignedInt(value);
		}

		if( name == "VSync" )
		{
			if (value == "Yes")
				mVSync = true;
			else
				mVSync = false;
		}
		
        if( name == "FSAA" )
        {
            StringVector values = StringUtil::split(value, " ", 1);
            mFSAASamples = StringConverter::parseUnsignedInt(values[0]);
            if (values.size() > 1)
                mFSAAHint = values[1];

        }
		
		if (name == "Backbuffer Count")
		{
			if (value == "Auto")
			{
				mBackBufferCount = -1;
			}
			else
			{
				mBackBufferCount = StringConverter::parseUnsignedInt(value);
			}
		}

        if( name == "Allow NVPerfHUD" )
        {
            if (value == "Yes")
                mUseNVPerfHUD = true;
            else
                mUseNVPerfHUD = false;
        }

        if (viewModeChanged || name == "Video Mode")
        {
            refreshFSAAOptions();
        }

        if (name == "Resource Creation Policy")
        {
            if (value == "Create on active device")
                mResourceManager->setCreationPolicy(RCP_CREATE_ON_ACTIVE_DEVICE);
            else if (value == "Create on all devices")
                mResourceManager->setCreationPolicy(RCP_CREATE_ON_ALL_DEVICES);     
        }

        if (name == "Multi device memory hint")
        {
            if (value == "Use minimum system memory")
                mResourceManager->setAutoHardwareBufferManagement(false);
            else if (value == "Auto hardware buffers management")
                mResourceManager->setAutoHardwareBufferManagement(true);
        }       

        if (name == "Fixed Pipeline Enabled")
        {
            if (value == "Yes")
            {
                mEnableFixedPipeline = true;
            }
            else
                mEnableFixedPipeline = false;
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::refreshFSAAOptions()
    {

        ConfigOptionMap::iterator it = mOptions.find( "FSAA" );
        ConfigOption* optFSAA = &it->second;
        optFSAA->possibleValues.clear();
        optFSAA->possibleValues.push_back("0");

        it = mOptions.find("Rendering Device");
        D3D9Driver *driver = getDirect3DDrivers()->item(it->second.currentValue);
        if (driver)
        {
            it = mOptions.find("Video Mode");
            D3D9VideoMode *videoMode = driver->getVideoModeList()->item(it->second.currentValue);
            if (videoMode)
            {
                DWORD numLevels = 0;
                bool bOK;

                for (unsigned int n = (unsigned int)D3DMULTISAMPLE_2_SAMPLES; n <= (unsigned int)D3DMULTISAMPLE_16_SAMPLES; n++)
                {
                    bOK = this->_checkMultiSampleQuality(
                        (D3DMULTISAMPLE_TYPE)n, 
                        &numLevels, 
                        videoMode->getFormat(), 
                        driver->getAdapterNumber(),
                        D3DDEVTYPE_HAL,
                        TRUE);
                    if (bOK)
                    {
                        optFSAA->possibleValues.push_back(StringConverter::toString(n));
                        if (n >= 8)
                            optFSAA->possibleValues.push_back(StringConverter::toString(n) + " [Quality]");
                    }
                }

            }
        }

        // Reset FSAA to none if previous doesn't avail in new possible values
        StringVector::const_iterator itValue =
            std::find(optFSAA->possibleValues.begin(),
            optFSAA->possibleValues.end(),
            optFSAA->currentValue);
        if (itValue == optFSAA->possibleValues.end())
        {
            optFSAA->currentValue = "0";
        }

    }
    //---------------------------------------------------------------------
    String D3D9RenderSystem::validateConfigOptions()
    {
        ConfigOptionMap::iterator it;

        // check if video mode is selected
        it = mOptions.find( "Video Mode" );
        if (it->second.currentValue.empty())
            return "A video mode must be selected.";

        it = mOptions.find( "Rendering Device" );
        bool foundDriver = false;
        D3D9DriverList* driverList = getDirect3DDrivers();
        for( ushort j=0; j < driverList->count(); j++ )
        {
            if( driverList->item(j)->DriverDescription() == it->second.currentValue )
            {
                foundDriver = true;
                break;
            }
        }

        if (!foundDriver)
        {
            // Just pick the first driver
            setConfigOption("Rendering Device", driverList->item(0)->DriverDescription());
            return "Your DirectX driver name has changed since the last time you ran OGRE; "
                "the 'Rendering Device' has been changed.";
        }

		it = mOptions.find( "VSync" );
		if( it->second.currentValue == "Yes" )
			mVSync = true;
		else
			mVSync = false;

        return BLANKSTRING;
    }
    //---------------------------------------------------------------------
    ConfigOptionMap& D3D9RenderSystem::getConfigOptions()
    {
        // return a COPY of the current config options
        return mOptions;
    }
    //---------------------------------------------------------------------
    RenderWindow* D3D9RenderSystem::_initialise( bool autoCreateWindow, const String& windowTitle )
    {
        RenderWindow* autoWindow = NULL;
        LogManager::getSingleton().logMessage( "D3D9 : Subsystem Initialising" );

        // Init using current settings
        mActiveD3DDriver = NULL;
        ConfigOptionMap::iterator opt = mOptions.find( "Rendering Device" );
        for( uint j=0; j < getDirect3DDrivers()->count(); j++ )
        {
            if( getDirect3DDrivers()->item(j)->DriverDescription() == opt->second.currentValue )
            {
                mActiveD3DDriver = getDirect3DDrivers()->item(j);
                break;
            }
        }

        if( !mActiveD3DDriver )
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, "Problems finding requested Direct3D driver!", "D3D9RenderSystem::initialise" );

        // get driver version
        mDriverVersion.major = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
        mDriverVersion.minor = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.HighPart);
        mDriverVersion.release = HIWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);
        mDriverVersion.build = LOWORD(mActiveD3DDriver->getAdapterIdentifier().DriverVersion.LowPart);

        // Create the device manager.
        mDeviceManager = OGRE_NEW D3D9DeviceManager();

        // Create the texture manager for use by others     
        mTextureManager = OGRE_NEW D3D9TextureManager();

        // Also create hardware buffer manager      
        mHardwareBufferManager = OGRE_NEW D3D9HardwareBufferManager();

        // Create the GPU program manager       
        mGpuProgramManager = OGRE_NEW D3D9GpuProgramManager();

        // Create & register HLSL factory       
        mHLSLProgramFactory = OGRE_NEW D3D9HLSLProgramFactory();
        
        if( autoCreateWindow )
        {
            bool fullScreen;
            opt = mOptions.find( "Full Screen" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find full screen option!", "D3D9RenderSystem::initialise" );
            fullScreen = opt->second.currentValue == "Yes";

            D3D9VideoMode* videoMode = NULL;
            unsigned int width, height;
            String temp;

            opt = mOptions.find( "Video Mode" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find Video Mode option!", "D3D9RenderSystem::initialise" );

            // The string we are manipulating looks like this :width x height @ colourDepth
            // Pull out the colour depth by getting what comes after the @ and a space
            String colourDepth = opt->second.currentValue.substr(opt->second.currentValue.rfind('@')+1);
            // Now we know that the width starts a 0, so if we can find the end we can parse that out
            String::size_type widthEnd = opt->second.currentValue.find(' ');
            // we know that the height starts 3 characters after the width and goes until the next space
            String::size_type heightEnd = opt->second.currentValue.find(' ', widthEnd+3);
            // Now we can parse out the values
            width = StringConverter::parseInt(opt->second.currentValue.substr(0, widthEnd));
            height = StringConverter::parseInt(opt->second.currentValue.substr(widthEnd+3, heightEnd));

            for( unsigned j=0; j < mActiveD3DDriver->getVideoModeList()->count(); j++ )
            {
                temp = mActiveD3DDriver->getVideoModeList()->item(j)->getDescription();

                // In full screen we only want to allow supported resolutions, so temp and opt->second.currentValue need to 
                // match exactly, but in windowed mode we can allow for arbitrary window sized, so we only need
                // to match the colour values
                if(fullScreen && (temp == opt->second.currentValue) ||
                    !fullScreen && (temp.substr(temp.rfind('@')+1) == colourDepth))
                {
                    videoMode = mActiveD3DDriver->getVideoModeList()->item(j);
                    break;
                }
            }

            if( !videoMode )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find requested video mode.", "D3D9RenderSystem::initialise" );

            // sRGB window option
            bool hwGamma = false;
            opt = mOptions.find( "sRGB Gamma Conversion" );
            if( opt == mOptions.end() )
                OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, "Can't find sRGB option!", "D3D9RenderSystem::initialise" );
            hwGamma = opt->second.currentValue == "Yes";

            NameValuePairList miscParams;
            miscParams["colourDepth"] = StringConverter::toString(videoMode->getColourDepth());
            miscParams["FSAA"] = StringConverter::toString(mFSAASamples);
            miscParams["FSAAHint"] = mFSAAHint;
			miscParams["vsync"] = StringConverter::toString(mVSync);
			miscParams["vsyncInterval"] = StringConverter::toString(mVSyncInterval);

            miscParams["useNVPerfHUD"] = StringConverter::toString(mUseNVPerfHUD);
            miscParams["gamma"] = StringConverter::toString(hwGamma);
            miscParams["monitorIndex"] = StringConverter::toString(static_cast<int>(mActiveD3DDriver->getAdapterNumber()));
			miscParams["Backbuffer Count"] = StringConverter::toString(mBackBufferCount);
			
            opt = mOptions.find("VSync");
            if (opt == mOptions.end())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find VSync options!", "D3D9RenderSystem::initialise");
            bool vsync = (opt->second.currentValue == "Yes");
            miscParams["vsync"] = StringConverter::toString(vsync);

            opt = mOptions.find("VSync Interval");
            if (opt == mOptions.end())
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't find VSync Interval options!", "D3D9RenderSystem::initialise");
            miscParams["vsyncInterval"] = opt->second.currentValue;

            autoWindow = _createRenderWindow( windowTitle, width, height, 
                fullScreen, &miscParams );

            // If we have 16bit depth buffer enable w-buffering.
            assert( autoWindow );
            if ( autoWindow->getColourDepth() == 16 ) 
            { 
                mWBuffer = true;
            } 
            else 
            {
                mWBuffer = false;
            }
        }

        LogManager::getSingleton().logMessage("***************************************");
        LogManager::getSingleton().logMessage("*** D3D9 : Subsystem Initialised OK ***");
        LogManager::getSingleton().logMessage("***************************************");

        // call superclass method
        RenderSystem::_initialise( autoCreateWindow );


        return autoWindow;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::reinitialise()
    {
        LogManager::getSingleton().logMessage( "D3D9 : Reinitialising" );
        this->shutdown();
        this->_initialise( true );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::shutdown()
    {
        RenderSystem::shutdown();
        
        if (mDeviceManager != NULL)
        {
            OGRE_DELETE mDeviceManager;
            mDeviceManager = NULL;
        }

        if (mDriverList != NULL)
        {
            OGRE_DELETE mDriverList;
            mDriverList = NULL;
        }               
        mActiveD3DDriver = NULL;    
                        
        LogManager::getSingleton().logMessage("D3D9 : Shutting down cleanly.");
        
        if (mTextureManager != NULL)
        {
            OGRE_DELETE mTextureManager;
            mTextureManager = NULL;
        }

        if (mHardwareBufferManager != NULL)
        {
            OGRE_DELETE mHardwareBufferManager;
            mHardwareBufferManager = NULL;
        }

        if (mGpuProgramManager != NULL)
        {
            OGRE_DELETE mGpuProgramManager;
            mGpuProgramManager = NULL;
        }           
    }
    //---------------------------------------------------------------------
    RenderWindow* D3D9RenderSystem::_createRenderWindow(const String &name, 
        unsigned int width, unsigned int height, bool fullScreen,
        const NameValuePairList *miscParams)
    {       
        // Log a message
        StringStream ss;
        ss << "D3D9RenderSystem::_createRenderWindow \"" << name << "\", " <<
            width << "x" << height << " ";

        if(fullScreen)
            ss << "fullscreen ";
        else
            ss << "windowed ";

        if(miscParams)
        {
            ss << " miscParams: ";
            NameValuePairList::const_iterator it;
            for(it=miscParams->begin(); it!=miscParams->end(); ++it)
            {
                ss << it->first << "=" << it->second << " ";
            }
            LogManager::getSingleton().logMessage(ss.str());
        }

        String msg;

        // Make sure we don't already have a render target of the 
        // same name as the one supplied
        if( mRenderTargets.find( name ) != mRenderTargets.end() )
        {
            msg = "A render target of the same name '" + name + "' already "
                "exists.  You cannot create a new window with this name.";
            OGRE_EXCEPT( Exception::ERR_INTERNAL_ERROR, msg, "D3D9RenderSystem::_createRenderWindow" );
        }

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        // Stereo driver must be created before device is created
        createStereoDriver(miscParams);
#endif

        D3D9RenderWindow* renderWindow = OGRE_NEW D3D9RenderWindow(mhInstance);
        
        renderWindow->create(name, width, height, fullScreen, miscParams);

        mResourceManager->lockDeviceAccess();

        try
        {
            mDeviceManager->linkRenderWindow(renderWindow);
        }
        catch (const Ogre::RenderingAPIException&)
        {
            // after catching the exception, clean up
            mResourceManager->unlockDeviceAccess();
            renderWindow->destroy();

            // re-throw
            throw;
        }

        mResourceManager->unlockDeviceAccess();
    
        mRenderWindows.push_back(renderWindow);     
        
        updateRenderSystemCapabilities(renderWindow);

        attachRenderTarget( *renderWindow );
        
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        // Must be called after device has been linked to window
        D3D9StereoDriverBridge::getSingleton().addRenderWindow(renderWindow);
        renderWindow->_validateStereo();
#endif

        return renderWindow;
    }   
    //---------------------------------------------------------------------
    bool D3D9RenderSystem::_createRenderWindows(const RenderWindowDescriptionList& renderWindowDescriptions, 
        RenderWindowList& createdWindows)
    {
        // Call base render system method.
        if (false == RenderSystem::_createRenderWindows(renderWindowDescriptions, createdWindows))
            return false;

        // Simply call _createRenderWindow in a loop.
        for (size_t i = 0; i < renderWindowDescriptions.size(); ++i)
        {
            const RenderWindowDescription& curRenderWindowDescription = renderWindowDescriptions[i];            
            RenderWindow* curWindow = NULL;

            curWindow = _createRenderWindow(curRenderWindowDescription.name, 
                curRenderWindowDescription.width, 
                curRenderWindowDescription.height, 
                curRenderWindowDescription.useFullScreen, 
                &curRenderWindowDescription.miscParams);
                            
            createdWindows.push_back(curWindow);                                            
        }
        
        return true;
    }

    //---------------------------------------------------------------------
    RenderSystemCapabilities* D3D9RenderSystem::updateRenderSystemCapabilities(D3D9RenderWindow* renderWindow)
    {           
        RenderSystemCapabilities* rsc = mRealCapabilities;
        if (rsc == NULL)
            rsc = OGRE_NEW RenderSystemCapabilities();

        rsc->setCategoryRelevant(CAPS_CATEGORY_D3D9, true);
        rsc->setDriverVersion(mDriverVersion);
        rsc->setDeviceName(mActiveD3DDriver->DriverDescription());
        rsc->setRenderSystemName(getName());

        if(mEnableFixedPipeline)
        {
            // Supports fixed-function
            rsc->setCapability(RSC_FIXED_FUNCTION);
        }   
            
                
        // Init caps to maximum.        
        rsc->setNumTextureUnits(1024);
        rsc->setCapability(RSC_ANISOTROPY);
        rsc->setCapability(RSC_AUTOMIPMAP);
        rsc->setCapability(RSC_DOT3);
        rsc->setCapability(RSC_CUBEMAPPING);        
        rsc->setCapability(RSC_TWO_SIDED_STENCIL);      
        rsc->setCapability(RSC_STENCIL_WRAP);
        rsc->setCapability(RSC_HWOCCLUSION);        
        rsc->setCapability(RSC_USER_CLIP_PLANES);           
        rsc->setCapability(RSC_32BIT_INDEX);            
        rsc->setCapability(RSC_VERTEX_FORMAT_UBYTE4);           
        rsc->setCapability(RSC_TEXTURE_1D);         
        rsc->setCapability(RSC_TEXTURE_3D);         
        rsc->setCapability(RSC_NON_POWER_OF_2_TEXTURES);
        rsc->setNonPOW2TexturesLimited(false);
        rsc->setNumMultiRenderTargets(OGRE_MAX_MULTIPLE_RENDER_TARGETS);
        rsc->setCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);       
        rsc->setCapability(RSC_POINT_SPRITES);          
        rsc->setCapability(RSC_POINT_EXTENDED_PARAMETERS);                              
        rsc->setMaxPointSize(2.19902e+012f);
        rsc->setCapability(RSC_MIPMAP_LOD_BIAS);                
        rsc->setCapability(RSC_PERSTAGECONSTANT);
        rsc->setCapability(RSC_HWSTENCIL);
        rsc->setStencilBufferBitDepth(8);
        rsc->setCapability(RSC_RTT_SEPARATE_DEPTHBUFFER);
        rsc->setCapability(RSC_RTT_MAIN_DEPTHBUFFER_ATTACHABLE);
        rsc->setCapability(RSC_RTT_DEPTHBUFFER_RESOLUTION_LESSEQUAL);
        rsc->setCapability(RSC_VERTEX_BUFFER_INSTANCE_DATA);
        rsc->setCapability(RSC_CAN_GET_COMPILED_SHADER_BUFFER);

        for (uint i=0; i < mDeviceManager->getDeviceCount(); ++i)
        {
            D3D9Device* device           = mDeviceManager->getDevice(i);
            IDirect3DDevice9* d3d9Device = device->getD3D9Device();

            IDirect3DSurface9* pSurf;
            

            // Check for hardware stencil support
            d3d9Device->GetDepthStencilSurface(&pSurf);

            if (pSurf != NULL)
            {
                D3DSURFACE_DESC surfDesc;

                pSurf->GetDesc(&surfDesc);
                pSurf->Release();

                if (surfDesc.Format != D3DFMT_D15S1 &&
                    surfDesc.Format != D3DFMT_D24S8 &&              
                    surfDesc.Format != D3DFMT_D24X4S4 && 
                    surfDesc.Format != D3DFMT_D24FS8)           
                    rsc->unsetCapability(RSC_HWSTENCIL);    
            }                                                                   

            // Check for hardware occlusion support
            HRESULT hr = d3d9Device->CreateQuery(D3DQUERYTYPE_OCCLUSION,  NULL);

            if (FAILED(hr))
                rsc->unsetCapability(RSC_HWOCCLUSION);
        }
        
        // Update RS caps using the minimum value found in adapter list.
        for (unsigned int i=0; i < mDriverList->count(); ++i)
        {
            D3D9Driver* pCurDriver       = mDriverList->item(i);            
            const D3DCAPS9& rkCurCaps    = pCurDriver->getD3D9DeviceCaps();

            if (rkCurCaps.MaxSimultaneousTextures < rsc->getNumTextureUnits())
            {
                rsc->setNumTextureUnits(static_cast<ushort>(rkCurCaps.MaxSimultaneousTextures));
            }

            // Check for Anisotropy.
            if (rkCurCaps.MaxAnisotropy <= 1)
                rsc->unsetCapability(RSC_ANISOTROPY);

            // Check automatic mipmap generation.
            if ((rkCurCaps.Caps2 & D3DCAPS2_CANAUTOGENMIPMAP) == 0)
                rsc->unsetCapability(RSC_AUTOMIPMAP);

            // Check Dot product 3.
            if ((rkCurCaps.TextureOpCaps & D3DTEXOPCAPS_DOTPRODUCT3) == 0)
                rsc->unsetCapability(RSC_DOT3);

            // Scissor test
            //if ((rkCurCaps.RasterCaps & D3DPRASTERCAPS_SCISSORTEST) == 0)
            //    rsc->unsetCapability(RSC_SCISSOR_TEST);


            // Two-sided stencil
            if ((rkCurCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED) == 0)
                rsc->unsetCapability(RSC_TWO_SIDED_STENCIL);

            // stencil wrap
            if ((rkCurCaps.StencilCaps & D3DSTENCILCAPS_INCR) == 0 ||
                (rkCurCaps.StencilCaps & D3DSTENCILCAPS_DECR) == 0)
                rsc->unsetCapability(RSC_STENCIL_WRAP);

            // User clip planes
            if (rkCurCaps.MaxUserClipPlanes == 0)           
                rsc->unsetCapability(RSC_USER_CLIP_PLANES);         

            // D3DFMT_INDEX32 type?
            if (rkCurCaps.MaxVertexIndex <= 0xFFFF)         
                rsc->unsetCapability(RSC_32BIT_INDEX);  

            // UBYTE4 type?
            if ((rkCurCaps.DeclTypes & D3DDTCAPS_UBYTE4) == 0)          
                rsc->unsetCapability(RSC_VERTEX_FORMAT_UBYTE4); 

            // Check cube map support.
            if ((rkCurCaps.TextureCaps & D3DPTEXTURECAPS_CUBEMAP) == 0)
                rsc->unsetCapability(RSC_CUBEMAPPING);
            
            // 3D textures?
            if ((rkCurCaps.TextureCaps & D3DPTEXTURECAPS_VOLUMEMAP) == 0)           
                rsc->unsetCapability(RSC_TEXTURE_3D);           

            if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_POW2)
            {
                // Conditional support for non POW2
                if (rkCurCaps.TextureCaps & D3DPTEXTURECAPS_NONPOW2CONDITIONAL)             
                    rsc->setNonPOW2TexturesLimited(true);               

                // Only power of 2 supported.
                else                    
                    rsc->unsetCapability(RSC_NON_POWER_OF_2_TEXTURES);              
            }   

            // Number of render targets
            if (rkCurCaps.NumSimultaneousRTs < rsc->getNumMultiRenderTargets())
            {
                rsc->setNumMultiRenderTargets(std::min((ushort)rkCurCaps.NumSimultaneousRTs, (ushort)OGRE_MAX_MULTIPLE_RENDER_TARGETS));
            }   

            if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS) == 0)
            {
                rsc->unsetCapability(RSC_MRT_DIFFERENT_BIT_DEPTHS);
            }

            // Point sprites 
            if (rkCurCaps.MaxPointSize <= 1.0f)
            {
                rsc->unsetCapability(RSC_POINT_SPRITES);
                // sprites and extended parameters go together in D3D
                rsc->unsetCapability(RSC_POINT_EXTENDED_PARAMETERS);                
            }
            
            // Take the minimum point size.
            if (rkCurCaps.MaxPointSize < rsc->getMaxPointSize())
                rsc->setMaxPointSize(rkCurCaps.MaxPointSize);   

            // Mipmap LOD biasing?
            if ((rkCurCaps.RasterCaps & D3DPRASTERCAPS_MIPMAPLODBIAS) == 0)         
                rsc->unsetCapability(RSC_MIPMAP_LOD_BIAS);          


            // Do we support per-stage src_manual constants?
            // HACK - ATI drivers seem to be buggy and don't support per-stage constants properly?
            // TODO: move this to RSC
            if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_PERSTAGECONSTANT) == 0)
                rsc->unsetCapability(RSC_PERSTAGECONSTANT);

            // Advanced blend operations? min max subtract rev 
            //if((rkCurCaps.PrimitiveMiscCaps & D3DPMISCCAPS_BLENDOP) == 0)
            //   rsc->unsetCapability(RSC_ADVANCED_BLEND_OPERATIONS);
        }               
                                    
        // Blending between stages supported
        rsc->setCapability(RSC_BLENDING);
        

        // We always support compression, D3DX will decompress if device does not support
        rsc->setCapability(RSC_TEXTURE_COMPRESSION);
        rsc->setCapability(RSC_TEXTURE_COMPRESSION_DXT);

        // We always support VBOs
        rsc->setCapability(RSC_VBO);

            
        convertVertexShaderCaps(rsc);
        convertPixelShaderCaps(rsc);

        // Adapter details
        const D3DADAPTER_IDENTIFIER9& adapterID = mActiveD3DDriver->getAdapterIdentifier();

        // determine vendor
        // Full list of vendors here: http://www.pcidatabase.com/vendors.php?sort=id
        switch(adapterID.VendorId)
        {
        case 0x10DE:
            rsc->setVendor(GPU_NVIDIA);
            break;
        case 0x1002:
            rsc->setVendor(GPU_AMD);
            break;
        case 0x163C:
        case 0x8086:
            rsc->setVendor(GPU_INTEL);
            break;
        case 0x5333:
            rsc->setVendor(GPU_S3);
            break;
        case 0x3D3D:
            rsc->setVendor(GPU_3DLABS);
            break;
        case 0x102B:
            rsc->setVendor(GPU_MATROX);
            break;
        case 0x1039:
            rsc->setVendor(GPU_SIS);
            break;
        default:
            rsc->setVendor(GPU_UNKNOWN);
            break;
        };

        // Infinite projection?
        // We have no capability for this, so we have to base this on our
        // experience and reports from users
        // Non-vertex program capable hardware does not appear to support it
        if (rsc->hasCapability(RSC_VERTEX_PROGRAM))
        {
            // GeForce4 Ti (and presumably GeForce3) does not
            // render infinite projection properly, even though it does in GL
            // So exclude all cards prior to the FX range from doing infinite
            if (rsc->getVendor() != GPU_NVIDIA || // not nVidia
                !((adapterID.DeviceId >= 0x200 && adapterID.DeviceId <= 0x20F) || //gf3
                (adapterID.DeviceId >= 0x250 && adapterID.DeviceId <= 0x25F) || //gf4ti
                (adapterID.DeviceId >= 0x280 && adapterID.DeviceId <= 0x28F) || //gf4ti
                (adapterID.DeviceId >= 0x170 && adapterID.DeviceId <= 0x18F) || //gf4 go
                (adapterID.DeviceId >= 0x280 && adapterID.DeviceId <= 0x28F)))  //gf4ti go
            {
                rsc->setCapability(RSC_INFINITE_FAR_PLANE);
            }

        }
    
        // We always support rendertextures bigger than the frame buffer
        rsc->setCapability(RSC_HWRENDER_TO_TEXTURE);

        // Determine if any floating point texture format is supported
        D3DFORMAT floatFormats[6] = {D3DFMT_R16F, D3DFMT_G16R16F, 
            D3DFMT_A16B16G16R16F, D3DFMT_R32F, D3DFMT_G32R32F, 
            D3DFMT_A32B32G32R32F};
        IDirect3DSurface9* bbSurf;
        renderWindow->getCustomAttribute("DDBACKBUFFER", &bbSurf);
        D3DSURFACE_DESC bbSurfDesc;
        bbSurf->GetDesc(&bbSurfDesc);

        for (int i = 0; i < 6; ++i)
        {
            if (SUCCEEDED(mD3D->CheckDeviceFormat(mActiveD3DDriver->getAdapterNumber(), 
                D3DDEVTYPE_HAL, bbSurfDesc.Format, 
                0, D3DRTYPE_TEXTURE, floatFormats[i])))
            {
                rsc->setCapability(RSC_TEXTURE_FLOAT);
                break;
            }

        }

    
        // TODO: make convertVertex/Fragment fill in rsc
        // TODO: update the below line to use rsc
        // Vertex textures
        if (rsc->isShaderProfileSupported("vs_3_0"))
        {
            // Run through all the texture formats looking for any which support
            // vertex texture fetching. Must have at least one!
            // All ATI Radeon up to X1n00 say they support vs_3_0, 
            // but they support no texture formats for vertex texture fetch (cheaters!)
            if (checkVertexTextureFormats(renderWindow))
            {
                rsc->setCapability(RSC_VERTEX_TEXTURE_FETCH);
                // always 4 vertex texture units in vs_3_0, and never shared
                rsc->setNumVertexTextureUnits(4);
                rsc->setVertexTextureUnitsShared(false);
            }
        }   
        else
        {
            //True HW Instancing is supported since Shader model 3.0 ATI has a nasty
            //hack for enabling it in their SM 2.0 cards, but we don't (and won't) support it
            rsc->unsetCapability( RSC_VERTEX_BUFFER_INSTANCE_DATA );
        }

        // Check alpha to coverage support
        // this varies per vendor! But at least SM3 is required
        if (rsc->isShaderProfileSupported("ps_3_0"))
        {
            // NVIDIA needs a separate check
            if (rsc->getVendor() == GPU_NVIDIA)
            {
                if (mD3D->CheckDeviceFormat(
                        D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0,D3DRTYPE_SURFACE, 
                        (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C')) == S_OK)
                {
                    rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
                }

            }
            else if (rsc->getVendor() == GPU_AMD)
            {
                // There is no check on ATI, we have to assume SM3 == support
                rsc->setCapability(RSC_ALPHA_TO_COVERAGE);
            }

            // no other cards have Dx9 hacks for alpha to coverage, as far as I know
        }


        if (mRealCapabilities == NULL)
        {       
            mRealCapabilities = rsc;
            mRealCapabilities->addShaderProfile("hlsl");

            // if we are using custom capabilities, then 
            // mCurrentCapabilities has already been loaded
            if(!mUseCustomCapabilities)
                mCurrentCapabilities = mRealCapabilities;

            fireEvent("RenderSystemCapabilitiesCreated");

            initialiseFromRenderSystemCapabilities(mCurrentCapabilities, renderWindow);
        }

        return rsc;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::convertVertexShaderCaps(RenderSystemCapabilities* rsc) const
    {
        ushort major = 0xFF;
        ushort minor = 0xFF;
        D3DCAPS9 minVSCaps;

        // Find the device with the lowest vertex shader caps.
        for (unsigned int i=0; i < mDriverList->count(); ++i)
        {
            D3D9Driver* pCurDriver      = mDriverList->item(i);         
            const D3DCAPS9& rkCurCaps   = pCurDriver->getD3D9DeviceCaps();
            ushort currMajor            = static_cast<ushort>((rkCurCaps.VertexShaderVersion & 0x0000FF00) >> 8);
            ushort currMinor            = static_cast<ushort>(rkCurCaps.VertexShaderVersion & 0x000000FF);

            if (currMajor < major)  
            {
                major = currMajor;
                minor = currMinor;
                minVSCaps = rkCurCaps;
            }
            else if (currMajor == major && currMinor < minor)
            {
                minor = currMinor;
                minVSCaps = rkCurCaps;
            }           
        }

        // In case we didn't found any vertex shader support
        // try the IDirect3DDevice9 caps instead of the IDirect3D9
        // software vertex processing is reported there
        if (major == 0 && minor == 0)
        {
            IDirect3DDevice9* lpD3DDevice9 = getActiveD3D9Device();
            D3DCAPS9 d3dDeviceCaps9;
            lpD3DDevice9->GetDeviceCaps(&d3dDeviceCaps9);
            major = static_cast<ushort>((d3dDeviceCaps9.VertexShaderVersion & 0x0000FF00) >> 8);
            minor = static_cast<ushort>(d3dDeviceCaps9.VertexShaderVersion & 0x000000FF);
        }
        
        bool vs2x = false;
        bool vs2a = false;

        // Special case detection for vs_2_x/a support
        if (major >= 2)
        {
            if ((minVSCaps.VS20Caps.Caps & D3DVS20CAPS_PREDICATION) &&
                (minVSCaps.VS20Caps.DynamicFlowControlDepth > 0) &&
                (minVSCaps.VS20Caps.NumTemps >= 12))
            {
                vs2x = true;
            }

            if ((minVSCaps.VS20Caps.Caps & D3DVS20CAPS_PREDICATION) &&
                (minVSCaps.VS20Caps.DynamicFlowControlDepth > 0) &&
                (minVSCaps.VS20Caps.NumTemps >= 13))
            {
                vs2a = true;
            }
        }

        // Populate max param count
        switch (major)
        {
        case 1:
            // No boolean params allowed
            rsc->setVertexProgramConstantBoolCount(0);
            // No integer params allowed
            rsc->setVertexProgramConstantIntCount(0);
            // float params, always 4D
            rsc->setVertexProgramConstantFloatCount(static_cast<ushort>(minVSCaps.MaxVertexShaderConst));

            break;
        case 2:
            // 16 boolean params allowed
            rsc->setVertexProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setVertexProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setVertexProgramConstantFloatCount(static_cast<ushort>(minVSCaps.MaxVertexShaderConst));
            break;
        case 3:
            // 16 boolean params allowed
            rsc->setVertexProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setVertexProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setVertexProgramConstantFloatCount(static_cast<ushort>(minVSCaps.MaxVertexShaderConst));
            break;
        }

        // populate syntax codes in program manager (no breaks in this one so it falls through)
        switch(major)
        {
        case 3:
            rsc->addShaderProfile("vs_3_0");
        case 2:
            if (vs2x)
                rsc->addShaderProfile("vs_2_x");
            if (vs2a)
                rsc->addShaderProfile("vs_2_a");

            rsc->addShaderProfile("vs_2_0");
        case 1:
            rsc->addShaderProfile("vs_1_1");
            rsc->setCapability(RSC_VERTEX_PROGRAM);
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::convertPixelShaderCaps(RenderSystemCapabilities* rsc) const
    {
        ushort major = 0xFF;
        ushort minor = 0xFF;
        D3DCAPS9 minPSCaps;

        // Find the device with the lowest pixel shader caps.
        for (unsigned int i=0; i < mDriverList->count(); ++i)
        {
            D3D9Driver* pCurDriver      = mDriverList->item(i);         
            const D3DCAPS9& currCaps    = pCurDriver->getD3D9DeviceCaps();
            ushort currMajor            = static_cast<ushort>((currCaps.PixelShaderVersion & 0x0000FF00) >> 8);
            ushort currMinor            = static_cast<ushort>(currCaps.PixelShaderVersion & 0x000000FF);

            if (currMajor < major)  
            {
                major = currMajor;
                minor = currMinor;
                minPSCaps = currCaps;
            }
            else if (currMajor == major && currMinor < minor)
            {
                minor = currMinor;
                minPSCaps = currCaps;
            }           
        }
        
        bool ps2a = false;
        bool ps2b = false;
        bool ps2x = false;

        // Special case detection for ps_2_x/a/b support
        if (major >= 2)
        {
            if ((minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT) &&
                (minPSCaps.PS20Caps.NumTemps >= 32))
            {
                ps2b = true;
            }

            if ((minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT) &&
                (minPSCaps.PS20Caps.Caps & D3DPS20CAPS_NODEPENDENTREADLIMIT) &&
                (minPSCaps.PS20Caps.Caps & D3DPS20CAPS_ARBITRARYSWIZZLE) &&
                (minPSCaps.PS20Caps.Caps & D3DPS20CAPS_GRADIENTINSTRUCTIONS) &&
                (minPSCaps.PS20Caps.Caps & D3DPS20CAPS_PREDICATION) &&
                (minPSCaps.PS20Caps.NumTemps >= 22))
            {
                ps2a = true;
            }

            // Does this enough?
            if (ps2a || ps2b)
            {
                ps2x = true;
            }
        }

        switch (major)
        {
        case 1:
            // no boolean params allowed
            rsc->setFragmentProgramConstantBoolCount(0);
            // no integer params allowed
            rsc->setFragmentProgramConstantIntCount(0);
            // float params, always 4D
            // NB in ps_1_x these are actually stored as fixed point values,
            // but they are entered as floats
            rsc->setFragmentProgramConstantFloatCount(8);
            break;
        case 2:
            // 16 boolean params allowed
            rsc->setFragmentProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setFragmentProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setFragmentProgramConstantFloatCount(32);
            break;
        case 3:
            // 16 boolean params allowed
            rsc->setFragmentProgramConstantBoolCount(16);
            // 16 integer params allowed, 4D
            rsc->setFragmentProgramConstantIntCount(16);
            // float params, always 4D
            rsc->setFragmentProgramConstantFloatCount(224);
            break;
        }

        // populate syntax codes in program manager (no breaks in this one so it falls through)
        switch(major)
        {
        case 3:
            if (minor > 0)
                rsc->addShaderProfile("ps_3_x");

            rsc->addShaderProfile("ps_3_0");
        case 2:
            if (ps2x)
                rsc->addShaderProfile("ps_2_x");
            if (ps2a)
                rsc->addShaderProfile("ps_2_a");
            if (ps2b)
                rsc->addShaderProfile("ps_2_b");

            rsc->addShaderProfile("ps_2_0");
        case 1:
            if (major > 1 || minor >= 4)
                rsc->addShaderProfile("ps_1_4");
            if (major > 1 || minor >= 3)
                rsc->addShaderProfile("ps_1_3");
            if (major > 1 || minor >= 2)
                rsc->addShaderProfile("ps_1_2");

            rsc->addShaderProfile("ps_1_1");
            rsc->setCapability(RSC_FRAGMENT_PROGRAM);
        }
    }
    //-----------------------------------------------------------------------
    bool D3D9RenderSystem::checkVertexTextureFormats(D3D9RenderWindow* renderWindow) const
    {
        bool anySupported = false;

        IDirect3DSurface9* bbSurf;
        renderWindow->getCustomAttribute("DDBACKBUFFER", &bbSurf);
        D3DSURFACE_DESC bbSurfDesc;
        bbSurf->GetDesc(&bbSurfDesc);

        for (uint ipf = static_cast<uint>(PF_L8); ipf < static_cast<uint>(PF_COUNT); ++ipf)
        {
            PixelFormat pf = (PixelFormat)ipf;

            D3DFORMAT fmt = 
                D3D9Mappings::_getPF(D3D9Mappings::_getClosestSupportedPF(pf));

            if (SUCCEEDED(mD3D->CheckDeviceFormat(
                mActiveD3DDriver->getAdapterNumber(), D3DDEVTYPE_HAL, bbSurfDesc.Format, 
                D3DUSAGE_QUERY_VERTEXTEXTURE, D3DRTYPE_TEXTURE, fmt)))
            {
                // cool, at least one supported
                anySupported = true;
                LogManager::getSingleton().stream()
                    << "D3D9: Vertex texture format supported - "
                    << PixelUtil::getFormatName(pf);
            }
        }

        return anySupported;

    }
    //-----------------------------------------------------------------------
    void D3D9RenderSystem::initialiseFromRenderSystemCapabilities(RenderSystemCapabilities* caps, RenderTarget* primary)
    {
        if (caps->getRenderSystemName() != getName())
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Trying to initialize D3D9RenderSystem from RenderSystemCapabilities that do not support Direct3D9",
                "D3D9RenderSystem::initialiseFromRenderSystemCapabilities");
        }
        if (caps->isShaderProfileSupported("hlsl"))
            HighLevelGpuProgramManager::getSingleton().addFactory(mHLSLProgramFactory);

        Log* defaultLog = LogManager::getSingleton().getDefaultLog();
        if (defaultLog)
        {
            caps->log(defaultLog);
        }
    }

    //-----------------------------------------------------------------------
    bool D3D9RenderSystem::_checkTextureFilteringSupported(TextureType ttype, PixelFormat format, int usage)
    {
        // Gets D3D format
        D3DFORMAT d3dPF = D3D9Mappings::_getPF(format);
        if (d3dPF == D3DFMT_UNKNOWN)
            return false;

        for (uint i = 0; i < mDeviceManager->getDeviceCount(); ++i)
        {
            D3D9Device* currDevice = mDeviceManager->getDevice(i);
            D3D9RenderWindow* currDevicePrimaryWindow = currDevice->getPrimaryWindow();
            IDirect3DSurface9* pSurface = currDevicePrimaryWindow->getRenderSurface();
            D3DSURFACE_DESC srfDesc;
            
            // Get surface desc
            if (FAILED(pSurface->GetDesc(&srfDesc)))
                return false;

            // Calculate usage
            DWORD d3dusage = D3DUSAGE_QUERY_FILTER;
            if (usage & TU_RENDERTARGET) 
                d3dusage |= D3DUSAGE_RENDERTARGET;
            if (usage & TU_DYNAMIC)
                d3dusage |= D3DUSAGE_DYNAMIC;

            // Detect resource type
            D3DRESOURCETYPE rtype;
            switch(ttype)
            {
            case TEX_TYPE_1D:
            case TEX_TYPE_2D:
                rtype = D3DRTYPE_TEXTURE;
                break;
            case TEX_TYPE_3D:
                rtype = D3DRTYPE_VOLUMETEXTURE;
                break;
            case TEX_TYPE_CUBE_MAP:
                rtype = D3DRTYPE_CUBETEXTURE;
                break;
            default:
                return false;
            }

            HRESULT hr = mD3D->CheckDeviceFormat(
                currDevice->getAdapterNumber(),
                currDevice->getDeviceType(),
                srfDesc.Format,
                d3dusage,
                rtype,
                d3dPF);

            if (FAILED(hr))
                return false;
        }
        
        return true;        
    }
    //-----------------------------------------------------------------------
    MultiRenderTarget * D3D9RenderSystem::createMultiRenderTarget(const String & name)
    {
        MultiRenderTarget *retval;
        retval = OGRE_NEW D3D9MultiRenderTarget(name);
        attachRenderTarget(*retval);

        return retval;
    }
    //---------------------------------------------------------------------
    RenderTarget* D3D9RenderSystem::detachRenderTarget(const String &name)
    {
        RenderTarget* target = RenderSystem::detachRenderTarget(name);
        detachRenderTargetImpl(name);
        return target;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::detachRenderTargetImpl(const String& name)
    {
        // Check render windows
        D3D9RenderWindowList::iterator sw;
        for (sw = mRenderWindows.begin(); sw != mRenderWindows.end(); ++sw)
        {
            if ((*sw)->getName() == name)
            {                   
                mRenderWindows.erase(sw);
                break;
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::destroyRenderTarget(const String& name)
    {       
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
        D3D9StereoDriverBridge::getSingleton().removeRenderWindow(name);
#endif

        detachRenderTargetImpl(name);

        // Do the real removal
        RenderSystem::destroyRenderTarget(name);    
    }
    //---------------------------------------------------------------------
    String D3D9RenderSystem::getErrorDescription( long errorNumber ) const
    {
        const String errMsg = DXGetErrorDescription( errorNumber );
        return errMsg;
    }
    //---------------------------------------------------------------------
    VertexElementType D3D9RenderSystem::getColourVertexElementType() const
    {
        return VET_COLOUR_ARGB;
	}
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setAmbientLight( float r, float g, float b )
    {
        HRESULT hr = __SetRenderState( D3DRS_AMBIENT, D3DCOLOR_COLORVALUE( r, g, b, 1.0f ) );
        if( FAILED( hr ) )
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR, 
            "Failed to set render stat D3DRS_AMBIENT", "D3D9RenderSystem::setAmbientLight" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_useLights(const LightList& lights, unsigned short limit)
    {
        IDirect3DDevice9* activeDevice = getActiveD3D9Device();
        LightList::const_iterator i, iend;
        iend = lights.end();
        unsigned short num = 0;
        for (i = lights.begin(); i != iend && num < limit; ++i, ++num)
        {
            setD3D9Light(num, i->light);
        }
        // Disable extra lights
        for (; num < mCurrentLights[activeDevice]; ++num)
        {
            setD3D9Light(num, NULL);
        }
        mCurrentLights[activeDevice] = std::min(limit, static_cast<unsigned short>(lights.size()));

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setD3D9Light( size_t index, const Light* lt )
    {
        HRESULT hr;

        D3DLIGHT9 d3dLight;
        ZeroMemory( &d3dLight, sizeof(d3dLight) );

        if (!lt)
        {
            if( FAILED( hr = getActiveD3D9Device()->LightEnable( static_cast<DWORD>(index), FALSE) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Unable to disable light", "D3D9RenderSystem::setD3D9Light" );
        }
        else
        {
            switch( lt->getType() )
            {
            case Light::LT_POINT:
                d3dLight.Type = D3DLIGHT_POINT;
                break;

            case Light::LT_DIRECTIONAL:
                d3dLight.Type = D3DLIGHT_DIRECTIONAL;
                break;

            case Light::LT_SPOTLIGHT:
                d3dLight.Type = D3DLIGHT_SPOT;
                d3dLight.Falloff = lt->getSpotlightFalloff();
                d3dLight.Theta = lt->getSpotlightInnerAngle().valueRadians();
                d3dLight.Phi = lt->getSpotlightOuterAngle().valueRadians();
                break;
            }

            ColourValue col;
            col = lt->getDiffuseColour();
            d3dLight.Diffuse = D3DXCOLOR( col.r, col.g, col.b, col.a );

            col = lt->getSpecularColour();
            d3dLight.Specular = D3DXCOLOR( col.r, col.g, col.b, col.a );

            Vector3 vec;
            if( lt->getType() != Light::LT_DIRECTIONAL )
            {
                vec = lt->getParentNode()->_getDerivedPosition();
                d3dLight.Position = D3DXVECTOR3( vec.x, vec.y, vec.z );
            }
            if( lt->getType() != Light::LT_POINT )
            {
                vec = lt->getDerivedDirection();
                d3dLight.Direction = D3DXVECTOR3( vec.x, vec.y, vec.z );
            }

            d3dLight.Range = lt->getAttenuationRange();
            d3dLight.Attenuation0 = lt->getAttenuationConstant();
            d3dLight.Attenuation1 = lt->getAttenuationLinear();
            d3dLight.Attenuation2 = lt->getAttenuationQuadric();

            if( FAILED( hr = getActiveD3D9Device()->SetLight( static_cast<DWORD>(index), &d3dLight ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set light details", "D3D9RenderSystem::setD3D9Light" );

            if( FAILED( hr = getActiveD3D9Device()->LightEnable( static_cast<DWORD>(index), TRUE ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to enable light", "D3D9RenderSystem::setD3D9Light" );
        }


    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setViewMatrix( const Matrix4 &m )
    {
        // save latest view matrix
        mViewMatrix = m;
        mViewMatrix[2][0] = -mViewMatrix[2][0];
        mViewMatrix[2][1] = -mViewMatrix[2][1];
        mViewMatrix[2][2] = -mViewMatrix[2][2];
        mViewMatrix[2][3] = -mViewMatrix[2][3];

        mDxViewMat = D3D9Mappings::makeD3DXMatrix( mViewMatrix );

        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->SetTransform( D3DTS_VIEW, &mDxViewMat ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Cannot set D3D9 view matrix", "D3D9RenderSystem::_setViewMatrix" );

        // also mark clip planes dirty
        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setProjectionMatrix( const Matrix4 &m )
    {
        // save latest matrix
        mDxProjMat = D3D9Mappings::makeD3DXMatrix( m );

        if( mActiveRenderTarget->requiresTextureFlipping() )
        {
            // Invert transformed y
            mDxProjMat._12 = - mDxProjMat._12;
            mDxProjMat._22 = - mDxProjMat._22;
            mDxProjMat._32 = - mDxProjMat._32;
            mDxProjMat._42 = - mDxProjMat._42;
        }

        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->SetTransform( D3DTS_PROJECTION, &mDxProjMat ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Cannot set D3D9 projection matrix", "D3D9RenderSystem::_setProjectionMatrix" );

        // also mark clip planes dirty
        if (!mClipPlanes.empty())
            mClipPlanesDirty = true;

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setWorldMatrix( const Matrix4 &m )
    {
        // save latest matrix
        mDxWorldMat = D3D9Mappings::makeD3DXMatrix( m );

        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->SetTransform( D3DTS_WORLD, &mDxWorldMat ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Cannot set D3D9 world matrix", "D3D9RenderSystem::_setWorldMatrix" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setSurfaceParams( const ColourValue &ambient, const ColourValue &diffuse,
        const ColourValue &specular, const ColourValue &emissive, Real shininess,
        TrackVertexColourType tracking )
    {

        D3DMATERIAL9 material;
        material.Diffuse = D3DXCOLOR( diffuse.r, diffuse.g, diffuse.b, diffuse.a );
        material.Ambient = D3DXCOLOR( ambient.r, ambient.g, ambient.b, ambient.a );
        material.Specular = D3DXCOLOR( specular.r, specular.g, specular.b, specular.a );
        material.Emissive = D3DXCOLOR( emissive.r, emissive.g, emissive.b, emissive.a );
        material.Power = shininess;

        HRESULT hr = getActiveD3D9Device()->SetMaterial( &material );
        if( FAILED( hr ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting D3D material", "D3D9RenderSystem::_setSurfaceParams" );


        if(tracking != TVC_NONE) 
        {
            __SetRenderState(D3DRS_COLORVERTEX, TRUE);
            __SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, (tracking&TVC_AMBIENT)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, (tracking&TVC_DIFFUSE)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_SPECULARMATERIALSOURCE, (tracking&TVC_SPECULAR)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
            __SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, (tracking&TVC_EMISSIVE)?D3DMCS_COLOR1:D3DMCS_MATERIAL);
        } 
        else 
        {
            __SetRenderState(D3DRS_COLORVERTEX, FALSE);               
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setPointParameters(Real size, 
        bool attenuationEnabled, Real constant, Real linear, Real quadratic,
        Real minSize, Real maxSize)
    {
        if(attenuationEnabled)
        {
            // scaling required
            __SetRenderState(D3DRS_POINTSCALEENABLE, TRUE);
            __SetFloatRenderState(D3DRS_POINTSCALE_A, constant);
            __SetFloatRenderState(D3DRS_POINTSCALE_B, linear);
            __SetFloatRenderState(D3DRS_POINTSCALE_C, quadratic);
        }
        else
        {
            // no scaling required
            __SetRenderState(D3DRS_POINTSCALEENABLE, FALSE);
        }
        __SetFloatRenderState(D3DRS_POINTSIZE, size);
        __SetFloatRenderState(D3DRS_POINTSIZE_MIN, minSize);
        if (maxSize == 0.0f)
            maxSize = mCurrentCapabilities->getMaxPointSize();
        __SetFloatRenderState(D3DRS_POINTSIZE_MAX, maxSize);


    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setPointSpritesEnabled(bool enabled)
    {
        if (enabled)
        {
            __SetRenderState(D3DRS_POINTSPRITEENABLE, TRUE);
        }
        else
        {
            __SetRenderState(D3DRS_POINTSPRITEENABLE, FALSE);
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTexture( size_t stage, bool enabled, const TexturePtr& tex )
    {
        HRESULT hr;
        D3D9TexturePtr dt = tex.staticCast<D3D9Texture>();
        if (enabled && !dt.isNull())
        {
            // note used
            dt->touch();

            IDirect3DBaseTexture9 *pTex = dt->getTexture();
            if (mTexStageDesc[stage].pTex != pTex)
            {
                hr = getActiveD3D9Device()->SetTexture(static_cast<DWORD>(stage), pTex);
                if( hr != S_OK )
                {
                    String str = "Unable to set texture '" + tex->getName() + "' in D3D9";
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, str, "D3D9RenderSystem::_setTexture" );
                }

                // set stage desc.
                mTexStageDesc[stage].pTex = pTex;
                mTexStageDesc[stage].texType = D3D9Mappings::get(dt->getTextureType());

                // Set gamma now too
                if (dt->isHardwareGammaReadToBeUsed())
                {
                    __SetSamplerState(getSamplerId(stage), D3DSAMP_SRGBTEXTURE, TRUE);
                }
                else
                {
                    __SetSamplerState(getSamplerId(stage), D3DSAMP_SRGBTEXTURE, FALSE);
                }
            }
        }
        else
        {
            if (mTexStageDesc[stage].pTex != 0)
            {
                hr = getActiveD3D9Device()->SetTexture(static_cast<DWORD>(stage), 0);
                if( hr != S_OK )
                {
                    String str = "Unable to disable texture '" + StringConverter::toString(stage) + "' in D3D9";
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, str, "D3D9RenderSystem::_setTexture" );
                }
            }

            hr = __SetTextureStageState(static_cast<DWORD>(stage), D3DTSS_COLOROP, D3DTOP_DISABLE);
            if( hr != S_OK )
            {
                String str = "Unable to disable texture '" + StringConverter::toString(stage) + "' in D3D9";
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, str, "D3D9RenderSystem::_setTexture" );
            }

            // set stage desc. to defaults
            mTexStageDesc[stage].pTex = 0;
            mTexStageDesc[stage].autoTexCoordType = TEXCALC_NONE;
            mTexStageDesc[stage].coordIndex = 0;
            mTexStageDesc[stage].texType = D3D9Mappings::D3D_TEX_TYPE_NORMAL;
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setVertexTexture(size_t stage, const TexturePtr& tex)
    {
        if (tex.isNull())
        {

            if (mTexStageDesc[stage].pVertexTex != 0)
            {
                HRESULT hr = getActiveD3D9Device()->SetTexture(D3DVERTEXTEXTURESAMPLER0 + static_cast<DWORD>(stage), 0);
                if( hr != S_OK )
                {
                    String str = "Unable to disable vertex texture '" 
                        + StringConverter::toString(stage) + "' in D3D9";
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, str, "D3D9RenderSystem::_setVertexTexture" );
                }
            }

            // set stage desc. to defaults
            mTexStageDesc[stage].pVertexTex = 0;
        }
        else
        {
            D3D9TexturePtr dt = tex.staticCast<D3D9Texture>();
            // note used
            dt->touch();

            IDirect3DBaseTexture9 *pTex = dt->getTexture();
            if (mTexStageDesc[stage].pVertexTex != pTex)
            {
                HRESULT hr = getActiveD3D9Device()->SetTexture(D3DVERTEXTEXTURESAMPLER0 + static_cast<DWORD>(stage), pTex);
                if( hr != S_OK )
                {
                    String str = "Unable to set vertex texture '" + tex->getName() + "' in D3D9";
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, str, "D3D9RenderSystem::_setVertexTexture" );
                }

                // set stage desc.
                mTexStageDesc[stage].pVertexTex = pTex;
            }

        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_disableTextureUnit(size_t texUnit)
    {
        RenderSystem::_disableTextureUnit(texUnit);
        // also disable vertex texture unit
        static TexturePtr nullPtr;
        _setVertexTexture(texUnit, nullPtr);
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureCoordSet( size_t stage, size_t index )
    {
        // if vertex shader is being used, stage and index must match
        if (mVertexProgramBound)
            index = stage;

        HRESULT hr;
        // Record settings
        mTexStageDesc[stage].coordIndex = index;

        if (mVertexProgramBound)
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXCOORDINDEX, index );
        else
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXCOORDINDEX, D3D9Mappings::get(mTexStageDesc[stage].autoTexCoordType, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) | index );
        if( FAILED( hr ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture coord. set index", "D3D9RenderSystem::_setTextureCoordSet" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureCoordCalculation( size_t stage, TexCoordCalcMethod m,
        const Frustum* frustum)
    {
        HRESULT hr;
        // record the stage state
        mTexStageDesc[stage].autoTexCoordType = m;
        mTexStageDesc[stage].frustum = frustum;

        if (mVertexProgramBound)
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXCOORDINDEX, mTexStageDesc[stage].coordIndex );
        else
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXCOORDINDEX, D3D9Mappings::get(m, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) | mTexStageDesc[stage].coordIndex );
        if(FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture auto tex.coord. generation mode", "D3D9RenderSystem::_setTextureCoordCalculation" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureMipmapBias(size_t unit, float bias)
    {
        if (mCurrentCapabilities->hasCapability(RSC_MIPMAP_LOD_BIAS))
        {
            // ugh - have to pass float data through DWORD with no conversion
            HRESULT hr = __SetSamplerState(getSamplerId(unit), D3DSAMP_MIPMAPLODBIAS, 
                *(DWORD*)&bias);
            if(FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture mipmap bias", 
                "D3D9RenderSystem::_setTextureMipmapBias" );

        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureMatrix( size_t stage, const Matrix4& xForm )
    {
        HRESULT hr;
        D3DXMATRIX d3dMat; // the matrix we'll maybe apply
        Matrix4 newMat = xForm; // the matrix we'll apply after conv. to D3D format
        // Cache texcoord calc method to register
        TexCoordCalcMethod autoTexCoordType = mTexStageDesc[stage].autoTexCoordType;

        // if a vertex program is bound, we mustn't set texture transforms
        if (mVertexProgramBound)
        {
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
            if( FAILED( hr ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to disable texture coordinate transform", "D3D9RenderSystem::_setTextureMatrix" );
            return;
        }


        if (autoTexCoordType == TEXCALC_ENVIRONMENT_MAP)
        {
            if (mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().VertexProcessingCaps & D3DVTXPCAPS_TEXGEN_SPHEREMAP)
            {
                /** Invert the texture for the spheremap */
                Matrix4 ogreMatEnvMap = Matrix4::IDENTITY;
                // set env_map values
                ogreMatEnvMap[1][1] = -1.0f;
                // concatenate with the xForm
                newMat = newMat.concatenate(ogreMatEnvMap);
            }
            else
            {
                /* If envmap is applied, but device doesn't support spheremap,
                then we have to use texture transform to make the camera space normal
                reference the envmap properly. This isn't exactly the same as spheremap
                (it looks nasty on flat areas because the camera space normals are the same)
                but it's the best approximation we have in the absence of a proper spheremap */
                // concatenate with the xForm
                newMat = newMat.concatenate(Matrix4::CLIPSPACE2DTOIMAGESPACE);
            }
        }

        // If this is a cubic reflection, we need to modify using the view matrix
        if (autoTexCoordType == TEXCALC_ENVIRONMENT_MAP_REFLECTION)
        {
            // Get transposed 3x3
            // We want to transpose since that will invert an orthonormal matrix ie rotation
            Matrix4 ogreViewTransposed;
            ogreViewTransposed[0][0] = mViewMatrix[0][0];
            ogreViewTransposed[0][1] = mViewMatrix[1][0];
            ogreViewTransposed[0][2] = mViewMatrix[2][0];
            ogreViewTransposed[0][3] = 0.0f;

            ogreViewTransposed[1][0] = mViewMatrix[0][1];
            ogreViewTransposed[1][1] = mViewMatrix[1][1];
            ogreViewTransposed[1][2] = mViewMatrix[2][1];
            ogreViewTransposed[1][3] = 0.0f;

            ogreViewTransposed[2][0] = mViewMatrix[0][2];
            ogreViewTransposed[2][1] = mViewMatrix[1][2];
            ogreViewTransposed[2][2] = mViewMatrix[2][2];
            ogreViewTransposed[2][3] = 0.0f;

            ogreViewTransposed[3][0] = 0.0f;
            ogreViewTransposed[3][1] = 0.0f;
            ogreViewTransposed[3][2] = 0.0f;
            ogreViewTransposed[3][3] = 1.0f;

            newMat = newMat.concatenate(ogreViewTransposed);
        }

        if (autoTexCoordType == TEXCALC_PROJECTIVE_TEXTURE)
        {
            // Derive camera space to projector space transform
            // To do this, we need to undo the camera view matrix, then 
            // apply the projector view & projection matrices
            newMat = mViewMatrix.inverse();
            if(mTexProjRelative)
            {
                Matrix4 viewMatrix;
                mTexStageDesc[stage].frustum->calcViewMatrixRelative(mTexProjRelativeOrigin, viewMatrix);
                newMat = viewMatrix * newMat;
            }
            else
            {
                newMat = mTexStageDesc[stage].frustum->getViewMatrix() * newMat;
            }
            newMat = mTexStageDesc[stage].frustum->getProjectionMatrix() * newMat;
            newMat = Matrix4::CLIPSPACE2DTOIMAGESPACE * newMat;
            newMat = xForm * newMat;
        }

        // need this if texture is a cube map, to invert D3D's z coord
        if (autoTexCoordType != TEXCALC_NONE &&
            autoTexCoordType != TEXCALC_PROJECTIVE_TEXTURE)
        {
            newMat[2][0] = -newMat[2][0];
            newMat[2][1] = -newMat[2][1];
            newMat[2][2] = -newMat[2][2];
            newMat[2][3] = -newMat[2][3];
        }

        // convert our matrix to D3D format
        d3dMat = D3D9Mappings::makeD3DXMatrix(newMat);

        // set the matrix if it's not the identity
        if (!D3DXMatrixIsIdentity(&d3dMat))
        {
            /* It's seems D3D automatically add a texture coordinate with value 1,
            and fill up the remaining texture coordinates with 0 for the input
            texture coordinates before pass to texture coordinate transformation.

            NOTE: It's difference with D3DDECLTYPE enumerated type expand in
            DirectX SDK documentation!

            So we should prepare the texcoord transform, make the transformation
            just like standardized vector expand, thus, fill w with value 1 and
            others with 0.
            */
            if (autoTexCoordType == TEXCALC_NONE)
            {
                /* FIXME: The actually input texture coordinate dimensions should
                be determine by texture coordinate vertex element. Now, just trust
                user supplied texture type matches texture coordinate vertex element.
                */
                if (mTexStageDesc[stage].texType == D3D9Mappings::D3D_TEX_TYPE_NORMAL)
                {
                    /* It's 2D input texture coordinate:

                    texcoord in vertex buffer     D3D expanded to     We are adjusted to
                    -->                 -->
                    (u, v)               (u, v, 1, 0)          (u, v, 0, 1)
                    */
                    std::swap(d3dMat._31, d3dMat._41);
                    std::swap(d3dMat._32, d3dMat._42);
                    std::swap(d3dMat._33, d3dMat._43);
                    std::swap(d3dMat._34, d3dMat._44);
                }
            }
            else
            {
                // All texgen generate 3D input texture coordinates.
            }

            // tell D3D the dimension of tex. coord.
            int texCoordDim = D3DTTFF_COUNT2;
            if (mTexStageDesc[stage].autoTexCoordType == TEXCALC_PROJECTIVE_TEXTURE)
            {
                /* We want texcoords (u, v, w, q) always get divided by q, but D3D
                projected texcoords is divided by the last element (in the case of
                2D texcoord, is w). So we tweak the transform matrix, transform the
                texcoords with w and q swapped: (u, v, q, w), and then D3D will
                divide u, v by q. The w and q just ignored as it wasn't used by
                rasterizer.
                */
                switch (mTexStageDesc[stage].texType)
                {
                case D3D9Mappings::D3D_TEX_TYPE_NORMAL:
                    std::swap(d3dMat._13, d3dMat._14);
                    std::swap(d3dMat._23, d3dMat._24);
                    std::swap(d3dMat._33, d3dMat._34);
                    std::swap(d3dMat._43, d3dMat._44);

                    texCoordDim = D3DTTFF_PROJECTED | D3DTTFF_COUNT3;
                    break;

                case D3D9Mappings::D3D_TEX_TYPE_CUBE:
                case D3D9Mappings::D3D_TEX_TYPE_VOLUME:
                    // Yes, we support 3D projective texture.
                    texCoordDim = D3DTTFF_PROJECTED | D3DTTFF_COUNT4;
                    break;
                }
            }
            else
            {
                switch (mTexStageDesc[stage].texType)
                {
                case D3D9Mappings::D3D_TEX_TYPE_NORMAL:
                    texCoordDim = D3DTTFF_COUNT2;
                    break;
                case D3D9Mappings::D3D_TEX_TYPE_CUBE:
                case D3D9Mappings::D3D_TEX_TYPE_VOLUME:
                    texCoordDim = D3DTTFF_COUNT3;
                    break;
                }
            }

            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXTURETRANSFORMFLAGS, texCoordDim );
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture coord. dimension", "D3D9RenderSystem::_setTextureMatrix" );

            hr = getActiveD3D9Device()->SetTransform( (D3DTRANSFORMSTATETYPE)(D3DTS_TEXTURE0 + stage), &d3dMat );
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set texture matrix", "D3D9RenderSystem::_setTextureMatrix" );
        }
        else
        {
            // disable all of this
            hr = __SetTextureStageState( static_cast<DWORD>(stage), D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE );
            if( FAILED( hr ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to disable texture coordinate transform", "D3D9RenderSystem::_setTextureMatrix" );

            // Needless to sets texture transform here, it's never used at all
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureAddressingMode( size_t stage, 
        const TextureUnitState::UVWAddressingMode& uvw )
    {
        HRESULT hr;
        if( FAILED( hr = __SetSamplerState( getSamplerId(stage), D3DSAMP_ADDRESSU, D3D9Mappings::get(uvw.u, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set texture addressing mode for U", "D3D9RenderSystem::_setTextureAddressingMode" );
        if( FAILED( hr = __SetSamplerState( getSamplerId(stage), D3DSAMP_ADDRESSV, D3D9Mappings::get(uvw.v, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set texture addressing mode for V", "D3D9RenderSystem::_setTextureAddressingMode" );
        if( FAILED( hr = __SetSamplerState( getSamplerId(stage), D3DSAMP_ADDRESSW, D3D9Mappings::get(uvw.w, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set texture addressing mode for W", "D3D9RenderSystem::_setTextureAddressingMode" );
    }
    //-----------------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureBorderColour(size_t stage,
        const ColourValue& colour)
    {
        HRESULT hr;
        if( FAILED( hr = __SetSamplerState( getSamplerId(stage), D3DSAMP_BORDERCOLOR, colour.getAsARGB()) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set texture border colour", "D3D9RenderSystem::_setTextureBorderColour" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureBlendMode( size_t stage, const LayerBlendModeEx& bm )
    {
        HRESULT hr = S_OK;
        D3DTEXTURESTAGESTATETYPE tss;
        D3DCOLOR manualD3D;

        // choose type of blend.
        if( bm.blendType == LBT_COLOUR )
            tss = D3DTSS_COLOROP;
        else if( bm.blendType == LBT_ALPHA )
            tss = D3DTSS_ALPHAOP;
        else
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
            "Invalid blend type", "D3D9RenderSystem::_setTextureBlendMode");

        // set manual factor if required by operation
        if (bm.operation == LBX_BLEND_MANUAL)
        {
            hr = __SetRenderState( D3DRS_TEXTUREFACTOR, D3DXCOLOR(0.0, 0.0, 0.0,  bm.factor) );
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
        }
        // set operation
        hr = __SetTextureStageState( static_cast<DWORD>(stage), tss, D3D9Mappings::get(bm.operation, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps()) );
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set operation", "D3D9RenderSystem::_setTextureBlendMode" );

        // choose source 1
        if( bm.blendType == LBT_COLOUR )
        {
            tss = D3DTSS_COLORARG1;
            manualD3D = D3DXCOLOR( bm.colourArg1.r, bm.colourArg1.g, bm.colourArg1.b, bm.colourArg1.a );
            mManualBlendColours[stage][0] = bm.colourArg1;
        }
        else if( bm.blendType == LBT_ALPHA )
        {
            tss = D3DTSS_ALPHAARG1;
            manualD3D = D3DXCOLOR( mManualBlendColours[stage][0].r, 
                mManualBlendColours[stage][0].g, 
                mManualBlendColours[stage][0].b, bm.alphaArg1 );
        }
        else
        {
            OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, 
                "Invalid blend type", "D3D9RenderSystem::_setTextureBlendMode");
        }
        // Set manual factor if required
        if (bm.source1 == LBS_MANUAL)
        {
            if (mCurrentCapabilities->hasCapability(RSC_PERSTAGECONSTANT))
            {
                // Per-stage state
                hr = __SetTextureStageState(static_cast<DWORD>(stage), D3DTSS_CONSTANT, manualD3D);
            }
            else
            {
                // Global state
                hr = __SetRenderState( D3DRS_TEXTUREFACTOR, manualD3D );
            }
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
        }
        // set source 1
        hr = __SetTextureStageState( static_cast<DWORD>(stage), tss, D3D9Mappings::get(bm.source1, mCurrentCapabilities->hasCapability(RSC_PERSTAGECONSTANT)) );
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set source1", "D3D9RenderSystem::_setTextureBlendMode" );

        // choose source 2
        if( bm.blendType == LBT_COLOUR )
        {
            tss = D3DTSS_COLORARG2;
            manualD3D = D3DXCOLOR( bm.colourArg2.r, bm.colourArg2.g, bm.colourArg2.b, bm.colourArg2.a );
            mManualBlendColours[stage][1] = bm.colourArg2;
        }
        else if( bm.blendType == LBT_ALPHA )
        {
            tss = D3DTSS_ALPHAARG2;
            manualD3D = D3DXCOLOR( mManualBlendColours[stage][1].r, 
                mManualBlendColours[stage][1].g, 
                mManualBlendColours[stage][1].b, 
                bm.alphaArg2 );
        }
        // Set manual factor if required
        if (bm.source2 == LBS_MANUAL)
        {
            if (mCurrentCapabilities->hasCapability(RSC_PERSTAGECONSTANT))
            {
                // Per-stage state
                hr = __SetTextureStageState(static_cast<DWORD>(stage), D3DTSS_CONSTANT, manualD3D);
            }
            else
            {
                hr = __SetRenderState( D3DRS_TEXTUREFACTOR, manualD3D );
            }
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set manual factor", "D3D9RenderSystem::_setTextureBlendMode" );
        }
        // Now set source 2
        hr = __SetTextureStageState( static_cast<DWORD>(stage), tss, D3D9Mappings::get(bm.source2, mCurrentCapabilities->hasCapability(RSC_PERSTAGECONSTANT)) );
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set source 2", "D3D9RenderSystem::_setTextureBlendMode" );

        // Set interpolation factor if lerping
        if (bm.operation == LBX_BLEND_DIFFUSE_COLOUR && 
            mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().TextureOpCaps & D3DTEXOPCAPS_LERP)
        {
            // choose source 0 (lerp factor)
            if( bm.blendType == LBT_COLOUR )
            {
                tss = D3DTSS_COLORARG0;
            }
            else if( bm.blendType == LBT_ALPHA )
            {
                tss = D3DTSS_ALPHAARG0;
            }
            hr = __SetTextureStageState(static_cast<DWORD>(stage), tss, D3DTA_DIFFUSE);

            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set lerp source 0", 
                "D3D9RenderSystem::_setTextureBlendMode" );

        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendOperation op )
    {
        HRESULT hr;
        if( sourceFactor == SBF_ONE && destFactor == SBF_ZERO)
        {
            if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D9RenderSystem::_setSceneBlending" );
        }
        else
        {
            if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D9RenderSystem::_setSceneBlending" );
            if (FAILED(hr = __SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set separate alpha blending option", "D3D9RenderSystem::_setSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_SRCBLEND, D3D9Mappings::get(sourceFactor) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set source blend", "D3D9RenderSystem::_setSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_DESTBLEND, D3D9Mappings::get(destFactor) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set destination blend", "D3D9RenderSystem::_setSceneBlending" );
        }

        if (FAILED(hr = __SetRenderState(D3DRS_BLENDOP, D3D9Mappings::get(op))))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set scene blending operation option", "D3D9RenderSystem::_setSceneBlendingOperation" );
        if (FAILED(hr = __SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mappings::get(op))))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set scene blending operation option", "D3D9RenderSystem::_setSceneBlendingOperation" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setSeparateSceneBlending( SceneBlendFactor sourceFactor, SceneBlendFactor destFactor, SceneBlendFactor sourceFactorAlpha, 
        SceneBlendFactor destFactorAlpha, SceneBlendOperation op, SceneBlendOperation alphaOp )
    {
        HRESULT hr;
        if( sourceFactor == SBF_ONE && destFactor == SBF_ZERO && 
            sourceFactorAlpha == SBF_ONE && destFactorAlpha == SBF_ZERO)
        {
            if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D9RenderSystem::_setSceneBlending" );
        }
        else
        {
            if (FAILED(hr = __SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha blending option", "D3D9RenderSystem::_setSeperateSceneBlending" );
            if (FAILED(hr = __SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE)))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set separate alpha blending option", "D3D9RenderSystem::_setSeperateSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_SRCBLEND, D3D9Mappings::get(sourceFactor) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set source blend", "D3D9RenderSystem::_setSeperateSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_DESTBLEND, D3D9Mappings::get(destFactor) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set destination blend", "D3D9RenderSystem::_setSeperateSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_SRCBLENDALPHA, D3D9Mappings::get(sourceFactorAlpha) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha source blend", "D3D9RenderSystem::_setSeperateSceneBlending" );
            if( FAILED( hr = __SetRenderState( D3DRS_DESTBLENDALPHA, D3D9Mappings::get(destFactorAlpha) ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha destination blend", "D3D9RenderSystem::_setSeperateSceneBlending" );
        }

        if (FAILED(hr = __SetRenderState(D3DRS_BLENDOP, D3D9Mappings::get(op))))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set scene blending operation option", "D3D9RenderSystem::_setSceneBlendingOperation" );
        if (FAILED(hr = __SetRenderState(D3DRS_BLENDOPALPHA, D3D9Mappings::get(alphaOp))))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha scene blending operation option", "D3D9RenderSystem::_setSceneBlendingOperation" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setAlphaRejectSettings( CompareFunction func, unsigned char value, bool alphaToCoverage )
    {
        HRESULT hr;
        bool a2c = false;
        static bool lasta2c = false;

        if (func != CMPF_ALWAYS_PASS)
        {
            if( FAILED( hr = __SetRenderState( D3DRS_ALPHATESTENABLE,  TRUE ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to enable alpha testing", 
                "D3D9RenderSystem::_setAlphaRejectSettings" );

            a2c = alphaToCoverage;
        }
        else
        {
            if( FAILED( hr = __SetRenderState( D3DRS_ALPHATESTENABLE,  FALSE ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to disable alpha testing", 
                "D3D9RenderSystem::_setAlphaRejectSettings" );
        }
        // Set always just be sure
        if( FAILED( hr = __SetRenderState( D3DRS_ALPHAFUNC, D3D9Mappings::get(func) ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha reject function", "D3D9RenderSystem::_setAlphaRejectSettings" );
        if( FAILED( hr = __SetRenderState( D3DRS_ALPHAREF, value ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set render state D3DRS_ALPHAREF", "D3D9RenderSystem::_setAlphaRejectSettings" );

        // Alpha to coverage
        if (getCapabilities()->hasCapability(RSC_ALPHA_TO_COVERAGE))
        {
            // Vendor-specific hacks on renderstate, gotta love 'em
            if (getCapabilities()->getVendor() == GPU_NVIDIA)
            {
                if (a2c)
                {
                    if( FAILED( hr = __SetRenderState( D3DRS_ADAPTIVETESS_Y,  (D3DFORMAT)MAKEFOURCC('A', 'T', 'O', 'C') ) ) )
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha to coverage option", "D3D9RenderSystem::_setAlphaRejectSettings" );
                }
                else
                {
                    if( FAILED( hr = __SetRenderState( D3DRS_ADAPTIVETESS_Y,  D3DFMT_UNKNOWN ) ) )
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha to coverage option", "D3D9RenderSystem::_setAlphaRejectSettings" );
                }

            }
            else if ((getCapabilities()->getVendor() == GPU_AMD))
            {
                if (a2c)
                {
                    if( FAILED( hr = __SetRenderState( D3DRS_POINTSIZE,  MAKEFOURCC('A','2','M','1') ) ) )
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha to coverage option", "D3D9RenderSystem::_setAlphaRejectSettings" );
                }
                else
                {
                    // discovered this through trial and error, seems to work
                    if( FAILED( hr = __SetRenderState( D3DRS_POINTSIZE,  MAKEFOURCC('A','2','M','0') ) ) )
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set alpha to coverage option", "D3D9RenderSystem::_setAlphaRejectSettings" );
                }
            }
            // no hacks available for any other vendors?
            lasta2c = a2c;
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setCullingMode( CullingMode mode )
    {
        mCullingMode = mode;
        HRESULT hr;
        bool flip = ((mActiveRenderTarget->requiresTextureFlipping() && !mInvertVertexWinding) ||
            (!mActiveRenderTarget->requiresTextureFlipping() && mInvertVertexWinding));

        if( FAILED (hr = __SetRenderState(D3DRS_CULLMODE, 
            D3D9Mappings::get(mode, flip))) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set culling mode", "D3D9RenderSystem::_setCullingMode" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setDepthBufferParams( bool depthTest, bool depthWrite, CompareFunction depthFunction )
    {
        _setDepthBufferCheckEnabled( depthTest );
        _setDepthBufferWriteEnabled( depthWrite );
        _setDepthBufferFunction( depthFunction );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setDepthBufferCheckEnabled( bool enabled )
    {
        HRESULT hr;

        if( enabled )
        {
            // Use w-buffer if available and enabled
            if( mWBuffer && mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().RasterCaps & D3DPRASTERCAPS_WBUFFER )
                hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_USEW );
            else
                hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
        }
        else
            hr = __SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );

        if( FAILED( hr ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting depth buffer test state", "D3D9RenderSystem::_setDepthBufferCheckEnabled" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setDepthBufferWriteEnabled( bool enabled )
    {
        HRESULT hr;

        if( FAILED( hr = __SetRenderState( D3DRS_ZWRITEENABLE, enabled ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting depth buffer write state", "D3D9RenderSystem::_setDepthBufferWriteEnabled" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setDepthBufferFunction( CompareFunction func )
    {
        HRESULT hr;
        if( FAILED( hr = __SetRenderState( D3DRS_ZFUNC, D3D9Mappings::get(func) ) ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting depth buffer test function", "D3D9RenderSystem::_setDepthBufferFunction" );
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setDepthBias(float constantBias, float slopeScaleBias)
    {

        if ((mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().RasterCaps & D3DPRASTERCAPS_DEPTHBIAS) != 0)
        {
            // Negate bias since D3D is backward
            // D3D also expresses the constant bias as an absolute value, rather than 
            // relative to minimum depth unit, so scale to fit
            constantBias = -constantBias / 250000.0f;
            HRESULT hr = __SetRenderState(D3DRS_DEPTHBIAS, FLOAT2DWORD(constantBias));
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting constant depth bias", 
                "D3D9RenderSystem::_setDepthBias");
        }

        if ((mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().RasterCaps & D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS) != 0)
        {
            // Negate bias since D3D is backward
            slopeScaleBias = -slopeScaleBias;
            HRESULT hr = __SetRenderState(D3DRS_SLOPESCALEDEPTHBIAS, FLOAT2DWORD(slopeScaleBias));
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting slope scale depth bias", 
                "D3D9RenderSystem::_setDepthBias");
        }


    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setColourBufferWriteEnabled(bool red, bool green, 
        bool blue, bool alpha)
    {
        DWORD val = 0;
        if (red) 
            val |= D3DCOLORWRITEENABLE_RED;
        if (green)
            val |= D3DCOLORWRITEENABLE_GREEN;
        if (blue)
            val |= D3DCOLORWRITEENABLE_BLUE;
        if (alpha)
            val |= D3DCOLORWRITEENABLE_ALPHA;
        HRESULT hr = __SetRenderState(D3DRS_COLORWRITEENABLE, val); 
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting colour write enable flags", 
            "D3D9RenderSystem::_setColourBufferWriteEnabled");
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setPolygonMode(PolygonMode level)
    {
        HRESULT hr = __SetRenderState(D3DRS_FILLMODE, D3D9Mappings::get(level));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting polygon mode.", "D3D9RenderSystem::setPolygonMode");
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setStencilCheckEnabled(bool enabled)
    {
        // Allow stencilling
        HRESULT hr = __SetRenderState(D3DRS_STENCILENABLE, enabled);
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error enabling / disabling stencilling.",
            "D3D9RenderSystem::setStencilCheckEnabled");
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setStencilBufferParams(CompareFunction func, 
        uint32 refValue, uint32 compareMask, uint32 writeMask, StencilOperation stencilFailOp, 
        StencilOperation depthFailOp, StencilOperation passOp, 
        bool twoSidedOperation, bool readBackAsTexture)
    {
        HRESULT hr;
        bool flip;

        // 2-sided operation
        if (twoSidedOperation)
        {
            if (!mCurrentCapabilities->hasCapability(RSC_TWO_SIDED_STENCIL))
                OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "2-sided stencils are not supported",
                "D3D9RenderSystem::setStencilBufferParams");
            hr = __SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting 2-sided stencil mode.",
                "D3D9RenderSystem::setStencilBufferParams");
            // NB: We should always treat CCW as front face for consistent with default
            // culling mode. Therefore, we must take care with two-sided stencil settings.
            flip = (mInvertVertexWinding && mActiveRenderTarget->requiresTextureFlipping()) ||
                (!mInvertVertexWinding && !mActiveRenderTarget->requiresTextureFlipping());

            // Set alternative versions of ops
            // fail op
            hr = __SetRenderState(D3DRS_CCW_STENCILFAIL, D3D9Mappings::get(stencilFailOp, !flip));
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil fail operation (2-sided).",
                "D3D9RenderSystem::setStencilBufferParams");

            // depth fail op
            hr = __SetRenderState(D3DRS_CCW_STENCILZFAIL, D3D9Mappings::get(depthFailOp, !flip));
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil depth fail operation (2-sided).",
                "D3D9RenderSystem::setStencilBufferParams");

            // pass op
            hr = __SetRenderState(D3DRS_CCW_STENCILPASS, D3D9Mappings::get(passOp, !flip));
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil pass operation (2-sided).",
                "D3D9RenderSystem::setStencilBufferParams");
        }
        else
        {
            hr = __SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
            if (FAILED(hr))
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting 1-sided stencil mode.",
                "D3D9RenderSystem::setStencilBufferParams");
            flip = false;
        }

        // func
        hr = __SetRenderState(D3DRS_STENCILFUNC, D3D9Mappings::get(func));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil buffer test function.",
            "D3D9RenderSystem::setStencilBufferParams");

        // reference value
        hr = __SetRenderState(D3DRS_STENCILREF, refValue);
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil buffer reference value.",
            "D3D9RenderSystem::setStencilBufferParams");

        // compare mask
        hr = __SetRenderState(D3DRS_STENCILMASK, compareMask);
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil buffer compare mask.",
            "D3D9RenderSystem::setStencilBufferParams");

        // compare mask
        hr = __SetRenderState(D3DRS_STENCILWRITEMASK, writeMask);
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil buffer write mask.",
            "D3D9RenderSystem::setStencilBufferParams");

        // fail op
        hr = __SetRenderState(D3DRS_STENCILFAIL, D3D9Mappings::get(stencilFailOp, flip));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil fail operation.",
            "D3D9RenderSystem::setStencilBufferParams");

        // depth fail op
        hr = __SetRenderState(D3DRS_STENCILZFAIL, D3D9Mappings::get(depthFailOp, flip));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil depth fail operation.",
            "D3D9RenderSystem::setStencilBufferParams");

        // pass op
        hr = __SetRenderState(D3DRS_STENCILPASS, D3D9Mappings::get(passOp, flip));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error setting stencil pass operation.",
            "D3D9RenderSystem::setStencilBufferParams");
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureUnitFiltering(size_t unit, FilterType ftype, 
        FilterOptions filter)
    {
        HRESULT hr;
        D3D9Mappings::eD3DTexType texType = mTexStageDesc[unit].texType;
        hr = __SetSamplerState( getSamplerId(unit), D3D9Mappings::get(ftype), 
            D3D9Mappings::get(ftype, filter, mDeviceManager->getActiveDevice()->getD3D9DeviceCaps(), texType));
        if (FAILED(hr))
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set texture filter ", "D3D9RenderSystem::_setTextureUnitFiltering");
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureUnitCompareFunction(size_t unit, CompareFunction function)
    {
        //no effect in directX9 rendersystem
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureUnitCompareEnabled(size_t unit, bool compare)
    {
        //no effect in directX9 rendersystem
    }
    //---------------------------------------------------------------------
    DWORD D3D9RenderSystem::_getCurrentAnisotropy(size_t unit)
    {
        DWORD oldVal;
        getActiveD3D9Device()->GetSamplerState(static_cast<DWORD>(unit), D3DSAMP_MAXANISOTROPY, &oldVal);
        return oldVal;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setTextureLayerAnisotropy(size_t unit, unsigned int maxAnisotropy)
    {
        if (static_cast<DWORD>(maxAnisotropy) > mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().MaxAnisotropy)
            maxAnisotropy = mDeviceManager->getActiveDevice()->getD3D9DeviceCaps().MaxAnisotropy;

        if (_getCurrentAnisotropy(unit) != maxAnisotropy)
            __SetSamplerState( getSamplerId(unit), D3DSAMP_MAXANISOTROPY, maxAnisotropy );
    }
    //---------------------------------------------------------------------
    HRESULT D3D9RenderSystem::__SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
    {
        HRESULT hr;
        DWORD oldVal;

        if ( FAILED( hr = getActiveD3D9Device()->GetRenderState(state, &oldVal) ) )
            return hr;
        if ( oldVal == value )
            return D3D_OK;
        else
            return getActiveD3D9Device()->SetRenderState(state, value);
    }
    //---------------------------------------------------------------------
    HRESULT D3D9RenderSystem::__SetSamplerState(DWORD sampler, D3DSAMPLERSTATETYPE type, DWORD value)
    {
        HRESULT hr;
        DWORD oldVal;

        if ( FAILED( hr = getActiveD3D9Device()->GetSamplerState(sampler, type, &oldVal) ) )
            return hr;
        if ( oldVal == value )
            return D3D_OK;
        else
            return getActiveD3D9Device()->SetSamplerState(sampler, type, value);
    }
    //---------------------------------------------------------------------
    HRESULT D3D9RenderSystem::__SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value)
    {
        HRESULT hr;
        DWORD oldVal;

        // can only set fixed-function texture stage state
        if (stage < 8)
        {
            if ( FAILED( hr = getActiveD3D9Device()->GetTextureStageState(stage, type, &oldVal) ) )
                return hr;
            if ( oldVal == value )
                return D3D_OK;
            else
                return getActiveD3D9Device()->SetTextureStageState(stage, type, value);
        }
        else
        {
            return D3D_OK;
        }
    }
    //---------------------------------------------------------------------
    DepthBuffer* D3D9RenderSystem::_createDepthBufferFor( RenderTarget *renderTarget )
    {
        IDirect3DSurface9* pBack[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
        memset (pBack, 0, sizeof(pBack) );
        renderTarget->getCustomAttribute( "DDBACKBUFFER", &pBack );
        if( !pBack[0] )
            return 0;

        D3DSURFACE_DESC srfDesc;
        if( FAILED(pBack[0]->GetDesc(&srfDesc)) )
        {
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                     "Failed to retrieve Surface Description from BackBuffer. RenderTarget: " +
                                                                            renderTarget->getName(),
                     "D3D9RenderSystem::_createDepthBufferFor" );
        }

        //Find an appropiarte format for this depth buffer that best matches the RenderTarget's
        D3DFORMAT dsfmt = _getDepthStencilFormatFor( srfDesc.Format );

        //Create the depthstencil surface
        IDirect3DSurface9 *depthBufferSurface = NULL;
        IDirect3DDevice9* activeDevice = getActiveD3D9Device();
        HRESULT hr = activeDevice->CreateDepthStencilSurface( 
                                            srfDesc.Width, srfDesc.Height, dsfmt,
                                            srfDesc.MultiSampleType, srfDesc.MultiSampleQuality, 
                                            TRUE,  // discard true or false?
                                            &depthBufferSurface, NULL);
        if( FAILED(hr) )
        {
            String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                        "Error CreateDepthStencilSurface : " + msg,
                        "D3D9RenderSystem::_createDepthBufferFor" );
        }

        D3D9DepthBuffer *newDepthBuffer = OGRE_NEW D3D9DepthBuffer( DepthBuffer::POOL_DEFAULT, this,
                                                activeDevice, depthBufferSurface,
                                                dsfmt, srfDesc.Width, srfDesc.Height,
                                                srfDesc.MultiSampleType, srfDesc.MultiSampleQuality, false );

        return newDepthBuffer;
    }

    //---------------------------------------------------------------------
    DepthBuffer* D3D9RenderSystem::_addManualDepthBuffer( IDirect3DDevice9* depthSurfaceDevice, IDirect3DSurface9 *depthSurface )
    {
        //If this depth buffer was already added, return that one
        DepthBufferVec::const_iterator itor = mDepthBufferPool[DepthBuffer::POOL_DEFAULT].begin();
        DepthBufferVec::const_iterator end  = mDepthBufferPool[DepthBuffer::POOL_DEFAULT].end();

        while( itor != end )
        {
            if( static_cast<D3D9DepthBuffer*>(*itor)->getDepthBufferSurface() == depthSurface )
                return *itor;

            ++itor;
        }

        //Nope, get the info about this depth buffer and create a new container fot it
        D3DSURFACE_DESC dsDesc;
        if( FAILED(depthSurface->GetDesc(&dsDesc)) )
            return 0;

        D3D9DepthBuffer *newDepthBuffer = OGRE_NEW D3D9DepthBuffer( DepthBuffer::POOL_DEFAULT, this,
                                                depthSurfaceDevice, depthSurface,
                                                dsDesc.Format, dsDesc.Width, dsDesc.Height,
                                                dsDesc.MultiSampleType, dsDesc.MultiSampleQuality, true );

        //Add the 'main' depth buffer to the pool
        mDepthBufferPool[newDepthBuffer->getPoolId()].push_back( newDepthBuffer );

        return newDepthBuffer;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_cleanupDepthBuffers( IDirect3DDevice9 *creator )
    {
        assert( creator );

        DepthBufferMap::iterator itMap = mDepthBufferPool.begin();
        DepthBufferMap::iterator enMap = mDepthBufferPool.end();

        while( itMap != enMap )
        {
            DepthBufferVec::iterator itor = itMap->second.begin();
            DepthBufferVec::iterator end  = itMap->second.end();

            while( itor != end )
            {
                //Only delete those who match the specified creator
                if( static_cast<D3D9DepthBuffer*>(*itor)->getDeviceCreator() == creator )
                {
                    OGRE_DELETE *itor;

                    //Erasing a vector invalidates iterators, we need to recalculate
                    //to avoid memory corruption and asserts. The new itor will point
                    //to the next iterator
                    const size_t idx = itor - itMap->second.begin();
                    itMap->second.erase( itor );    //Erase
                    itor = itMap->second.begin() + idx;
                    end  = itMap->second.end();
                }
                else
                    ++itor;
            }

            //Erase the pool if it's now empty. Note erasing from a map is
            //valid while iterating through it
            if( itMap->second.empty() )
            {
                DepthBufferMap::iterator deadi = itMap++;
                mDepthBufferPool.erase( deadi );
            }
            else
                ++itMap;
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_cleanupDepthBuffers( IDirect3DSurface9 *manualSurface )
    {
        assert( manualSurface );

        DepthBufferMap::iterator itMap = mDepthBufferPool.begin();
        DepthBufferMap::iterator enMap = mDepthBufferPool.end();

        while( itMap != enMap )
        {
            DepthBufferVec::iterator itor = itMap->second.begin();
            DepthBufferVec::iterator end  = itMap->second.end();

            while( itor != end )
            {
                //Only delete those who match the specified surface
                if( static_cast<D3D9DepthBuffer*>(*itor)->getDepthBufferSurface() == manualSurface )
                {
                    OGRE_DELETE *itor;

                    //Erasing a vector invalidates iterators, we need to recalculate
                    //to avoid memory corruption and asserts. The new itor will point
                    //to the next iterator
                    const size_t idx = itor - itMap->second.begin();
                    itMap->second.erase( itor );    //Erase
                    itor = itMap->second.begin() + idx;
                    end  = itMap->second.end();
                }
                else
                    ++itor;
            }

            //Erase the pool if it's now empty. Note erasing from a map is
            //valid while iterating through it
            if( itMap->second.empty() )
            {
                DepthBufferMap::iterator deadi = itMap++;
                mDepthBufferPool.erase( deadi );
            }
            else
                ++itMap;
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_setRenderTarget(RenderTarget *target)
    {
        mActiveRenderTarget = target;

        if (mActiveRenderTarget)
        {
            HRESULT hr;

            // If this is called without going through RenderWindow::update, then 
            // the device will not have been set. Calling it twice is safe, the 
            // implementation ensures nothing happens if the same device is set twice
            if (std::find(mRenderWindows.begin(), mRenderWindows.end(), target) != mRenderWindows.end())
            {
                D3D9RenderWindow *window = static_cast<D3D9RenderWindow*>(target);
                mDeviceManager->setActiveRenderTargetDevice(window->getDevice());
                // also make sure we validate the device; if this never went 
                // through update() it won't be set
                window->_validateDevice();

#if OGRE_NO_QUAD_BUFFER_STEREO == 0
                window->_validateStereo();
#endif
            }

            // Retrieve render surfaces (up to OGRE_MAX_MULTIPLE_RENDER_TARGETS)
            IDirect3DSurface9* pBack[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
            memset(pBack, 0, sizeof(pBack));
            target->getCustomAttribute( "DDBACKBUFFER", &pBack );
            if (!pBack[0])
                return;

            IDirect3DDevice9* activeDevice = getActiveD3D9Device();
            D3D9DepthBuffer *depthBuffer = static_cast<D3D9DepthBuffer*>(target->getDepthBuffer());

            if( target->getDepthBufferPool() != DepthBuffer::POOL_NO_DEPTH &&
                (!depthBuffer || depthBuffer->getDeviceCreator() != activeDevice ) )
            {
                //Depth is automatically managed and there is no depth buffer attached to this RT
                //or the Current D3D device doesn't match the one this Depth buffer was created
                setDepthBufferFor( target );

                //Retrieve depth buffer again
                depthBuffer = static_cast<D3D9DepthBuffer*>(target->getDepthBuffer());
            }

            if ((depthBuffer != NULL) && ( depthBuffer->getDeviceCreator() != activeDevice))
            {
                OGRE_EXCEPT( Exception::ERR_RENDERINGAPI_ERROR,
                    "Can't use a depth buffer from a different device!",
                    "D3D9RenderSystem::_setRenderTarget" );
            }

            IDirect3DSurface9 *depthSurface = depthBuffer ? depthBuffer->getDepthBufferSurface() : NULL;

            // create the list of old render targets.
            // The list of old render targets is needed so that we can avoid trying to bind a render target
            // to a slot while it is already bound to another slot from before.  Doing that would fail.
            //
            // NOTE:  pOldRenderTargets[0] is NEVER set!!!
            // We don't need it, so we don't waste time looking it up.
			IDirect3DSurface9* pOldRenderTargets[OGRE_MAX_MULTIPLE_RENDER_TARGETS];
			memset(pOldRenderTargets, 0, sizeof(pOldRenderTargets));
			uint maxRenderTargetCount = mCurrentCapabilities->getNumMultiRenderTargets();
            uint oldRenderTargetCount = 1;
			for (uint i = 1; i < maxRenderTargetCount; ++i)
			{
				hr = activeDevice->GetRenderTarget(i, &pOldRenderTargets[ i ]);
                if (hr == D3D_OK)
                {
                    // GetRenderTarget bumps the reference count, so need to release to avoid a resource leak
                    pOldRenderTargets[ i ]->Release();
                    oldRenderTargetCount = i + 1;
                }
                else if (hr == D3DERR_NOTFOUND)
                {
                    // exit at the first "NOTFOUND"
                    // assumption: render targets must be contiguous
                    break;
                }
				else if (FAILED(hr))
				{
					String msg = DXGetErrorDescription(hr);
					OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to GetRenderTarget : " + msg, "D3D9RenderSystem::_setRenderTarget" );
				}
			}
			// Bind render targets
			for (uint iRt = 0; iRt < maxRenderTargetCount; ++iRt)
			{
                IDirect3DSurface9* rt = pBack[ iRt ];
                // if new render target differs from what is already there,
                if ( rt != pOldRenderTargets[ iRt ] )   // NOTE: always true when iRt == 0
                {
                    // check that the new render target isn't occupying a slot from before, and if it is, clear out the previous slot.
                    // Otherwise, we could end up trying to set the same render target in 2 different slots which will fail.
                    for (uint iOldRt = iRt + 1; iOldRt < oldRenderTargetCount; ++iOldRt)
                    {
                        // if it is (rare case),
                        if ( rt == pOldRenderTargets[ iOldRt ] )
                        {
                            // clear it out of the old slot, so that we can successfully put it in its new slot
                            hr = activeDevice->SetRenderTarget( iOldRt, NULL );
                            if (FAILED(hr))
                            {
                                String msg = DXGetErrorDescription(hr);
                                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to SetRenderTarget(NULL) : " + msg, "D3D9RenderSystem::_setRenderTarget" );
                            }
                        }
                    }
                    hr = activeDevice->SetRenderTarget( iRt, rt );
                    if (FAILED(hr))
                    {
                        String msg = DXGetErrorDescription(hr);
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to SetRenderTarget : " + msg, "D3D9RenderSystem::_setRenderTarget" );
                    }
                }
			}
			hr = activeDevice->SetDepthStencilSurface( depthSurface );
			if (FAILED(hr))
			{
				String msg = DXGetErrorDescription(hr);
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to setDepthStencil : " + msg, "D3D9RenderSystem::_setRenderTarget" );
			}
		}
	}
	//---------------------------------------------------------------------
    void D3D9RenderSystem::_setViewport( Viewport *vp )
    {
        if (!vp)
        {
            mActiveViewport = NULL;
            _setRenderTarget(NULL);
        }
        else if( vp != mActiveViewport || vp->_isUpdated() )
        {
            mActiveViewport = vp;

            // ok, it's different, time to set render target and viewport params
            D3DVIEWPORT9 d3dvp;
            HRESULT hr;

            // Set render target
            RenderTarget* target = vp->getTarget();
            _setRenderTarget(target);

            //Reset the viewport after the render target has been set. If the device
            //had been reset the viewport would have been set to NULL.
            mActiveViewport = vp;

            _setCullingMode( mCullingMode );

            // set viewport dimensions
            d3dvp.X = vp->getActualLeft();
            d3dvp.Y = vp->getActualTop();
            d3dvp.Width = vp->getActualWidth();
            d3dvp.Height = vp->getActualHeight();
            if (target->requiresTextureFlipping())
            {
                // Convert "top-left" to "bottom-left"
                d3dvp.Y = target->getHeight() - d3dvp.Height - d3dvp.Y;
            }

            // Z-values from 0.0 to 1.0 (TODO: standardise with OpenGL)
            d3dvp.MinZ = 0.0f;
            d3dvp.MaxZ = 1.0f;

            if( FAILED( hr = getActiveD3D9Device()->SetViewport( &d3dvp ) ) )
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set viewport.", "D3D9RenderSystem::_setViewport" );

            // Set sRGB write mode
            __SetRenderState(D3DRS_SRGBWRITEENABLE, target->isHardwareGammaEnabled());

			if (FAILED(hr = __SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE)))
			{
				OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to enable scissor rendering state; " + getErrorDescription(hr),
					"D3D9RenderSystem::setScissorTest");
			}

			setScissorTest(true, vp->getScissorActualLeft(), vp->getScissorActualTop(),
				vp->getScissorActualTop() + vp->getScissorActualWidth(), vp->getScissorActualLeft() + vp->getScissorActualWidth());

            vp->_clearUpdatedFlag();
        }
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setHlmsMacroblock(const HlmsMacroblock *macroblock)
	{
		_setDepthBufferCheckEnabled(macroblock->mDepthCheck);
		_setDepthBufferWriteEnabled(macroblock->mDepthWrite);
		_setDepthBufferFunction(macroblock->mDepthFunc);

		_setDepthBias(macroblock->mDepthBiasConstant, macroblock->mDepthBiasSlopeScale);
		_setCullingMode(macroblock->mCullMode);

		if (macroblock->mAlphaToCoverageEnabled)
		{
			_setAlphaRejectSettings(CMPF_GREATER_EQUAL, (DWORD)0x00000001, true);
		}
		else
		{
			_setAlphaRejectSettings(CMPF_GREATER_EQUAL, (DWORD)0x00000001, false);
		}

		if (macroblock->mScissorTestEnabled)
		{
			setScissorTest(true, mActiveViewport->getScissorActualLeft(), mActiveViewport->getScissorActualTop(),
				mActiveViewport->getScissorActualTop() + mActiveViewport->getScissorActualWidth(),
				mActiveViewport->getScissorActualLeft() + mActiveViewport->getScissorActualWidth());
		}
		else
		{
			setScissorTest(false);
		}

		_setDepthBufferWriteEnabled(macroblock->mDepthWrite);
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setHlmsBlendblock(const HlmsBlendblock *blendblock)
	{
		if (blendblock->mSeparateBlend)
		{
			_setSeparateSceneBlending(
				blendblock->mSourceBlendFactor, blendblock->mDestBlendFactor,
				blendblock->mSourceBlendFactorAlpha, blendblock->mDestBlendFactorAlpha,
				blendblock->mBlendOperation, blendblock->mBlendOperationAlpha);
		}
		else
		{
			_setSceneBlending(blendblock->mSourceBlendFactor, blendblock->mDestBlendFactor,
				blendblock->mBlendOperation);
		}
	}
	//---------------------------------------------------------------------
	void D3D9RenderSystem::_setProgramsFromHlms(const HlmsCache *hlmsCache)
	{
		unbindGpuProgram(GPT_VERTEX_PROGRAM);
		unbindGpuProgram(GPT_FRAGMENT_PROGRAM);

		GpuProgram* vertexPrgm = hlmsCache->vertexShader.get();
		GpuProgram* fragmentPrgm = hlmsCache->pixelShader.get();

		if (vertexPrgm)
		{
			bindGpuProgram(vertexPrgm);
		}

		if (fragmentPrgm)
		{
			bindGpuProgram(fragmentPrgm);
		}
	}
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_beginFrame()
    {
        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->BeginScene() ) )
        {
            String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error beginning frame :" + msg, "D3D9RenderSystem::_beginFrame" );
        }

        mLastVertexSourceCount = 0;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_endFrame()
    {
        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->EndScene() ) )
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error ending frame", "D3D9RenderSystem::_endFrame" );

        mDeviceManager->getActiveDevice()->clearDeviceStreams();

        mDeviceManager->destroyInactiveRenderDevices();
    }
    //---------------------------------------------------------------------
    struct D3D9RenderContext : public RenderSystem::RenderSystemContext
    {
        RenderTarget* target;
    };
    //---------------------------------------------------------------------
    RenderSystem::RenderSystemContext* D3D9RenderSystem::_pauseFrame(void)
    {
        //Stop rendering
        _endFrame();

        D3D9RenderContext* context = OGRE_ALLOC_T(D3D9RenderContext, 1, MEMCATEGORY_RENDERSYS);
        context->target = mActiveRenderTarget;
        
        
        return context;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_resumeFrame(RenderSystemContext* context)
    {
        //Resume rendering
        _beginFrame();
        D3D9RenderContext* d3dContext = static_cast<D3D9RenderContext*>(context);

        OGRE_FREE(context, MEMCATEGORY_RENDERSYS);
    }
    void D3D9RenderSystem::setVertexDeclaration(VertexDeclaration* decl)
    {
        setVertexDeclaration(decl, true);
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setVertexDeclaration(VertexDeclaration* decl, bool useGlobalInstancingVertexBufferIsAvailable)
    {
        HRESULT hr;

        D3D9VertexDeclaration* d3ddecl = 
            static_cast<D3D9VertexDeclaration*>(decl);

        if (FAILED(hr = getActiveD3D9Device()->SetVertexDeclaration(d3ddecl->getD3DVertexDeclaration(getGlobalInstanceVertexBufferVertexDeclaration(), useGlobalInstancingVertexBufferIsAvailable))))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D9 vertex declaration", 
                "D3D9RenderSystem::setVertexDeclaration");
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setVertexBufferBinding(VertexBufferBinding* binding)
    {
        setVertexBufferBinding(binding, 1, true, false);
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setVertexBufferBinding(
        VertexBufferBinding* binding, size_t numberOfInstances, bool useGlobalInstancingVertexBufferIsAvailable, bool indexesUsed)
    {
        /*if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Null program bound.",
                "D3D9RenderSystem::bindGpuProgram");
        }*/

        HRESULT hr;

        if (useGlobalInstancingVertexBufferIsAvailable)
        {
            numberOfInstances *= getGlobalNumberOfInstances();
        }
        
        HardwareVertexBufferSharedPtr globalInstanceVertexBuffer = getGlobalInstanceVertexBuffer();
        VertexDeclaration* globalVertexDeclaration = getGlobalInstanceVertexBufferVertexDeclaration();
        bool hasInstanceData = useGlobalInstancingVertexBufferIsAvailable &&
                    !globalInstanceVertexBuffer.isNull() && globalVertexDeclaration != NULL 
                || binding->getHasInstanceData();


        // TODO: attempt to detect duplicates
        const VertexBufferBinding::VertexBufferBindingMap& binds = binding->getBindings();
        VertexBufferBinding::VertexBufferBindingMap::const_iterator i, iend;
        size_t source = 0;
        iend = binds.end();
        for (i = binds.begin(); i != iend; ++i, ++source)
        {
            D3D9HardwareVertexBuffer* d3d9buf = 
                static_cast<D3D9HardwareVertexBuffer*>(i->second.get());

            // Unbind gap sources
            for ( ; source < i->first; ++source)
            {
                hr = getActiveD3D9Device()->SetStreamSource(static_cast<UINT>(source), NULL, 0, 0);
                if (FAILED(hr))
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to reset unused D3D9 stream source", 
                        "D3D9RenderSystem::setVertexBufferBinding");
                }
            }

            hr = getActiveD3D9Device()->SetStreamSource(
                    static_cast<UINT>(source),
                    d3d9buf->getD3D9VertexBuffer(),
                    0, // no stream offset, this is handled in _render instead
                    static_cast<UINT>(d3d9buf->getVertexSize()) // stride
                    );

            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D9 stream source for buffer binding", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }

            // SetStreamSourceFreq
            if ( hasInstanceData ) 
            {
                if ( d3d9buf->getIsInstanceData() )
                {
                    hr = getActiveD3D9Device()->SetStreamSourceFreq( static_cast<UINT>(source), D3DSTREAMSOURCE_INSTANCEDATA | d3d9buf->getInstanceDataStepRate() );
                }
                else
                {
                    if ( !indexesUsed )
                    {
                        OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Instance data used without index data.", 
                            "D3D9RenderSystem::setVertexBufferBinding");
                    }
                    hr = getActiveD3D9Device()->SetStreamSourceFreq( static_cast<UINT>(source), D3DSTREAMSOURCE_INDEXEDDATA | numberOfInstances );
                }
                if (FAILED(hr))
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D9 stream source Freq", 
                        "D3D9RenderSystem::setVertexBufferBinding");
                }
            }
            else
            {
                hr = getActiveD3D9Device()->SetStreamSourceFreq( static_cast<UINT>(source), 1 );
                if (FAILED(hr))
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to reset unused D3D9 stream source Freq", 
                        "D3D9RenderSystem::setVertexBufferBinding");
                }
            }

        }
        
        if (useGlobalInstancingVertexBufferIsAvailable)
        {
        // bind global instance buffer if exist
        if( !globalInstanceVertexBuffer.isNull() )
        {
            if ( !indexesUsed )
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Instance data used without index data.", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }

            D3D9HardwareVertexBuffer * d3d9buf = 
                static_cast<D3D9HardwareVertexBuffer*>(globalInstanceVertexBuffer.get());

            hr = getActiveD3D9Device()->SetStreamSource(
                    static_cast<UINT>(source),
                    d3d9buf->getD3D9VertexBuffer(),
                    0, // no stream offset, this is handled in _render instead
                    static_cast<UINT>(d3d9buf->getVertexSize()) // stride
                    );

            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D9 stream source for buffer binding", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }

            hr = getActiveD3D9Device()->SetStreamSourceFreq( static_cast<UINT>(source), D3DSTREAMSOURCE_INSTANCEDATA | d3d9buf->getInstanceDataStepRate() );
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set D3D9 stream source Freq", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }
        }

        }
        
        // Unbind any unused sources
        for (size_t unused = source; unused < mLastVertexSourceCount; ++unused)
        {

            hr = getActiveD3D9Device()->SetStreamSource(static_cast<UINT>(unused), NULL, 0, 0);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to reset unused D3D9 stream source", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }

            hr = getActiveD3D9Device()->SetStreamSourceFreq( static_cast<UINT>(unused), 1 );
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to reset unused D3D9 stream source Freq", 
                    "D3D9RenderSystem::setVertexBufferBinding");
            }

        }
        mLastVertexSourceCount = source;

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::_render(const RenderOperation& op)
    {
        // Exit immediately if there is nothing to render
        // This caused a problem on FireGL 8800
        if (op.vertexData->vertexCount == 0)
            return;

        // Call super class
        RenderSystem::_render(op);


        if ( !mEnableFixedPipeline && !mRealCapabilities->hasCapability(RSC_FIXED_FUNCTION)
             && 
             (
                ( !mVertexProgramBound ) ||
                (!mFragmentProgramBound && op.operationType != RenderOperation::OT_POINT_LIST)        
              )
           ) 
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Attempted to render using the fixed pipeline when it is disabled.",
                "D3D9RenderSystem::_render");
        }

        // To think about: possibly remove setVertexDeclaration and 
        // setVertexBufferBinding from RenderSystem since the sequence is
        // a bit too D3D9-specific?
        setVertexDeclaration(op.vertexData->vertexDeclaration, op.useGlobalInstancingVertexBufferIsAvailable);
        setVertexBufferBinding(op.vertexData->vertexBufferBinding, op.numberOfInstances, op.useGlobalInstancingVertexBufferIsAvailable, op.useIndexes);

        // Determine rendering operation
        D3DPRIMITIVETYPE primType = D3DPT_TRIANGLELIST;
        DWORD primCount = 0;
        switch( op.operationType )
        {
        case RenderOperation::OT_POINT_LIST:
            primType = D3DPT_POINTLIST;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount);
            break;

        case RenderOperation::OT_LINE_LIST:
            primType = D3DPT_LINELIST;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 2;
            break;

        case RenderOperation::OT_LINE_STRIP:
            primType = D3DPT_LINESTRIP;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 1;
            break;

        case RenderOperation::OT_TRIANGLE_LIST:
            primType = D3DPT_TRIANGLELIST;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) / 3;
            break;

        case RenderOperation::OT_TRIANGLE_STRIP:
            primType = D3DPT_TRIANGLESTRIP;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
            break;

        case RenderOperation::OT_TRIANGLE_FAN:
            primType = D3DPT_TRIANGLEFAN;
            primCount = static_cast<DWORD>(op.useIndexes ? op.indexData->indexCount : op.vertexData->vertexCount) - 2;
            break;
        }

        if (!primCount)
            return;

        // Issue the op
        HRESULT hr;
        if( op.useIndexes )
        {
            D3D9HardwareIndexBuffer* d3dIdxBuf = 
                static_cast<D3D9HardwareIndexBuffer*>(op.indexData->indexBuffer.get());
            hr = getActiveD3D9Device()->SetIndices( d3dIdxBuf->getD3DIndexBuffer() );
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to set index buffer", "D3D9RenderSystem::_render" );
            }

            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase + 
                        mDerivedDepthBiasMultiplier * mCurrentPassIterationNum, 
                        mDerivedDepthBiasSlopeScale);
                }
                // do indexed draw operation
                hr = getActiveD3D9Device()->DrawIndexedPrimitive(
                    primType, 
                    static_cast<INT>(op.vertexData->vertexStart), 
                    0, // Min vertex index - assume we can go right down to 0 
                    static_cast<UINT>(op.vertexData->vertexCount), 
                    static_cast<UINT>(op.indexData->indexStart), 
                    static_cast<UINT>(primCount)
                    );

            } while (updatePassIterationRenderState());
        }
        else
        {
            // nfz: gpu_iterate
            do
            {
                // Update derived depth bias
                if (mDerivedDepthBias && mCurrentPassIterationNum > 0)
                {
                    _setDepthBias(mDerivedDepthBiasBase + 
                        mDerivedDepthBiasMultiplier * mCurrentPassIterationNum, 
                        mDerivedDepthBiasSlopeScale);
                }
                // Unindexed, a little simpler!
                hr = getActiveD3D9Device()->DrawPrimitive(
                    primType, 
                    static_cast<UINT>(op.vertexData->vertexStart), 
                    static_cast<UINT>(primCount)
                    ); 

            } while (updatePassIterationRenderState());
        } 

        if( FAILED( hr ) )
        {
            String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Failed to DrawPrimitive : " + msg, "D3D9RenderSystem::_render" );
        }

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::bindGpuProgram(GpuProgram* prg)
    {
        if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Null program bound.",
                "D3D9RenderSystem::bindGpuProgram");
        }

        HRESULT hr;
        switch (prg->getType())
        {
        case GPT_VERTEX_PROGRAM:
            hr = getActiveD3D9Device()->SetVertexShader(
                static_cast<D3D9GpuVertexProgram*>(prg)->getVertexShader());
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error calling SetVertexShader", "D3D9RenderSystem::bindGpuProgram");
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            hr = getActiveD3D9Device()->SetPixelShader(
                static_cast<D3D9GpuFragmentProgram*>(prg)->getPixelShader());
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error calling SetPixelShader", "D3D9RenderSystem::bindGpuProgram");
            }
            break;
        };

        // Make sure texcoord index is equal to stage value, As SDK Doc suggests:
        // "When rendering using vertex shaders, each stage's texture coordinate index must be set to its default value."
        // This solves such an errors when working with the Debug runtime -
        // "Direct3D9: (ERROR) :Stage 1 - Texture coordinate index in the stage must be equal to the stage index when programmable vertex pipeline is used".
        for (unsigned int nStage=0; nStage < 8; ++nStage)
            __SetTextureStageState(nStage, D3DTSS_TEXCOORDINDEX, nStage);

        RenderSystem::bindGpuProgram(prg);

    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::unbindGpuProgram(GpuProgramType gptype)
    {
        /*if (!prg)
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                "Null program bound.",
                "D3D9RenderSystem::bindGpuProgram");
        }*/

        HRESULT hr;
        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            mActiveVertexGpuProgramParameters.setNull();
            hr = getActiveD3D9Device()->SetVertexShader(NULL);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error resetting SetVertexShader to NULL", 
                    "D3D9RenderSystem::unbindGpuProgram");
            }
            break;
        case GPT_FRAGMENT_PROGRAM:
            mActiveFragmentGpuProgramParameters.setNull();
            hr = getActiveD3D9Device()->SetPixelShader(NULL);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error resetting SetPixelShader to NULL", 
                    "D3D9RenderSystem::unbindGpuProgram");
            }
            break;
        };
        RenderSystem::unbindGpuProgram(gptype);
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::bindGpuProgramParameters(GpuProgramType gptype, 
        GpuProgramParametersSharedPtr params, uint16 variability)
    {
        // special case pass iteration
        if (variability == (uint16)GPV_PASS_ITERATION_NUMBER)
        {
            bindGpuProgramPassIterationParameters(gptype);
            return;
        }
        
        if (variability & (uint16)GPV_GLOBAL)
        {
            // D3D9 doesn't support shared constant buffers, so use copy routine
            params->_copySharedParams();
        }

        HRESULT hr;
        GpuLogicalBufferStructPtr floatLogical = params->getFloatLogicalBufferStruct();
        GpuLogicalBufferStructPtr intLogical = params->getIntLogicalBufferStruct();

        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            mActiveVertexGpuProgramParameters = params;
            {
                            OGRE_LOCK_MUTEX(floatLogical->mutex);

                    for (GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin();
                        i != floatLogical->map.end(); ++i)
                    {
                        if (i->second.variability & variability)
                        {
                            size_t logicalIndex = i->first;
                            const float* pFloat = params->getFloatPointer(i->second.physicalIndex);
                            size_t slotCount = i->second.currentSize / 4;
                            assert (i->second.currentSize % 4 == 0 && "Should not have any "
                                "elements less than 4 wide for D3D9");

                        if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantF(
                            (UINT)logicalIndex, pFloat, (UINT)slotCount)))
                            {
                                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                    "Unable to upload vertex shader float parameters", 
                                    "D3D9RenderSystem::bindGpuProgramParameters");
                            }
                        }

                    }

            }
            // bind ints
            {
                            OGRE_LOCK_MUTEX(intLogical->mutex);

                    for (GpuLogicalIndexUseMap::const_iterator i = intLogical->map.begin();
                        i != intLogical->map.end(); ++i)
                    {
                        if (i->second.variability & variability)
                        {
                            size_t logicalIndex = i->first;
                            const int* pInt = params->getIntPointer(i->second.physicalIndex);
                            size_t slotCount = i->second.currentSize / 4;
                            assert (i->second.currentSize % 4 == 0 && "Should not have any "
                                "elements less than 4 wide for D3D9");

                        if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantI(
                            static_cast<UINT>(logicalIndex), pInt, static_cast<UINT>(slotCount))))
                            {
                                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                    "Unable to upload vertex shader int parameters", 
                                    "D3D9RenderSystem::bindGpuProgramParameters");
                            }
                        }
                    }

            }

            break;
        case GPT_FRAGMENT_PROGRAM:
            mActiveFragmentGpuProgramParameters = params;
            {
                            OGRE_LOCK_MUTEX(floatLogical->mutex);

                    for (GpuLogicalIndexUseMap::const_iterator i = floatLogical->map.begin();
                        i != floatLogical->map.end(); ++i)
                    {
                        if (i->second.variability & variability)
                        {
                            size_t logicalIndex = i->first;
                            const float* pFloat = params->getFloatPointer(i->second.physicalIndex);
                            size_t slotCount = i->second.currentSize / 4;
                            assert (i->second.currentSize % 4 == 0 && "Should not have any "
                                "elements less than 4 wide for D3D9");

                        if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantF(
                            static_cast<UINT>(logicalIndex), pFloat, static_cast<UINT>(slotCount))))
                            {
                                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                    "Unable to upload pixel shader float parameters", 
                                    "D3D9RenderSystem::bindGpuProgramParameters");
                            }
                        }
                    }

            }
            // bind ints
            {
                            OGRE_LOCK_MUTEX(intLogical->mutex);

                    for (GpuLogicalIndexUseMap::const_iterator i = intLogical->map.begin();
                        i != intLogical->map.end(); ++i)
                    {
                        if (i->second.variability & variability)
                        {
                            size_t logicalIndex = i->first;
                            const int* pInt = params->getIntPointer(i->second.physicalIndex);
                            size_t slotCount = i->second.currentSize / 4;
                            assert (i->second.currentSize % 4 == 0 && "Should not have any "
                                "elements less than 4 wide for D3D9");

                        if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantI(
                            static_cast<UINT>(logicalIndex), pInt, static_cast<UINT>(slotCount))))
                            {
                                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                                    "Unable to upload pixel shader int parameters", 
                                    "D3D9RenderSystem::bindGpuProgramParameters");
                            }
                        }

                    }

            }
            break;
        };
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::bindGpuProgramPassIterationParameters(GpuProgramType gptype)
    {

        HRESULT hr;
        size_t physicalIndex = 0;
        size_t logicalIndex = 0;
        const float* pFloat;

        switch(gptype)
        {
        case GPT_VERTEX_PROGRAM:
            if (mActiveVertexGpuProgramParameters->hasPassIterationNumber())
            {
                physicalIndex = mActiveVertexGpuProgramParameters->getPassIterationNumberIndex();
                logicalIndex = mActiveVertexGpuProgramParameters->getFloatLogicalIndexForPhysicalIndex(physicalIndex);
                pFloat = mActiveVertexGpuProgramParameters->getFloatPointer(physicalIndex);

                if (FAILED(hr = getActiveD3D9Device()->SetVertexShaderConstantF(
                    static_cast<UINT>(logicalIndex), pFloat, 1)))
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "Unable to upload vertex shader multi pass parameters", 
                        "D3D9RenderSystem::bindGpuProgramMultiPassParameters");
                }
            }
            break;

        case GPT_FRAGMENT_PROGRAM:
            if (mActiveFragmentGpuProgramParameters->hasPassIterationNumber())
            {
                physicalIndex = mActiveFragmentGpuProgramParameters->getPassIterationNumberIndex();
                logicalIndex = mActiveFragmentGpuProgramParameters->getFloatLogicalIndexForPhysicalIndex(physicalIndex);
                pFloat = mActiveFragmentGpuProgramParameters->getFloatPointer(physicalIndex);
                if (FAILED(hr = getActiveD3D9Device()->SetPixelShaderConstantF(
                    static_cast<UINT>(logicalIndex), pFloat, 1)))
                {
                    OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, 
                        "Unable to upload pixel shader multi pass parameters", 
                        "D3D9RenderSystem::bindGpuProgramMultiPassParameters");
                }
            }
            break;

        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setClipPlanesImpl(const PlaneList& clipPlanes)
    {
        size_t i;
        size_t numClipPlanes;
        D3DXPLANE dx9ClipPlane;
        DWORD mask = 0;
        HRESULT hr;

        numClipPlanes = clipPlanes.size();
        for (i = 0; i < numClipPlanes; ++i)
        {
            const Plane& plane = clipPlanes[i];

            dx9ClipPlane.a = plane.normal.x;
            dx9ClipPlane.b = plane.normal.y;
            dx9ClipPlane.c = plane.normal.z;
            dx9ClipPlane.d = plane.d;

            if (mVertexProgramBound)
            {
                // programmable clips in clip space (ugh)
                // must transform worldspace planes by view/proj
                D3DXMATRIX xform;
                D3DXMatrixMultiply(&xform, &mDxViewMat, &mDxProjMat);
                D3DXMatrixInverse(&xform, NULL, &xform);
                D3DXMatrixTranspose(&xform, &xform);
                D3DXPlaneTransform(&dx9ClipPlane, &dx9ClipPlane, &xform);
            }

            hr = getActiveD3D9Device()->SetClipPlane(static_cast<DWORD>(i), dx9ClipPlane);
            if (FAILED(hr))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set clip plane", 
                    "D3D9RenderSystem::setClipPlanes");
            }

            mask |= (1 << i);
        }

        hr = __SetRenderState(D3DRS_CLIPPLANEENABLE, mask);
        if (FAILED(hr))
        {
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set render state for clip planes", 
                "D3D9RenderSystem::setClipPlanes");
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::setScissorTest(bool enabled, size_t left, size_t top, size_t right,
        size_t bottom)
    {
        HRESULT hr;
        if (enabled)
        {
            if (FAILED(hr = __SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE)))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to enable scissor rendering state; " + getErrorDescription(hr), 
                    "D3D9RenderSystem::setScissorTest");
            }
            RECT rect;
            rect.left = static_cast<LONG>(left);
            rect.top = static_cast<LONG>(top);
            rect.bottom = static_cast<LONG>(bottom);
            rect.right = static_cast<LONG>(right);
            if (FAILED(hr = getActiveD3D9Device()->SetScissorRect(&rect)))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to set scissor rectangle; " + getErrorDescription(hr), 
                    "D3D9RenderSystem::setScissorTest");
            }
        }
        else
        {
            if (FAILED(hr = __SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE)))
            {
                OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Unable to disable scissor rendering state; " + getErrorDescription(hr), 
                    "D3D9RenderSystem::setScissorTest");
            }
        }
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::clearFrameBuffer(unsigned int buffers, 
        const ColourValue& colour, Real depth, unsigned short stencil)
    {
        DWORD flags = 0;
        if (buffers & FBT_COLOUR)
        {
            flags |= D3DCLEAR_TARGET;
        }
        if (buffers & FBT_DEPTH)
        {
            flags |= D3DCLEAR_ZBUFFER;
        }
        // Only try to clear the stencil buffer if supported
        if (buffers & FBT_STENCIL && mCurrentCapabilities->hasCapability(RSC_HWSTENCIL))
        {
            flags |= D3DCLEAR_STENCIL;
        }
        HRESULT hr;
        if( FAILED( hr = getActiveD3D9Device()->Clear( 
            0, 
            NULL, 
            flags,
            colour.getAsARGB(), 
            depth, 
            stencil ) ) )
        {
            String msg = DXGetErrorDescription(hr);
            OGRE_EXCEPT(Exception::ERR_RENDERINGAPI_ERROR, "Error clearing frame buffer : " 
                + msg, "D3D9RenderSystem::clearFrameBuffer" );
        }
    }
    // ------------------------------------------------------------------
    void D3D9RenderSystem::setClipPlane (ushort index, Real A, Real B, Real C, Real D)
    {
        float plane[4] = { A, B, C, D };
        getActiveD3D9Device()->SetClipPlane (index, plane);
    }

    // ------------------------------------------------------------------
    void D3D9RenderSystem::enableClipPlane (ushort index, bool enable)
    {
        DWORD prev;
        getActiveD3D9Device()->GetRenderState(D3DRS_CLIPPLANEENABLE, &prev);
        __SetRenderState(D3DRS_CLIPPLANEENABLE, enable?
            (prev | (1 << index)) : (prev & ~(1 << index)));
    }
    //---------------------------------------------------------------------
    HardwareOcclusionQuery* D3D9RenderSystem::createHardwareOcclusionQuery()
    {
        D3D9HardwareOcclusionQuery* ret = OGRE_NEW D3D9HardwareOcclusionQuery(); 
        mHwOcclusionQueries.push_back(ret);
        return ret;
    }
    //---------------------------------------------------------------------
    Real D3D9RenderSystem::getHorizontalTexelOffset()
    {
        // D3D considers the origin to be in the center of a pixel
        return -0.5f;
    }
    //---------------------------------------------------------------------
    Real D3D9RenderSystem::getVerticalTexelOffset()
    {
        // D3D considers the origin to be in the center of a pixel
        return -0.5f;
    }
    //---------------------------------------------------------------------
    Real D3D9RenderSystem::getMinimumDepthInputValue()
    {
        // Range [0.0f, 1.0f]
        return 0.0f;
    }
    //---------------------------------------------------------------------
    Real D3D9RenderSystem::getMaximumDepthInputValue()
    {
        // Range [0.0f, 1.0f]
        // D3D inverts even identity view matrices, so maximum INPUT is -1.0
        return -1.0f;
    }
    //---------------------------------------------------------------------
    IDirect3D9* D3D9RenderSystem::getDirect3D9()
    {
        IDirect3D9* pDirect3D9 = msD3D9RenderSystem->mD3D;

        if (pDirect3D9 == NULL)
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                "Direct3D9 interface is NULL !!!", 
                "D3D9RenderSystem::getDirect3D9" );
        }

        return pDirect3D9;
    }

    //---------------------------------------------------------------------
    UINT D3D9RenderSystem::getResourceCreationDeviceCount()
    {
        D3D9ResourceCreationPolicy creationPolicy = msD3D9RenderSystem->mResourceManager->getCreationPolicy();

        if (creationPolicy == RCP_CREATE_ON_ACTIVE_DEVICE)
        {
            return 1;
        }
        else if (creationPolicy == RCP_CREATE_ON_ALL_DEVICES)
        {
            return msD3D9RenderSystem->mDeviceManager->getDeviceCount();
        }

        OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
            "Invalid resource creation policy !!!", 
            "D3D9RenderSystem::getResourceCreationDeviceCount" );

        return 0;
    }

    //---------------------------------------------------------------------
    IDirect3DDevice9* D3D9RenderSystem::getResourceCreationDevice(UINT index)
    {
        D3D9ResourceCreationPolicy creationPolicy = msD3D9RenderSystem->mResourceManager->getCreationPolicy();
        IDirect3DDevice9* d3d9Device = NULL;

        if (creationPolicy == RCP_CREATE_ON_ACTIVE_DEVICE)
        {
            d3d9Device = msD3D9RenderSystem->getActiveD3D9Device();
        }
        else if (creationPolicy == RCP_CREATE_ON_ALL_DEVICES) 
        {
            d3d9Device = msD3D9RenderSystem->mDeviceManager->getDevice(index)->getD3D9Device();
        }
        else
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                "Invalid resource creation policy !!!", 
                "D3D9RenderSystem::getResourceCreationDevice" );
        }

        return d3d9Device;
    }

    //---------------------------------------------------------------------
    IDirect3DDevice9* D3D9RenderSystem::getActiveD3D9Device()
    {   
        D3D9Device* activeDevice = msD3D9RenderSystem->mDeviceManager->getActiveDevice();
        IDirect3DDevice9* d3d9Device;

        d3d9Device = activeDevice->getD3D9Device();

        if (d3d9Device == NULL)
        {
            OGRE_EXCEPT( Exception::ERR_INVALIDPARAMS, 
                "Current d3d9 device is NULL !!!", 
                "D3D9RenderSystem::getActiveD3D9Device" );
        }

        return d3d9Device;
    }   

    //---------------------------------------------------------------------
    IDirect3DDevice9* D3D9RenderSystem::getActiveD3D9DeviceIfExists()
    {   
        D3D9Device* activeDevice = msD3D9RenderSystem->mDeviceManager->getActiveDevice();
        return activeDevice ? activeDevice->getD3D9Device() : NULL;
    }   

    //---------------------------------------------------------------------
    // Formats to try, in decreasing order of preference
    D3DFORMAT ddDepthStencilFormats[]={
        D3DFMT_D24FS8,
        D3DFMT_D24S8,
        D3DFMT_D24X4S4,
        D3DFMT_D24X8,
        D3DFMT_D15S1,
        D3DFMT_D16,
        D3DFMT_D32
    };
#define NDSFORMATS (sizeof(ddDepthStencilFormats)/sizeof(D3DFORMAT))

    D3DFORMAT D3D9RenderSystem::_getDepthStencilFormatFor(D3DFORMAT fmt)
    {
        /// Check if result is cached
        DepthStencilHash::iterator i = mDepthStencilHash.find((unsigned int)fmt);
        if(i != mDepthStencilHash.end())
            return i->second;
        /// If not, probe with CheckDepthStencilMatch
        D3DFORMAT dsfmt = D3DFMT_UNKNOWN;

        /// Get description of primary render target
        D3D9Device* activeDevice = mDeviceManager->getActiveDevice();
        IDirect3DSurface9* mSurface = activeDevice->getPrimaryWindow()->getRenderSurface();
        D3DSURFACE_DESC srfDesc;

        if(mSurface && SUCCEEDED(mSurface->GetDesc(&srfDesc)))
        {
            /// Probe all depth stencil formats
            /// Break on first one that matches
            for(size_t x=0; x<NDSFORMATS; ++x)
            {
                // Verify that the depth format exists
                if (mD3D->CheckDeviceFormat(
                    activeDevice->getAdapterNumber(),
                    activeDevice->getDeviceType(),
                    srfDesc.Format,
                    D3DUSAGE_DEPTHSTENCIL,
                    D3DRTYPE_SURFACE,
                    ddDepthStencilFormats[x]) != D3D_OK)
                {
                    continue;
                }
                // Verify that the depth format is compatible
                if(mD3D->CheckDepthStencilMatch(
                    activeDevice->getAdapterNumber(),
                    activeDevice->getDeviceType(), 
                    srfDesc.Format,
                    fmt, ddDepthStencilFormats[x]) == D3D_OK)
                {
                    dsfmt = ddDepthStencilFormats[x];
                    break;
                }
            }
        }
        /// Cache result
        mDepthStencilHash[(unsigned int)fmt] = dsfmt;
        return dsfmt;
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::registerThread()
    {
        // nothing to do - D3D9 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::unregisterThread()
    {
        // nothing to do - D3D9 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::preExtraThreadsStarted()
    {
        // nothing to do - D3D9 shares rendering context already
    }
    //---------------------------------------------------------------------
    void D3D9RenderSystem::postExtraThreadsStarted()
    {
        // nothing to do - D3D9 shares rendering context already
    }
    //---------------------------------------------------------------------
    D3D9ResourceManager* D3D9RenderSystem::getResourceManager()
    {
        return msD3D9RenderSystem->mResourceManager;
    }

    //---------------------------------------------------------------------
    D3D9DeviceManager* D3D9RenderSystem::getDeviceManager()
    {
        return msD3D9RenderSystem->mDeviceManager;
    }

    //---------------------------------------------------------------------
    RenderSystemCapabilities* D3D9RenderSystem::createRenderSystemCapabilities() const
    {
        return mRealCapabilities;
    }

    //---------------------------------------------------------------------
	bool D3D9RenderSystem::IsActiveDeviceLost() 
	{
		return D3D9RenderSystem::getDeviceManager()->getActiveDevice()->isDeviceLost();
	}

    unsigned int D3D9RenderSystem::getDisplayMonitorCount() const
    {
        return mD3D->GetAdapterCount();
    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::beginProfileEvent( const String &eventName )
    {
        if( eventName.empty() )
            return;

        vector<wchar_t>::type result(eventName.length() + 1, '\0');
        (void)MultiByteToWideChar(CP_ACP, 0, eventName.data(), eventName.length(), &result[0], result.size());
        (void)D3DPERF_BeginEvent(D3DCOLOR_ARGB(1, 0, 1, 0), &result[0]);
    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::endProfileEvent( void )
    {
        (void)D3DPERF_EndEvent();
    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::markProfileEvent( const String &eventName )
    {
        if( eventName.empty() )
            return;

        vector<wchar_t>::type result(eventName.length() + 1, '\0');
        (void)MultiByteToWideChar(CP_ACP, 0, eventName.data(), eventName.length(), &result[0], result.size());
        (void)D3DPERF_SetMarker(D3DCOLOR_ARGB(1, 0, 1, 0), &result[0]);
    }

    //---------------------------------------------------------------------
    DWORD D3D9RenderSystem::getSamplerId(size_t unit) 
    {
        return static_cast<DWORD>(unit) +
            ((mTexStageDesc[unit].pVertexTex == NULL) ? 0 : D3DVERTEXTEXTURESAMPLER0);
    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::notifyOnDeviceLost(D3D9Device* device)
    {   
        StringStream ss;

        ss << "D3D9 Device 0x[" << device->getD3D9Device() << "] entered lost state";
        LogManager::getSingleton().logMessage(ss.str());

        fireDeviceEvent(device, "DeviceLost");

    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::notifyOnDeviceReset(D3D9Device* device)
    {       
        // Reset state attributes.  
        mVertexProgramBound = false;
        mFragmentProgramBound = false;
        mLastVertexSourceCount = 0;

        // Restore previous active device.

        // Invalidate active view port.
        mActiveViewport = NULL;

        StringStream ss;

        // Reset the texture stages, they will need to be rebound
        for (size_t i = 0; i < OGRE_MAX_TEXTURE_LAYERS; ++i)
            _setTexture(i, false, TexturePtr());

        LogManager::getSingleton().logMessage("!!! Direct3D Device successfully restored.");

        ss << "D3D9 device: 0x[" << device->getD3D9Device() << "] was reset";
        LogManager::getSingleton().logMessage(ss.str());

        fireDeviceEvent(device, "DeviceRestored");

    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::determineFSAASettings(IDirect3DDevice9* d3d9Device,
        size_t fsaa, const String& fsaaHint, D3DFORMAT d3dPixelFormat, 
        bool fullScreen, D3DMULTISAMPLE_TYPE *outMultisampleType, DWORD *outMultisampleQuality)
    {
        bool ok = false;
        bool qualityHint = fsaaHint.find("Quality") != String::npos;
        size_t origFSAA = fsaa;

        D3D9DriverList* driverList = getDirect3DDrivers();
        D3D9Driver* deviceDriver = mActiveD3DDriver;
        D3D9Device* device = mDeviceManager->getDeviceFromD3D9Device(d3d9Device);

        for (uint i = 0; i < driverList->count(); ++i)
        {
            D3D9Driver* currDriver = driverList->item(i);

            if (currDriver->getAdapterNumber() == device->getAdapterNumber())
            {
                deviceDriver = currDriver;
                break;
            }
        }

        bool tryCSAA = false;
        // NVIDIA, prefer CSAA if available for 8+
        // it would be tempting to use getCapabilities()->getVendor() == GPU_NVIDIA but
        // if this is the first window, caps will not be initialised yet
        if (deviceDriver->getAdapterIdentifier().VendorId == 0x10DE && 
            fsaa >= 8)
        {
            tryCSAA  = true;
        }

        while (!ok)
        {
            // Deal with special cases
            if (tryCSAA)
            {
                // see http://developer.nvidia.com/object/coverage-sampled-aa.html
                switch(fsaa)
                {
                case 8:
                    if (qualityHint)
                    {
                        *outMultisampleType = D3DMULTISAMPLE_8_SAMPLES;
                        *outMultisampleQuality = 0;
                    }
                    else
                    {
                        *outMultisampleType = D3DMULTISAMPLE_4_SAMPLES;
                        *outMultisampleQuality = 2;
                    }
                    break;
                case 16:
                    if (qualityHint)
                    {
                        *outMultisampleType = D3DMULTISAMPLE_8_SAMPLES;
                        *outMultisampleQuality = 2;
                    }
                    else
                    {
                        *outMultisampleType = D3DMULTISAMPLE_4_SAMPLES;
                        *outMultisampleQuality = 4;
                    }
                    break;
                }
            }
            else // !CSAA
            {
                *outMultisampleType = (D3DMULTISAMPLE_TYPE)fsaa;
                *outMultisampleQuality = 0;
            }


            HRESULT hr;
            DWORD outQuality;
            hr = mD3D->CheckDeviceMultiSampleType( 
                deviceDriver->getAdapterNumber(), 
                D3DDEVTYPE_HAL, 
                d3dPixelFormat, 
                fullScreen, 
                *outMultisampleType, 
                &outQuality);

            if (SUCCEEDED(hr) && 
                (!tryCSAA || outQuality > *outMultisampleQuality))
            {
                ok = true;
            }
            else
            {
                // downgrade
                if (tryCSAA && fsaa == 8)
                {
                    // for CSAA, we'll try downgrading with quality mode at all samples.
                    // then try without quality, then drop CSAA
                    if (qualityHint)
                    {
                        // drop quality first
                        qualityHint = false;
                    }
                    else
                    {
                        // drop CSAA entirely 
                        tryCSAA = false;
                    }
                    // return to original requested samples
                    fsaa = origFSAA;
                }
                else
                {
                    // drop samples
                    --fsaa;

                    OgreAssert(fsaa > 0, "FSAA underflow: infinite loop (this should never happen)");

                    if (fsaa <= 1)
                    {
                        // ran out of options, no FSAA
                        fsaa = 0;
                        ok = true;

                        *outMultisampleType = D3DMULTISAMPLE_NONE;
                        *outMultisampleQuality = 0;
                    }
                }
            }

        } // while !ok
    }

    //---------------------------------------------------------------------
    void D3D9RenderSystem::fireDeviceEvent( D3D9Device* device, const String & name )
    {
        NameValuePairList params;
        params["D3DDEVICE"] =  StringConverter::toString((size_t)device->getD3D9Device());
        params["DEVICE_ADAPTER_NUMBER"] =  StringConverter::toString(device->getAdapterNumber());

        fireEvent(name, &params);
    }
    //---------------------------------------------------------------------
#if OGRE_NO_QUAD_BUFFER_STEREO == 0
    void D3D9RenderSystem::createStereoDriver(const NameValuePairList* miscParams)
    {
        // Get the value used to create the render system.  If none, get the parameter value used to create the window.
        StereoModeType stereoMode = StringConverter::parseStereoMode(mOptions["Stereo Mode"].currentValue);
        if (stereoMode == SMT_NONE)
        {
            NameValuePairList::const_iterator iter = miscParams->find("stereoMode");
            if (iter != miscParams->end())
              stereoMode = StringConverter::parseStereoMode((*iter).second);
        }

        // Always create the stereo bridge regardless of the mode
        mStereoDriver = OGRE_NEW D3D9StereoDriverBridge(stereoMode);
    }
    //---------------------------------------------------------------------
    bool D3D9RenderSystem::setDrawBuffer(ColourBufferType colourBuffer)
    {
        return D3D9StereoDriverBridge::getSingleton().setDrawBuffer(colourBuffer);
    }
    //---------------------------------------------------------------------
#endif
}
