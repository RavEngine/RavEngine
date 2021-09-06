#include "GUI.hpp"
#include "App.hpp"
#include "Debug.hpp"
#include "InputManager.hpp"
#include <RmlUi/Debugger.h>
#include <uuids.h>

using namespace RavEngine;
using namespace std;
using namespace Rml;

#define SDL2RML(a,b) case a: value = Rml::Input::KeyIdentifier::KI_ ## b; break;

/**
 Converts SDL (USB) scancodes to RML keys
 */
static inline Rml::Input::KeyIdentifier SDLtoRML(const int scancode){
	int value = 0;
	//a-z
	if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z){
		value = scancode +(Rml::Input::KeyIdentifier::KI_A - SDL_SCANCODE_A);
	}
	//arrows and controls
	else{
		switch(scancode){
			SDL2RML(SDL_SCANCODE_LEFT, LEFT);
			SDL2RML(SDL_SCANCODE_RIGHT, RIGHT);
			SDL2RML(SDL_SCANCODE_UP, UP);
			SDL2RML(SDL_SCANCODE_DOWN, DOWN);
			SDL2RML(SDL_SCANCODE_DELETE, DELETE)
			SDL2RML(SDL_SCANCODE_BACKSPACE, BACK)
			SDL2RML(SDL_SCANCODE_HOME, HOME)
			SDL2RML(SDL_SCANCODE_END, END)
			SDL2RML(SDL_SCANCODE_PAGEUP, PRIOR)
			SDL2RML(SDL_SCANCODE_PAGEDOWN, NEXT)
			SDL2RML(SDL_SCANCODE_INSERT, INSERT)
			SDL2RML(SDL_SCANCODE_RETURN, RETURN)
			SDL2RML(SDL_SCANCODE_KP_ENTER, NUMPADENTER)
			default:
				value = 0;
				break;
		}
	}

	return static_cast<Rml::Input::KeyIdentifier>(value);
}

static inline char TypeCharacter(SDL_Keycode key, uint32_t modifiers){
	char code = 0;
	switch(key){
		//no type
		case SDL_SCANCODE_RIGHT:
		case SDL_SCANCODE_UP:
		case SDL_SCANCODE_DOWN:
		case SDL_SCANCODE_LEFT:
		case SDL_SCANCODE_BACKSPACE:
		case SDL_SCANCODE_DELETE:
			break;
		default:
			code = static_cast<char>(SDL_GetKeyFromScancode(static_cast<SDL_Scancode>(key)));
			if (modifiers & Rml::Input::KeyModifier::KM_SHIFT || modifiers & Rml::Input::KeyModifier::KM_CAPSLOCK){
				code = std::toupper(code);
			}
			break;
	}
	return code;
}

ElementDocument* GUIComponent::AddDocument(const std::string &name){
	if (IsDocumentLoaded(name)){
		Debug::Fatal("Document is already loaded");
	}
	
	string dir = "/uis/" + name;
	
	ElementDocument* ed = nullptr;
	ExclusiveAccess([&] {
		ed = context->LoadDocument(dir);
	});
	
	
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
	ExclusiveAccess([&] {
		context->UnloadDocument(ptr);
	});
}

bool GUIComponent::IsDocumentLoaded(const std::string &name) const{
	return documents.contains(name);
}

bool GUIComponent::LoadFont(const std::string& filename){
	string dir = StrFormat("/fonts/{}",filename);
	
	return Rml::LoadFontFace(dir);
}

bool GUIComponent::Update(){
	MouseMove();
    // swap queues
    auto a = current.load();
    auto b = inactive.load();
    std::swap(a,b);
    current.store(a);
    inactive.store(b);
    
    bool result;
    ExclusiveAccess([&]{
        // process the 'inactive' queue (which was filled previously)
        std::function<void(void)> task;
        auto ptr = a;
        while(ptr->try_dequeue(task)){
            task();
        }
        result = context->Update();
    });
    
    return result;
}

bool GUIComponent::Render(){
	bool result;
    ExclusiveAccess([&]{
        result = context->Render();
    });
	return result;
}

GUIComponent::~GUIComponent(){
	for (const auto& pair : documents) {
		RemoveDocument(pair.first);		//destroy all the documents 
	}
	ExclusiveAccess([&] {
		RemoveContext(context->GetName());
	});
	
}

GUIComponent::GUIComponent() : GUIComponent(App::Renderer->GetBufferSize().width,App::Renderer->GetBufferSize().height, App::Renderer->GetDPIScale()){}

GUIComponent::GUIComponent(int width, int height, float DPIScale){
	auto uuid = uuids::uuid::create();
	
	context = Rml::CreateContext(uuid.to_string(), Vector2i(width,height));
	context->SetDensityIndependentPixelRatio(DPIScale);
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
		case SDL_SCANCODE_LGUI:
		case SDL_SCANCODE_RGUI:
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
	
	switch(charcode){
		case SDL_BUTTON_LEFT:
		case SDL_BUTTON_RIGHT:
		case SDL_BUTTON_MIDDLE:
			EnqueueUIUpdate([this,charcode] {
				context->ProcessMouseButtonDown(charcode - 1, modifier_state);
			});
			break;
		default:
            EnqueueUIUpdate([this,charcode] {
				context->ProcessKeyDown(SDLtoRML(charcode), modifier_state);
				//don't process on modifier keys
				auto code = TypeCharacter(charcode, modifier_state);
				if (! (modifier_state & Rml::Input::KeyModifier::KM_CTRL) && code != 0){
					context->ProcessTextInput(TypeCharacter(charcode, modifier_state));
				}
			});
			break;
	}
}

void GUIComponent::AnyActionUp(const int charcode){
	//If is a modifier, remove from the bitmask
	switch(charcode){
		case SDL_SCANCODE_LCTRL:
		case SDL_SCANCODE_RCTRL:
		case SDL_SCANCODE_LGUI:
		case SDL_SCANCODE_RGUI:
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
	switch(charcode){
		case SDL_BUTTON_LEFT:
		case SDL_BUTTON_RIGHT:
		case SDL_BUTTON_MIDDLE:
            EnqueueUIUpdate([this,charcode] {
				context->ProcessMouseButtonUp(charcode - 1, modifier_state);
			});
			break;
		default:
            EnqueueUIUpdate([this,charcode] {
				context->ProcessKeyUp(SDLtoRML(charcode), modifier_state);
			});
			break;
	}
}

void GUIComponent::MouseX(float normalized_pos){
	MousePos.x = normalized_pos;
}

void GUIComponent::MouseY(float normalized_pos){
	MousePos.y = normalized_pos;
}

void GUIComponent::ScrollY(float amt){
	if (std::abs(amt) > 0.1){
        EnqueueUIUpdate([this,amt]{
			context->ProcessMouseWheel(amt, modifier_state);
		});
	}
}

void GUIComponent::MouseMove(){
	//Forward to canvas, using the bitmask
    auto dim = context->GetDimensions();
    EnqueueUIUpdate([this,dim] {
		context->ProcessMouseMove(MousePos.x * dim.x, MousePos.y * dim.y, modifier_state);
	});
}

void GUIComponent::SetDimensions(uint32_t width, uint32_t height){
    EnqueueUIUpdate([this,width,height] {
		context->SetDimensions(Rml::Vector2i(width, height));
	});
}

void GUIComponent::Debug(){
#ifdef _DEBUG
	Rml::Debugger::SetContext(context);
	Rml::Debugger::SetVisible(true);
#endif
}
