#pragma once
#include "Component.hpp"
#include "Transform.hpp"

namespace RavEngine {
	class World;
	/**
	Define an Entity-side Script which can contain code. Subclass to add behavior. Be sure to invoke the base class constructor!
	*/
	class ScriptComponent : public Component {
	protected:
		void RegisterAllAlternateTypes() override{
			RegisterAlternateQueryType<ScriptComponent>();
		}
	public:

		/**
		Ensure this is invoked in subclasses.
		*/
		ScriptComponent() : Component() {
			RegisterAllAlternateTypes();
		};

		/**
		Override to provide cleanup behavior.
		*/
		virtual ~ScriptComponent() {}

		/**
		Invoked when the owning entity of this script has been added to the world.
		*/
		virtual void Start() {};

		/**
		Called by the world when the owning entity has been despawned, but before despawn work has begun. It may not be deallocated immediately after depending on reference counts. However it is best to stop or destroy anything possible.
		*/
		virtual void Stop() {}

		/**
		Invoked as the last step of the systems execution on a background thread. Any cross-object access must be appropriately protected.
		@param fpsScale the frame rate scalar for this frame.
		*/
		virtual void Tick(float fpsScale) = 0;

		/**
		Mark the current entity for destruction. It will be destroyed after the current frame.
		@return true if the entity was successfully marked for destruction, false otherwise
		*/
		bool Destroy();

		/**
		 Check if the owning Entity has been spawned
		 @return true if the entity is in the world, false otherwise
		 */
		bool IsInWorld();

		/**
		@return true if the current ScriptComponent is attached to an Entity, false otherwise.
		*/
		bool IsAttached() {
			return !getOwner().isNull();
		}

		/**
		Shortcut to get the transform component of the attached entity
		@throws if the script is not attached to any entity.
		*/
		Ref<Transform> transform();

		/**
		Get the current world for the attached entity
		*/
		Ref<World> GetWorld();
	};
}
