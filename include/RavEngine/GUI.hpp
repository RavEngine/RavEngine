#pragma once

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core.h>
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
	
	/**
	 Load a document from disk with a name
	 @param name the name of the RML file
	 @throws if this document has already been loaded into this context
	 */
	Rml::ElementDocument* AddDocument(const std::string& name);
	
	/**
	 Unload a document from disk with a name
	 @param name the name of the RML file
	 @throws if this document is not loaded
	 */
	void RemoveDocument(const std::string& name);
	
	/**
	 @returns true if a document with the passed name is currently loaded.
	 @param name the name of the RML file
	 */
	bool IsDocumentLoaded(const std::string& name) const;
	
	/**
	 Get a pointer to a document, for performing queries or bindings.
	 @param name the name of the document.
	 @return a pointer to the RML ElementDocument
	 @throws if the document is not loaded.
	 */
	Rml::ElementDocument* GetDocument(const std::string& name) const;
	
	static bool LoadFont(const std::string& filename, const std::string& fontname, Rml::Style::FontStyle style, Rml::Style::FontWeight weight);
	
	virtual ~GUIComponent();
	
	bool Update();
	
	bool Render();
};
}

