#pragma once
#if !RVE_SERVER
#include "Ref.hpp"
    #include <SDL3/SDL_scancode.h>
    #include <SDL3/SDL_mouse.h>
    #include <SDL3/SDL_events.h>
    #include <SDL3/SDL_gamepad.h>
#include "Function.hpp"
#include "IInputListener.hpp"
#include "SpinLock.hpp"
#include "Map.hpp"
#include "DataStructures.hpp"
#include <RavEngine/mathtypes.hpp>
#include "WeakRef.hpp"

namespace RavEngine {

	enum ActionState{
		Released = 0,
		Pressed = 1
	};

	struct Special {
		enum {
			MOUSEMOVE_X = -500,
			MOUSEMOVE_Y = -501,
			MOUSEMOVE_XVEL = -502,
			MOUSEMOVE_YVEL = -503,
			MOUSEWHEEL_X = -504,
			MOUSEWHEEL_Y = -505,
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
	constexpr inline CID operator | (CID lhs, CID rhs)
	{
		using T = std::underlying_type_t<CID>;
		return static_cast<CID>(static_cast<T>(lhs) | static_cast<T>(rhs));
	}

    constexpr inline CID& operator |= (CID& lhs, CID rhs)
	{
		lhs = lhs | rhs;
		return lhs;
	}
    constexpr inline CID operator & (CID lhs, CID rhs)
	{
		using T = std::underlying_type_t<CID>;
		return static_cast<CID>(static_cast<T>(lhs) & static_cast<T>(rhs));
	}

    constexpr inline CID& operator &= (CID& lhs, CID rhs)
	{
		lhs = lhs & rhs;
		return lhs;
	}

	/**
	 Construct a Controller ID object
	 @param x the id of the controller
	 */
    constexpr inline static CID Make_CID(int x){
		return static_cast<CID>(1 << x);
	}

	typedef Function<void(float)> axisCallback;
	typedef Function<void()> actionCallback;

    class InputManager
    {
	protected:
		class ActionBinding{
			actionCallback func;	//the lambda to invoke
			void* func_addr = nullptr;		//used for equality comparison
			size_t id;	//used for determining validity
			CID controller;
			ActionState state;
			
		public:
			/**
			 Create an ActionBinding object
			 @param thisptr the owning object to be bound to
			 @param fn the lambda to execute
			 @param addr the address of the original function on thisptr
			 @param c the controller bitmask for this binding
			 @param s the required state of the source controller
			 */
			ActionBinding(decltype(id) id, actionCallback fn, void* addr, CID c, ActionState s) : id(id), func(fn), controller(c), state(s), func_addr(addr){}
			/**
			 Execute this ActionBinding. If the action binding is invalid, or the input state / controller are not applicable, this function will do nothing.
			 @param state_in the state of the action being invoked
			 @param controller the source controller sending the action
			 */
            constexpr inline void operator()(ActionState state_in, CID c_in) const{
				if (state_in == state && (controller & c_in) != CID::NONE && IsValid()){
					func();
				}
			}
			/**
			 @return true if this object can be executed (the bound object has not been destroyed), false otherwise
			 */
            inline bool IsValid() const{
                return true;
			}
			
			/**
			 Check equality
			 */
            constexpr inline bool operator==(const ActionBinding& other) const{
				return controller == other.controller && state == other.state && id == other.id && func_addr == other.func_addr;
			}
		};
		
		class AxisBinding{
		protected:
			axisCallback func;		//the lambda to invoke
			void* func_addr = nullptr;		//used for equality comparison
			size_t id; //used for determining validity
			CID controller;
			float deadzone = 0;
		public:
			/**
			 Create an ActionBinding object
			 @param thisptr the owning object to be bound to
			 @param fn the lambda to execute
			 @param addr the address of the original function on thisptr
			 @param c the controller bitmask for this binding
			 @param dz the binding deadzone filter range
			 @param scale the scale factor to apply to the passed value
			 */
			AxisBinding(decltype(id) id, axisCallback fn, void* addr, CID c, float dz): id(id), func(fn), func_addr(addr), controller(c),deadzone(dz){}
			
			/**
			 Execute this ActionBinding
			 @param value the axis' value
			 @param c_in the source controller
			 */
            constexpr inline void operator()(float value, CID c_in) const{
				//check if can run
				if ((controller & c_in) != CID::NONE && IsValid()){
					func(std::abs(value) >= deadzone ? value : 0);	//pass 0 if in deadzone range
				}
			}
            inline bool IsValid() const{
                return true;
				//return ! bound_object.expired();
			}
			
            constexpr inline bool operator==(const AxisBinding& other) const{
				return deadzone == other.deadzone && controller == other.controller && id == other.id && func_addr == other.func_addr;
			}
		};
		
		
		//stores which mappings each event ID is bound to
		locked_hashmap<int, locked_hashset<std::string,SpinLock>,SpinLock> CodeToAction;
		//the binding objects, keyed by binding ID
		locked_hashmap<std::string, LinkedList<ActionBinding>,SpinLock> ActionBindings;
		//stores which mappings each event ID is bound to
		struct AxisID{
			std::string identifier;
			float scale = 1;
		};
		
		struct aid_hasher{
            inline std::size_t operator()(const AxisID& other) const{
				std::hash<std::string> hasher;
				return hasher(other.identifier);
			}
		};
		
		struct aid_eq{
            inline bool operator()(const AxisID& A, const AxisID& B) const{
				return A.identifier == B.identifier && A.scale == B.scale;
			}
		};
		
		locked_hashmap<int, locked_hashset<AxisID,SpinLock,aid_hasher,aid_eq>,SpinLock> CodeToAxis;
		//the buffered axis inputs for each identifier
		struct AxisInput{
			float value;
			CID source_controller;
		};
		struct AxisData{
            UnorderedMap<int,AxisInput> bufferedInputs;
			LinkedList<AxisBinding> bindings;
		};
        UnorderedMap<std::string, AxisData> AxisBindings;
		
		//AnyActions
		LinkedList<WeakPtrKey<IInputListener>> AnyEventBindings;
		
		/**
		 Process a single action event
		 @param ID the event ID. It may need to be transformed if there is overlap in SDL
		 @param state_in the state of the event
		 @param controller the source controller
		 */
		void ProcessActionID(int ID, ActionState state_in, CID controller);
		
		/**
		 Buffer an Axis value
		 @param ID the event ID. It may need to be transformed if there is overlap in SDL
		 @param value the axis' value
		 @param controller the source controller
		 */
		void ProcessAxisID(int ID, float value, CID controller);
        
        void ProcessTextInput(const std::string_view str);
		
		/**
		 Purge invalid bindings from the lists
		 */
		void CleanupBindings();
	public:
        InputManager();

		static vector2 GetMousePosPixels(float dpiScaleFactor);
		
		//process axis maps
		void TickAxes();
		
		/**
		 Prcocess an input from SDL
		 */
		void ProcessInput(const SDL_Event&, uint32_t windowflags, float scale, int windowWidth, int windowHeight, float dpiScale);

		/**
		 Create an action mapping entry. Action mappings correspond to items that have two states: pressed and released.
		 @param name the identifer to use when binding or unbinding actions
		 @param Id the button identifier to use. See the SDL key bindings for more information. To bind controllers, see the special bindings at the top of this file.
		 */
        inline void AddActionMap(const std::string& name, int Id){
			CodeToAction[Id].insert(name);
		}
		
		/**
		 Create an axis mapping entry. Axis mappings correspond to items that have a range of values, such as the mouse or analog sticks.
		 @param name the identifer to use when binding or unbinding axes
		 @param Id the button identifier to use. See the SDL key bindings for more information. To bind controllers, see the special bindings at the top of this file.
		 @param scale the scale factor to apply to all bindings mapped to this axis
		 */
        inline void AddAxisMap(const std::string& name, int Id, float scale = 1){
			CodeToAxis[Id].insert({name,scale});
		}
		
		/**
		 Remove an action mapping entry. Both the name and ID must match to complete removal.
		 @param name the identifer to look for
		 @param Id the button identifier to use. See the SDL key bindings for more information.
		 */
        inline void RemoveActionMap(const std::string& name, int Id){
			CodeToAction[Id].erase(name);
		}
		
		/**
		 Remove an axis mapping entry. Both the name and ID must match to complete removal.
		 @param name the identifer to look for
		 @param Id the button identifier to use. See the SDL key bindings for more information.
		 */
        inline void RemoveAxisMap(const std::string& name, int Id, float scale = 1){
			CodeToAxis[Id].erase({name, scale});
		}
		
		/**
		 * Bind an action map to a member function
		 * @param name the name of the action map to bind to
		 * @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 * @param f the method to invoke when the action is triggered. Must take no parameters. Use &Classname::Methodname.
		 * @param type the required state of the action to invoke the method.
		 */
        template<class U,typename T>
        inline void BindAction(const std::string& name, T thisptr, void(U::* f)(), ActionState type, CID controllers){
			auto binding = [=]() mutable{
				(thisptr.get()->*f)();
			};
			ActionBinding ab(thisptr.get_id(),binding,&f,controllers,type);
			
			ActionBindings[name].push_back(ab);
		}

        /**
         Bind a function to an Axis mapping
		 @param name the Axis mapping to bind to
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take one float parameter. Use &Classname::Methodname.
		 @param deadZone the minimum value (+/-) required to activate this binding
         */
        template<typename U, typename T>
		inline void BindAxis(const std::string& name, T thisptr, void(U::* f)(float), CID controllers, float deadZone = 0) {
			auto func = [=](float amt) mutable{
				(thisptr.get()->*f)(amt);
			};
			AxisBinding ab(thisptr.get_id(), func, &f, controllers, deadZone);
			
			AxisBindings[name].bindings.push_back(ab);
        }

		/**
		 Bind a function to an Action mapping
		 @param name the Action mapping to unbind
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take no parameters. Use &Classname::Methodname.
		 @param state the state to use to match the callback
		 */
		template<typename U, typename T>
		inline void UnbindAction(const std::string& name, T thisptr, void(U::* f)(), ActionState type, CID controllers){
			auto binding = [=](){
				(thisptr.get()->*f)();
			};
			ActionBinding ab(thisptr.get_id(),binding,&f,controllers,type);
			
			ActionBindings[name].remove(ab);	//remove the first binding that compares equal
		}
		
		/**
		 Unbind an Axis mapping
		 @param thisptr the object to bind to. Use `this` if within the class you want to bind to
		 @param f the method to invoke when the action is triggered. Must take one float parameter. Use &Classname::Methodname.
		 @param deadZone the minimum value (+/-) required to activate this binding
		 */
		template<typename U, typename T>
		inline void UnbindAxis(const std::string& name, T thisptr, void(U::* f)(float), CID controllers, float deadZone){
			auto func = [=](float amt){
				(thisptr.get()->*f)(amt);
			};
			AxisBinding ab(thisptr.get_id(), func, &f, controllers, deadZone);
			
			AxisBindings[name].bindings.remove(ab);
		}
		
		/**
		 Bind an object to recieve AnyEvents. This will invoke its AnyDown and AnyUp virtual methods
		 */
		inline void BindAnyAction(Ref<IInputListener> listener){
			AnyEventBindings.push_back(listener);
		}
		
		/**
		 Unbind an object to recieve AnyEvents. This is done automatically when an IInputListener is destructed. 
		 */
		inline void UnbindAnyAction(Ref<IInputListener> listener){
			AnyEventBindings.remove(listener);
		}
    };
}
#endif

