#pragma once
#include "SharedObject.hpp"
//#include <SDL_scancode.h>
//#include <SDL_mouse.h>
//#include <SDL_gamecontroller.h>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <functional>
#include <typeindex>

enum class ActionState{
  Released, Pressed
};

struct Special {
    enum {
        MOUSEMOVE_X = -500,
        MOUSEMOVE_Y = -501,
        MOUSEMOVE_XVEL = -502,
        MOUSEMOVE_YVEL = -503,
        CONTROLLER_AXIS_OFFSET = -8000,
        CONTROLLER_BUTTON_OFFSET = -10000
    };
};

//use this when binding controller buttons
struct ControllerButton {
    enum {
        SDL_CONTROLLER_BUTTON_INVALID = -1,
        SDL_CONTROLLER_BUTTON_A = Special::CONTROLLER_BUTTON_OFFSET,
        SDL_CONTROLLER_BUTTON_B,
        SDL_CONTROLLER_BUTTON_X,
        SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_BACK,
        SDL_CONTROLLER_BUTTON_GUIDE,
        SDL_CONTROLLER_BUTTON_START,
        SDL_CONTROLLER_BUTTON_LEFTSTICK,
        SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
        SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_DPAD_UP,
        SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT,
        SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        SDL_CONTROLLER_BUTTON_MAX
    };
};

//use this when binding controller axis
struct ControllerAxis {
    enum {
        SDL_CONTROLLER_AXIS_INVALID = -1,
        SDL_CONTROLLER_AXIS_LEFTX = Special::CONTROLLER_AXIS_OFFSET,
        SDL_CONTROLLER_AXIS_LEFTY,
        SDL_CONTROLLER_AXIS_RIGHTX,
        SDL_CONTROLLER_AXIS_RIGHTY,
        SDL_CONTROLLER_AXIS_TRIGGERLEFT,
        SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
        SDL_CONTROLLER_AXIS_MAX
    };
};


struct Event{
    int ID;
    ActionState value;
    friend std::ostream& operator<<(std::ostream& os, const Event& dt){
        os << "EVT id = " << dt.ID << " value = " << (dt.value == ActionState::Released ? "Released" : "Pressed");
        return os;
    }
};

typedef std::function<void(float)> axisCallback;
typedef std::function<void()> actionCallback;

namespace RavEngine {
    class InputSystem : public SharedObject
    {
    protected:
        std::list<Event> actionValues;
        std::unordered_set<int> awareActionValues;
        std::unordered_map<int, std::list<std::string>> codeToAction;
        std::unordered_map<std::string, std::list<std::pair<std::pair<actionCallback, std::type_index>, ActionState>>> actionMappings;

        //axis storage
        std::unordered_map<int, float> axisValues;                      //ids to current values
        std::unordered_map<int, float> axisScalars;                     //ids to scalars
        std::unordered_map<int, std::list<std::string>> codeToAxis;                //ids to strings
        std::unordered_map<std::string, std::list<std::pair<axisCallback, std::type_index>>> axisMappings;     //strings to methods

        /**
         Helper used for registering axis inputs inside the engine
         */
        void reg_axis(int code, float value) {
            if (axisScalars.find(code) != axisScalars.end()) {
                axisValues[code] = value;
            }
        }

        //std::unordered_set<SDL_GameController*> connectedControllers;

    public:
        InputSystem();

        void InitGameControllers();

        //based on the state of inputs, invoke bound actions
        virtual void tick();

        //methods to get input values
        void SDL_key(bool state, int charcode);
        void SDL_mousemove(float x, float y, int xvel, int yvel);
        void SDL_mousekey(bool state, int charcode);
        void SDL_ControllerAxis(int axisID, float value);

        void AddActionMap(const std::string& name, int Id);
        void AddAxisMap(const std::string& name, int Id, float scale = 1);
        void RemoveActionMap(const std::string& name, int Id);
        void RemoveAxisMap(const std::string& name, int Id);

        void BindAction(const std::string&, const actionCallback&, ActionState);
        void BindAxis(const std::string&, const axisCallback&);
        void UnbindAction(const std::string&, const actionCallback&, ActionState);
        void UnbindAxis(const std::string&, const axisCallback&);

        virtual ~InputSystem();

    };
}