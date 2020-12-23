#include "GUI.hpp"
#include <RmlUi/Core.h>
#include <RmlUi/Core/Context.h>
#include "App.hpp"

using namespace RavEngine;
using namespace std;
using namespace Rml;


void GUIComponent::AddDocument(const std::string &name){
	if (IsDocumentLoaded(name)){
		throw runtime_error("Document is already loaded");
	}
	
	string dir = "/uis/" + name;
	
	auto docstr = App::Resources->FileContentsAt(dir.c_str());
	ElementDocument* ed = context->LoadDocumentFromMemory(docstr, name);
	
	documents[name] = ed;
}

void GUIComponent::RemoveDocument(const std::string &name){
	if (!IsDocumentLoaded(name)){
		throw runtime_error("Cannot unload document that is not loaded");
	}
	
	auto ptr = documents.at(name);
	documents.erase(name);
	context->UnloadDocument(ptr);
}

bool GUIComponent::IsDocumentLoaded(const std::string &name){
	return documents.contains(name);
}

bool GUIComponent::Update(){
	return context->Update();
}

bool GUIComponent::Render(){
	return context->Render();
}

GUIComponent::~GUIComponent(){
	for(const auto& pair : documents){
		RemoveDocument(pair.first);		//destroy all the documents 
	}
	RemoveContext(context->GetName());
}

GUIComponent::GUIComponent(const string& name) : GUIComponent(name,App::Renderer->GetBufferSize().width,App::Renderer->GetBufferSize().height){
}

GUIComponent::GUIComponent(const string& name, int width, int height){
	context = Rml::CreateContext(name, Vector2i(width,height));
}
