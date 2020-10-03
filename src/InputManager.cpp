
#include "InputManager.hpp"
#include "GameplayStatics.hpp"
#include <SDL_events.h>
#include <SDL.h>

using namespace std;
using namespace RavEngine;

//default deadzone for default argument
const float InputManager::AxisCallback::defaultDeadzone = 0.2;

InputManager::InputManager() {
    InitGameControllers();
}

/**
 * Initialize game controller inputs
 */
void InputManager::InitGameControllers() {
    //setup game controllers
    int numControllers = 0;

    //determine which are controllers
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            ++numControllers;
        }
    }

    //register all the controllers
    for (int i = 0; i < numControllers; ++i) {
        SDL_GameController* controller = SDL_GameControllerOpen(i);
        if (SDL_GameControllerGetAttached(controller) == 1) {
            connectedControllers.insert(controller);
        }
    }
    SDL_GameControllerEventState(SDL_ENABLE);
}

void InputManager::AggregateInput(const SDL_Event& event, uint32_t windowflags, float scale){
	switch (event.type) {
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			SDL_key(event.key.state, event.key.keysym.scancode,Make_CID(0));
			break;
		case SDL_MOUSEMOTION:
			if (windowflags & SDL_WINDOW_INPUT_FOCUS) {
				int width, height;
				SDL_GetWindowSize(RenderEngine::GetWindow(), &width, &height);
				SDL_mousemove((float)event.motion.x / width, (float)event.motion.y / height, event.motion.xrel, event.motion.yrel, scale);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			if (windowflags & SDL_WINDOW_INPUT_FOCUS) {
				SDL_mousekey(event.button.state, event.button.button, Make_CID(0));
			}
			break;
		case SDL_CONTROLLERAXISMOTION:
		case SDL_CONTROLLER_AXIS_LEFTX:
		case SDL_CONTROLLER_AXIS_LEFTY:
			SDL_ControllerAxis(event.caxis.axis + Special::CONTROLLER_AXIS_OFFSET, (event.caxis.value) / ((float)SHRT_MAX), Make_CID(event.cdevice.which + 1));
			break;
		case SDL_CONTROLLERBUTTONDOWN:
		case SDL_CONTROLLERBUTTONUP:
			SDL_mousekey(event.cbutton.state + Special::CONTROLLER_BUTTON_OFFSET, event.cbutton.button,Make_CID(event.cdevice.which + 1));
			break;
		case SDL_CONTROLLERDEVICEADDED:
		case SDL_CONTROLLERDEVICEREMOVED:
			break;
	}
}

void InputManager::Tick() {
    //action mappings
	//get the list of functions to invoke
	for (auto& p : codeToAction) {
		for(auto& a : p.second.bindingNames){
			if (actionMappings.find(a) != actionMappings.end()) {
				auto toInvoke = actionMappings.at(a);
				
				//determine which to invoke
				for (auto& action : toInvoke) {
					for(const auto& state : p.second.inputs){
						if (action.IsCorrectState(state.state)) {
							action(state.controller);
						}
					}
				}
			}
		}
		
	}
    
	
	for(auto& p : codeToAction){
		p.second.inputs.clear();
	}
	    
    //call all axis mappings
    for (auto& pair : codeToAxis){
		for (auto& a : pair.second.bindingNames) {
            if (axisMappings.find(a) != axisMappings.end()) {
				auto scale = codeToAxis[pair.first].scale;
				for(const auto& evt : codeToAxis[pair.first].values){
					for (auto& f : axisMappings.at(a)) {
						f(evt.second, scale,evt.first);
					}
				}
            }
        }
    }

	
    //mouse velocity needs to be cleared
    reg_axis(static_cast<int>(Special::MOUSEMOVE_XVEL), 0, CID::C0);
    reg_axis(static_cast<int>(Special::MOUSEMOVE_YVEL), 0, CID::C0);
}

/**
 * Add a key event
 * @param state true = pressed, false = released
 * @param charcode the keycode of the key central to the event
 */
void InputManager::SDL_key(bool state, int charcode, CID controller)
{
    //axis mapping?
    if (codeToAxis.find(charcode) != codeToAxis.end()){
		codeToAxis[charcode].values[controller] = (float)state;
    }
    
    //action mapping?
    if (codeToAction.find(charcode) != codeToAction.end()){
		codeToAction[charcode].inputs.push_back({controller,static_cast<ActionState>(state)});
    }
}

/**
 * Add a mouse movement event
 * @param x the x coordinate of the cursor (top-left origin)
 * @param y the y coordinate of the cursor (top-left origin)
 * @param xvel the x velocity of the cursor
 * @param yvel the y velocity of the cursor
 */
void InputManager::SDL_mousemove(float x, float y, int xvel, int yvel, float scale)
{
    const float velscale = 1 / scale;

    //mouse movements are axis events only
    reg_axis(static_cast<int>(Special::MOUSEMOVE_X),x,CID::C0);
    reg_axis(static_cast<int>(Special::MOUSEMOVE_Y),y,CID::C0);
    reg_axis(static_cast<int>(Special::MOUSEMOVE_XVEL), xvel * velscale, CID::C0);
    reg_axis(static_cast<int>(Special::MOUSEMOVE_YVEL), yvel * velscale, CID::C0);
}

void InputManager::SDL_mousekey(bool state, int charcode, CID controller)
{
    //clicks are treated the same as keyboard input
    SDL_key(state, charcode, controller);
}

void InputManager::SDL_ControllerAxis(int axisID, float value, CID controller)
{
    reg_axis(axisID, value, controller);
}

InputManager::~InputManager() {
    //need to remove dynamic mapping
    for (auto& p : axisMappings) {
        auto l = p.second;
        for (auto& entry : l) {
            entry.GetObj()->OnUnregister(this);
        }
    }
    for (auto& p : actionMappings) {
        auto l = p.second;
        for (auto& entry : l) {
            entry.GetObj()->OnUnregister(this);
        }
    }
}

/**
 Define an axis mapping by name. If the mapping already exists, an additonal input is created for the mapping. Axis maps are called every frame regardless of value.
 @param name the string identifier for the mapping
 @param Id the input ID (scancode, etc) for the mapping
 @param scale the scalar to multiply against the axis value
 */
void InputManager::AddAxisMap(const std::string& name, int Id, float scale){
    
    //add so that tick is aware of it
	codeToAxis[Id].bindingNames.push_back(name);
	codeToAxis[Id].scale = scale;
}

/**
Define an action mapping by name. If the mapping already exists, an additonal input is created for the mapping. The values of an action map may be 0 (off / released), or 1 (on / pressed). Action maps are called when they occur.
  @param name the string identifier for the mapping
 @param Id the input ID (scancode, etc) for the mapping
 */
void InputManager::AddActionMap(const std::string& name, int Id){
    
    //add so that tick is aware of this
	codeToAction[Id].bindingNames.push_back(name);
}

void InputManager::RemoveActionMap(const std::string& name, int Id)
{
    //remove from ID tracking lists
    codeToAction.erase(Id);
}

void InputManager::RemoveAxisMap(const std::string& name, int Id)
{
    //remove from ID tracking lists
    codeToAction.erase(Id);
}

void RavEngine::InputManager::UnbindAllFor(IInputListener* act)
{
	//unbind axis maps
	for(const auto& p : axisMappings){
		auto key = p.second;
		key.remove_if([&act](AxisCallback& callback) -> bool {
			return callback.ObjectsMatch(act);
		});
	}
	//unbind action maps
	for(const auto& p : actionMappings){
		auto key = p.second;
		key.remove_if([&act](ActionCallback callback){
			return callback.ObjectsMatch(act);
		});
	}
}

void RavEngine::InputManager::SetRelativeMouseMode(bool mode){
	SDL_SetRelativeMouseMode(mode ? SDL_TRUE : SDL_FALSE);
}

bool InputManager::GetRelativeMouseMode(){
	return SDL_GetRelativeMouseMode();
}

