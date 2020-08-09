#pragma once

#include <OgreSceneManager.h>
#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreWindowEventUtilities.h>

class OgreStatics {
public:
	OgreStatics() {}
	virtual ~OgreStatics() {
		delete managerFactory;
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
	Ogre::Camera* const createCamera(const Ogre::String& name, bool notShadowCaster = true, bool forCubeMapping = false) {
		auto cam = managerFactory->createCamera(name, notShadowCaster, forCubeMapping);
		cam->detachFromParent();
		return cam;
	};

	/**
	Creates an instance of a SceneNode.
	@param sceneType Dynamic if this node is to be updated frequently. Static if you don't plan to be updating this node in a long time (performance optimization).
	*/
	Ogre::SceneNode* const createSceneNode(Ogre::SceneMemoryMgrTypes sceneType = Ogre::SceneMemoryMgrTypes::SCENE_DYNAMIC) {
		return managerFactory->createSceneNode(sceneType);	//this does not add the node to the hierarcy, so no need to detatch
	}

	/**
	Create a scene manager.
	@param name Optional name to given the new instance that is created. If you leave this blank, an auto name will be assigned. 
	@param type A mask containing one or more SceneType flags
	@param numThreads Number of worker threads. Must be greater than 0; you should not oversubscribe the system. I.e. if the system has 4 cores and you intend to run your logic 100% in one of the cores, set this value to 3. If you intend to fully use 2 cores for your own stuff, set this value to 2. 
	*/
	Ogre::SceneManager* const createSceneManager(const std::string& name, Ogre::SceneType type = Ogre::SceneType::ST_GENERIC, size_t numThreads = 1u){
		return root->createSceneManager(type, numThreads, name);
	}

	/**
	This call is meant for internal use.
	@return const-pointer to the Root. 
	*/
	Ogre::Root* const GetRoot() const{
		return root;
	}

	/**
	This call is meant for internal use.
	@return const-pointer to the factory manager.
	*/
	Ogre::SceneManager* const GetSceneManager() const {
		return managerFactory;
	}

	/**
	This call is meant for internal use.
	@return const-pointer to the window.
	*/
	Ogre::Window* const GetWindow() const {
		return window;
	}

protected:
	Ogre::SceneManager* managerFactory = nullptr;
	Ogre::Root* root = nullptr;
	Ogre::Window* window = nullptr;
};