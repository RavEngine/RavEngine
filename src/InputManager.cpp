
#include "InputManager.hpp"
#include <SDL_events.h>
#include <SDL.h>
#include <RenderEngine.hpp>
#include "Debug.hpp"
#include "App.hpp"

using namespace std;
using namespace RavEngine;


InputManager::InputManager() {
	//register all the controllers
	SDL_GameControllerEventState(SDL_ENABLE);
}

vector2i RavEngine::InputManager::GetMousePosPixels()
{
	vector2i pos;
	SDL_GetMouseState(&pos.x, &pos.y);
#ifdef __APPLE__
	pos *= GetApp()->GetRenderEngine().GetDPIScale();
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

void InputManager::ProcessInput(const SDL_Event& event, uint32_t windowflags, float scale){
	switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			ProcessActionID(event.key.keysym.scancode, static_cast<ActionState>(event.key.state), Make_CID(0));
			ProcessAxisID(event.key.keysym.scancode, event.key.state, Make_CID(0));
			break;
		case SDL_MOUSEMOTION:
			if (windowflags & SDL_WINDOW_INPUT_FOCUS) {
				int width, height;
				SDL_GetWindowSize(RenderEngine::GetWindow(), &width, &height);
				
				float velscale = 1 / scale;
				
				ProcessAxisID(Special::MOUSEMOVE_X, (float)event.motion.x / width, CID::C0);
				ProcessAxisID(Special::MOUSEMOVE_Y, (float)event.motion.y / height, CID::C0);
				
				ProcessAxisID(Special::MOUSEMOVE_XVEL, event.motion.xrel * velscale, CID::C0);
				ProcessAxisID(Special::MOUSEMOVE_YVEL, event.motion.yrel * velscale, CID::C0);
				
			}
			break;
		case SDL_MOUSEWHEEL:
			ProcessAxisID(Special::MOUSEWHEEL_X, event.wheel.x * -0.2, CID::C0);
			ProcessAxisID(Special::MOUSEWHEEL_Y, event.wheel.y * -0.2, CID::C0);
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (windowflags & SDL_WINDOW_INPUT_FOCUS) {
				ProcessActionID(event.button.button, static_cast<ActionState>(event.button.state), CID::C0);
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLER_AXIS_LEFTX:
		case SDL_CONTROLLER_AXIS_LEFTY:
			ProcessAxisID(event.caxis.axis + Special::CONTROLLER_AXIS_OFFSET, event.caxis.value, Make_CID(event.cdevice.which + 1));
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			// a controller button can be an axis or an action
			ProcessActionID(event.cbutton.button + Special::CONTROLLER_BUTTON_OFFSET, static_cast<ActionState>(event.cbutton.state), Make_CID(event.cdevice.which + 1));
			ProcessAxisID(event.cbutton.button + Special::CONTROLLER_BUTTON_OFFSET, static_cast<ActionState>(event.cbutton.state), Make_CID(event.cdevice.which + 1));
			break;
		case SDL_CONTROLLERDEVICEADDED:
			{
				Debug::Log("Controller added : {}",SDL_GameControllerName(SDL_GameControllerOpen(event.cdevice.which)));
			}
			break;
		case SDL_CONTROLLERDEVICEREMOVED:
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
