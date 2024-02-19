#if !RVE_SERVER
#include "InputManager.hpp"
#include <SDL_events.h>
#include <SDL.h>
#include <RenderEngine.hpp>
#include "Debug.hpp"
#include "App.hpp"
#include <phmap.h>

using namespace std;
using namespace RavEngine;


InputManager::InputManager() {
	//register all the controllers
    SDL_SetGamepadEventsEnabled(true);
}

vector2 RavEngine::InputManager::GetMousePosPixels(float scaleFactor)
{
	vector2 pos;
	SDL_GetMouseState(&pos.x, &pos.y);
#ifdef __APPLE__
	pos *= scaleFactor;
#endif
	return pos;
}

void InputManager::ProcessActionID(int id, ActionState state_in, CID controller){
	if (CodeToAction.contains(id)){
		//get the actions that need to be run
		for(const auto& actionName : CodeToAction[id]){
			//for each action, get the bindings to execute
			for(const auto& binding : ActionBindings[actionName]){
				binding(state_in, controller);	//execute the binding
			}
		}
	}
	//process the Any actions
	for(auto l : AnyEventBindings){
		Ref<IInputListener> listener = l.lock();
		if (listener){
			if (state_in){
				listener->AnyActionDown(id);
			}
			else{
				listener->AnyActionUp(id);
			}
		}
	}
	
}

void InputManager::ProcessAxisID(int ID, float value, CID controller){
	if (CodeToAxis.contains(ID)){
		//buffer the input
		for(const auto& axisName : CodeToAxis[ID]){
			auto scale = axisName.scale;
			AxisBindings[axisName.identifier].bufferedInputs[ID]={value * scale,controller};
		}
	}
}

void InputManager::TickAxes(){
	
	for(auto& binding : AxisBindings){
		//get each binding
		for(const auto& axis : binding.second.bindings){
			//pass each buffered value to each Action
			for(const auto& buffered_value : binding.second.bufferedInputs){
				axis(buffered_value.second.value, buffered_value.second.source_controller);
			}
		}
	}
	
	//clear mouse velocity inputs
	auto clearvel = [&](int ID){
		if (CodeToAxis.contains(ID)){
			//buffer the input
			for(const auto& axisName : CodeToAxis[ID]){
				auto scale = axisName.scale;
				AxisBindings[axisName.identifier].bufferedInputs[ID]={0,CID::C0};
			}
		}
	};
	
	//now clear mouse velocity inputs
	clearvel(Special::MOUSEMOVE_XVEL);
	clearvel(Special::MOUSEMOVE_YVEL);
	clearvel(Special::MOUSEWHEEL_X);
	clearvel(Special::MOUSEWHEEL_Y);
	
	CleanupBindings();
}

void InputManager::CleanupBindings(){
	
	//clean up invalid action bindings
	for(auto& binding : ActionBindings){
		binding.second.remove_if([](const ActionBinding& b) -> bool{
			return !b.IsValid();
		});
	}
	
	//clean up invalid axis bindings
	for(auto& binding : AxisBindings){
		binding.second.bindings.remove_if([](const AxisBinding& b) -> bool{
			return !b.IsValid();
		});
	}
	
	//clean up invalid Any Actions
	AnyEventBindings.remove_if([](const WeakPtrKey<IInputListener>& w) -> bool{
		return w.get_weak().expired();
	});
}

void InputManager::ProcessInput(const SDL_Event& event, uint32_t windowflags, float scale, int windowWidth, int windowHeight, float dpiScale){
	switch (event.type) {
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			ProcessActionID(event.key.keysym.scancode, static_cast<ActionState>(event.key.state), Make_CID(0));
			ProcessAxisID(event.key.keysym.scancode, event.key.state, Make_CID(0));
			break;
        case SDL_EVENT_MOUSE_MOTION:
            if (windowflags & SDL_EVENT_WINDOW_TAKE_FOCUS) {
				int width = windowWidth, height = windowHeight;
				
				float velscale = 1 / scale;
				
				ProcessAxisID(Special::MOUSEMOVE_X, (float)event.motion.x / (width / dpiScale), CID::C0);
				ProcessAxisID(Special::MOUSEMOVE_Y, (float)event.motion.y / (height / dpiScale), CID::C0);
				
				ProcessAxisID(Special::MOUSEMOVE_XVEL, event.motion.xrel * velscale, CID::C0);
				ProcessAxisID(Special::MOUSEMOVE_YVEL, event.motion.yrel * velscale, CID::C0);
				
			}
			break;
        case SDL_EVENT_MOUSE_WHEEL:
			ProcessAxisID(Special::MOUSEWHEEL_X, event.wheel.x * -0.2, CID::C0);
			ProcessAxisID(Special::MOUSEWHEEL_Y, event.wheel.y * -0.2, CID::C0);
			break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (windowflags & SDL_EVENT_WINDOW_TAKE_FOCUS) {
				ProcessActionID(event.button.button, static_cast<ActionState>(event.button.state), CID::C0);
			}
			break;
            
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		case SDL_GAMEPAD_AXIS_LEFTX:
		case SDL_GAMEPAD_AXIS_LEFTY:
			ProcessAxisID(event.gaxis.axis + Special::CONTROLLER_AXIS_OFFSET, event.gaxis.value, Make_CID(event.gdevice.which + 1));
			break;
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP:
			// a controller button can be an axis or an action
			ProcessActionID(event.gbutton.button + Special::CONTROLLER_BUTTON_OFFSET, static_cast<ActionState>(event.gbutton.state), Make_CID(event.gdevice.which + 1));
			ProcessAxisID(event.gbutton.button + Special::CONTROLLER_BUTTON_OFFSET, static_cast<ActionState>(event.gbutton.state), Make_CID(event.gdevice.which + 1));
			break;
		case SDL_EVENT_GAMEPAD_ADDED:
			{
				Debug::Log("Controller added : {}",SDL_GetGamepadName(SDL_OpenGamepad(event.gdevice.which)));
			}
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			Debug::Log("Controller removed");
			break;
	}
}

void RavEngine::InputManager::SetRelativeMouseMode(bool mode){
	SDL_SetRelativeMouseMode(mode ? SDL_TRUE : SDL_FALSE);
}

bool InputManager::GetRelativeMouseMode(){
	return SDL_GetRelativeMouseMode();
}
#endif
