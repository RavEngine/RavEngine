#pragma once

#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreWindowEventUtilities.h>

class OgreStatics {
public:
	OgreStatics() {}
	virtual ~OgreStatics() {
		delete manager;
		delete root;
	}


	void init();

	/**
	Constructs an Ogre camera using a hidden SceneManager, detatches it, and returns the pointer
	@param name Name to give the new camera. Must be unique. 
	@param notShadowCaster True if this camera should be considered when creating the global light list of culled lights against all cameras. For example, cameras used for shadow mapping shouldn't be taken into account (set to false) 
	@param forCubeMapping True this camera will be used at least once in one of its passes as a cubemap (thus having to change the orientation but not position mid-rendering) 
	@return the created camera. Must be added to a scene to use.
	*/
	Ogre::Camera* createCamera(const Ogre::String& name, bool notShadowCaster = true, bool forCubeMapping = false) {
		auto cam = manager->createCamera(name, notShadowCaster, forCubeMapping);
		cam->detachFromParent();
		return cam;
	};

	Ogre::Root* const GetRoot() const{
		return root;
	}

	Ogre::SceneManager* const GetSceneManager() const {
		return manager;
	}

	Ogre::Window* const GetWindow() const {
		return window;
	}

protected:
	Ogre::SceneManager* manager = nullptr;
	Ogre::Root* root = nullptr;
	Ogre::Window* window = nullptr;
};