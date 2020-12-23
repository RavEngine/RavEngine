#pragma once

#include <RmlUi/Core/Context.h>
#include "Component.hpp"
#include "SpinLock.hpp"
#include "Locked_Hashmap.hpp"
#include "Queryable.hpp"

namespace RavEngine{
class GUIComponent : public RavEngine::Component, public RavEngine::Queryable<GUIComponent>{
protected:
	Rml::Context* context = nullptr;
	locked_hashmap<std::string, Rml::ElementDocument*, SpinLock> documents;
	
public:
	
	/**
	 Construct a GUI document using the current screen size
	 @param docname the name of the rml file to load
	 */
	GUIComponent(const std::string& name);
	
	
	/**
	 Construct a GUI renderer with user-supplied size
	 @param docname the name of the rml file to load
	 @param width the width in pixels of the context
	 @param height the height in pixels of the context
	 */
	GUIComponent(const std::string& name, int width, int height);
	
	void AddDocument(const std::string& name);
	void RemoveDocument(const std::string& name);
	bool IsDocumentLoaded(const std::string& name);
	
	virtual ~GUIComponent();
	
	bool Update();
	
	bool Render();
};
}

