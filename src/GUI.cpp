#include "GUI.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include "InputManager.hpp"

using namespace RavEngine;
using namespace std;
using namespace Rml;

/**
 Converts SDL (USB) scancodes to RML keys
 */
static inline Rml::Input::KeyIdentifier SDLtoRML(const int scancode){
	int value = 0;
	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z){
		value = scancode +(Rml::Input::KeyIdentifier::KI_A - SDL_SCANCODE_A);
	}

	return static_cast<Rml::Input::KeyIdentifier>(value);
}

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
	MouseMove();
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


void GUIComponent::AnyActionDown(const int charcode){
	//If is a modifier, add to the bitmask
	switch(charcode){
		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL:
			modifier_state |= Rml::Input::KeyModifier::KM_CTRL;
			break;
		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT:
			modifier_state |= Rml::Input::KeyModifier::KM_SHIFT;
			break;
		case SDL_SCANCODE_MENU:
			modifier_state |= Rml::Input::KeyModifier::KM_META;
			break;
		case SDL_SCANCODE_CAPSLOCK:
			modifier_state |= Rml::Input::KeyModifier::KM_CAPSLOCK;
			break;
		case SDL_SCANCODE_NUMLOCKCLEAR:
			modifier_state |= Rml::Input::KeyModifier::KM_NUMLOCK;
			break;
		case SDL_SCANCODE_SCROLLLOCK:
			modifier_state |= Rml::Input::KeyModifier::KM_SCROLLLOCK;
			break;
	}
	context->ProcessKeyDown(SDLtoRML(charcode), 1);
}

void GUIComponent::AnyActionUp(const int charcode){
	//If is a modifier, remove from the bitmask
	switch(charcode){
		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL:
			modifier_state &= ~Rml::Input::KeyModifier::KM_CTRL;
			break;
		case SDL_SCANCODE_LSHIFT:
		case SDL_SCANCODE_RSHIFT:
			modifier_state &= ~Rml::Input::KeyModifier::KM_SHIFT;
			break;
		case SDL_SCANCODE_MENU:
			modifier_state &= ~Rml::Input::KeyModifier::KM_META;
			break;
		case SDL_SCANCODE_CAPSLOCK:
			modifier_state &= ~Rml::Input::KeyModifier::KM_CAPSLOCK;
			break;
		case SDL_SCANCODE_NUMLOCKCLEAR:
			modifier_state &= ~Rml::Input::KeyModifier::KM_NUMLOCK;
			break;
		case SDL_SCANCODE_SCROLLLOCK:
			modifier_state &= ~Rml::Input::KeyModifier::KM_SCROLLLOCK;
			break;
	}
	context->ProcessKeyUp(SDLtoRML(charcode), 0);
}

void GUIComponent::MouseX(float normalized_pos){
	MousePos.x = normalized_pos;
}

void GUIComponent::MouseY(float normalized_pos){
	MousePos.y = normalized_pos;
}

void GUIComponent::MouseMove(){
	//Forward to canvas, using the bitmask
	auto dim = context->GetDimensions();
	context->ProcessMouseMove(MousePos.x * dim.x, MousePos.y * dim.y, modifier_state);
}

void GUIComponent::SetDimensions(uint32_t width, uint32_t height){
	context->SetDimensions(Rml::Vector2i(width,height));
}
