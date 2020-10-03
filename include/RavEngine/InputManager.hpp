#pragma once
#include "SharedObject.hpp"
#include "SDL_scancode.h"
#include <SDL_mouse.h>
#include <SDL_gamecontroller.h>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include <functional>
#include <typeindex>
#include <SDL_events.h>
#include "IInputListener.hpp"

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

//controller IDs
enum class CID{
	NONE = 0,
	C0 = 1 << 0,
	C1 = 1 << 1,
	C2 = 1 << 2,
	C3 = 1 << 3,
	C4 = 1 << 4,
	C5 = 1 << 5,
	C6 = 1 << 6,
	C7 = 1 << 7,
	
	C8 = 1 << 8,
	C9 = 1 << 9,
	C10 = 1 << 10,
	C11 = 1 << 11,
	C12 = 1 << 12,
	C13 = 1 << 13,
	C14 = 1 << 14,
	C15 = 1 << 15,
	ANY = ~0
};
//bitwise
inline CID operator | (CID lhs, CID rhs)
{
	using T = std::underlying_type_t<CID>;
	return static_cast<CID>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

inline CID& operator |= (CID& lhs, CID rhs)
{
	lhs = lhs | rhs;
	return lhs;
}
inline CID operator & (CID lhs, CID rhs)
{
	using T = std::underlying_type_t<CID>;
	return static_cast<CID>(static_cast<T>(lhs) & static_cast<T>(rhs));
}

inline CID& operator &= (CID& lhs, CID rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

/**
 Construct a Controller ID object
 @param x the id of the controller
 */
static CID Make_CID(int x){
	return static_cast<CID>(1 << x);
}


typedef std::function<void(float)> axisCallback;
typedef std::function<void()> actionCallback;

namespace RavEngine {
    class InputManager : public SharedObject
    {
    protected:
		//helper classes
		class Callback{
		protected:
			IInputListener* obj;
			void* func;
			CID controllers;
		public:
			Callback(IInputListener* o, void* f, CID con) : obj(o), func(f), controllers(con){}
			bool operator==(const Callback& other)const{
				return func == other.func && obj == other.obj && controllers == other.controllers;
			}
			/**
			 Check if the stored pointer matches another
			 @param in the pointer to check
			 */
			bool ObjectsMatch(void* in) const{
				return in == obj;
			}
			IInputListener* const GetObj() {
				return obj;
			}
		};
		
		/**
		 Describes an AxisCallback as stored internally. Defines function and equality operators
		 */
		class AxisCallback : public Callback{
			axisCallback exec;
			float deadZone;
		public:
			static const float defaultDeadzone;
			/**
			 Construct an AxisCallback object.
			 @param thisptr the object to invoke the function on
			 @param f the function pointer to invoke.
			 */
			template<class U>
			AxisCallback(IInputListener* thisptr, void(U::* f)(float), float dz, CID con) : Callback(thisptr,&f, con), deadZone(dz){
				exec = std::bind(f, static_cast<U*>(thisptr), std::placeholders::_1);
			}
			/**
			 Execute the function on the stored pointer
			 @param f the float to pass
			 @param scale the amount to scale f
			 */
			void operator()(float f, float scale, CID incontrol){
				//check controllers, and do not execute if below deadzone
				if ((controllers & incontrol) != CID::NONE && abs(f) >= deadZone) {
					exec(f * scale);
				}
			}

			bool operator==(const AxisCallback& other) const {
				return Callback::operator==(other) && deadZone == other.deadZone;
			}
		};
		/**
		 Describes an ActionCallback as stored internally. Defines function and equality operators
		 */
		class ActionCallback : public Callback{
			actionCallback exec;
			ActionState type;
		public:
			/**
			 Construct an ActionCallback object
			 @param thisptr the object to invoke the function on
			 @param f the function pointer to invoke
			 @param t the state to bind
			 */
			template<class U>
			ActionCallback(IInputListener* thisptr, void(U::* f)(), ActionState t, CID con) : Callback(thisptr, &f, con){
				exec = std::bind(f, static_cast<U*>(thisptr));
				type = t;
			}
			/**
			 Execute the function on the stored pointer
			 */
			void operator()(CID incontrol){
				//check controllers
				if ((controllers & incontrol) != CID::NONE){
					exec();
				}
			}
			bool operator==(const ActionCallback& other)const{
				return Callback::operator==(other) && type == other.type;
			}
			/**
			 Determine if the states match
			 @param state the state to check
			 */
			bool IsCorrectState(ActionState state) const{
				return type == state;
			}
		};
		
		
		struct InputOccurance{
			CID controller;
		};
		struct ActionOccurance : public InputOccurance{
			ActionState state;
		};
		struct AxisOccurance : public InputOccurance{
			float currentValue;
		};
		
		struct Record{
			std::list<std::string> bindingNames;
		};
		
		struct ActionRecord : public Record{
			std::list<ActionOccurance> inputs;
		};
		
		struct AxisRecord : public Record {
			float scale = 1;
			std::list<AxisOccurance> events;
		};
		
        std::unordered_map<int, ActionRecord> codeToAction;
        std::unordered_map<std::string, std::list<ActionCallback>> actionMappings;

        std::unordered_map<int, AxisRecord> codeToAxis;                //ids to records
        std::unordered_map<std::string, std::list<AxisCallback>> axisMappings;     //strings to methods

        /**
         Helper used for registering axis inputs inside the engine
         */
        void reg_axis(int code, float value, CID controller) {
            if (codeToAxis.find(code) != codeToAxis.end()) {
				codeToAxis[code].events.push_back({controller,value});
            }
        }

        std::unordered_set<SDL_GameController*> connectedControllers;
		
		//methods to get input values
		void SDL_key(bool state, int charcode, CID controllerID);
		void SDL_mousemove(float x, float y, int xvel, int yvel, float scale);
		void SDL_mousekey(bool state, int charcode, CID controllerId);
		void SDL_ControllerAxis(int axisID, float value, CID controllerID);

    public:

        InputManager();

        void InitGameControllers();

        //based on the state of inputs, invoke bound actions
        void Tick();
		
		void AggregateInput(const SDL_Event&, uint32_t windowflags, float scale);

		/**
		 Create an action mapping entry. Action mappings correspond to items that have two states: pressed and released.
		 @param name the identifer to use when binding or unbinding actions
		 @param Id the button identifier to use. See the SDL key bindings for more information. To bind controllers, see the special bindings at the top of this file.
		 */
        void AddActionMap(const std::string& name, int Id);
		
		/**
		 Create an axis mapping entry. Axis mappings correspond to items that have a range of values, such as the mouse or analog sticks.
		 @param name the identifer to use when binding or unbinding axes
		 @param Id the button identifier to use. See the SDL key bindings for more information. To bind controllers, see the special bindings at the top of this file.
		 @param scale the scale factor to apply to all bindings mapped to this axis
		 */
        void AddAxisMap(const std::string& name, int Id, float scale = 1);
		
		/**
		 Remove an action mapping entry. Both the name and ID must match to complete removal.
		 @param name the identifer to look for
		 @param Id the button identifier to use. See the SDL key bindings for more information.
		 */
        void RemoveActionMap(const std::string& name, int Id);
		
		/**
		 Remove an axis mapping entry. Both the name and ID must match to complete removal.
		 @param name the identifer to look for
		 @param Id the button identifier to use. See the SDL key bindings for more information.
		 */
        void RemoveAxisMap(const std::string& name, int Id);
		
		/**
		 * Set the state of relative mouse mode. If true, the mouse will send events even if outside the application window. If false, the mouse will only send events if inside the application window.
		 * @param mode the new state
		 */
		static void SetRelativeMouseMode(bool mode);
		
		/**
		 * @returns the current relative mouse mode
		 */
		static bool GetRelativeMouseMode();

		/**
		 * Bind an action map to a member function
		 * @param name the name of the action map to bind to
		 * @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 * @param f the method to invoke when the action is triggered. Must take no parameters. Use &Classname::Methodname.
		 * @param type the required state of the action to invoke the method.
		 */
        template<class U>
		void BindAction(const std::string& name, IInputListener* thisptr, void(U::* f)(), ActionState type, CID controllers){
			ActionCallback action(thisptr,f,type,controllers);
			actionMappings[name].push_back(action);
			thisptr->OnRegister(this);
		}

        /**
         Bind a function to an Axis mapping
		 @param name the Axis mapping to bind to
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take one float parameter. Use &Classname::Methodname.
		 @param deadZone the minimum value (+/-) required to activate this binding
         */
        template<typename U>
        void BindAxis(const std::string& name, IInputListener* thisptr, void(U::* f)(float), CID controllers, float deadZone = AxisCallback::defaultDeadzone) {
			AxisCallback axis(thisptr, f, deadZone,controllers);
            axisMappings[name].push_back(axis);
			thisptr->OnRegister(this);
        }

		/**
		 Bind a function to an Action mapping
		 @param name the Action mapping to unbind
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take no parameters. Use &Classname::Methodname.
		 @param state the state to use to match the callback
		 */
		template<typename U>
		void UnbindAction(const std::string& name, IInputListener* thisptr, void(U::* f)(), ActionState type, CID controllers){
			ActionCallback action(thisptr,f,type,controllers);
			actionMappings[name].remove(action);
			thisptr->OnUnregister(this);
		}
		
		
		/**
		 Unbind an Axis mapping
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take one float parameter. Use &Classname::Methodname.
		 @param deadZone the minimum value (+/-) required to activate this binding
		 */
		template<typename U>
		void UnbindAxis(const std::string& name, IInputListener* thisptr, void(U::* f)(float), CID controllers, float deadZone = AxisCallback::defaultDeadzone){
			AxisCallback axis(thisptr, f, deadZone,controllers);
			axisMappings[name].remove(axis);
			thisptr->OnUnregister(this);
		}

		/**
		 * Unbind all Action and Axis mappings for a given listener. Listeners automatically invoke this on destruction.
		 * @param act the listener to unbind
		 */
        void UnbindAllFor(IInputListener* act);

        virtual ~InputManager();

    };
}
