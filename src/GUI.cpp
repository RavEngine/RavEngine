#include "GUI.hpp"
#include "App.hpp"
#include "Debug.hpp"

using namespace RavEngine;
using namespace std;
using namespace Rml;


ElementDocument* GUIComponent::AddDocument(const std::string &name){
	if (IsDocumentLoaded(name)){
		Debug::Fatal("Document is already loaded");
	}
	
	string dir = "/uis/" + name;
	
	ElementDocument* ed = context->LoadDocument(dir);
	
	if (ed == nullptr){
		Debug::Fatal("Cannot load document at path {}", dir);
	}
	ed->Show();
	documents[name] = ed;
	return ed;
}

void GUIComponent::RemoveDocument(const std::string &name){
	if (!IsDocumentLoaded(name)){
		Debug::Fatal("Cannot unload document that is not loaded");
	}
	
	auto ptr = documents.at(name);
	documents.erase(name);
	context->UnloadDocument(ptr);
}

bool GUIComponent::IsDocumentLoaded(const std::string &name) const{
	return documents.contains(name);
}

bool GUIComponent::LoadFont(const std::string& filename){
	string dir = "/fonts/" + filename;
	
	return Rml::LoadFontFace(dir);
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

Rml::ElementDocument* GUIComponent::GetDocument(const std::string &name) const{
	if (!IsDocumentLoaded(name)){
		Debug::Fatal("Cannot get pointer to {} because it is not loaded.",name);
	}
	return documents.at(name);
}
