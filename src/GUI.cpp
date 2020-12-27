#include "GUI.hpp"
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
	if (ed == nullptr){
		throw runtime_error("Cannot load document");
	}
	ed->Show();
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

bool GUIComponent::IsDocumentLoaded(const std::string &name) const{
	return documents.contains(name);
}

bool GUIComponent::LoadFont(const std::string& filename, const std::string& fontname, Rml::Style::FontStyle style, Rml::Style::FontWeight weight){
	string dir = "/fonts/" + filename;
	auto docstr = App::Resources->FileContentsAt(dir.c_str());
	const Rml::byte* data = reinterpret_cast<const Rml::byte*>(docstr.c_str());
	return Rml::LoadFontFace(data, docstr.size(), fontname, style, weight);
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
