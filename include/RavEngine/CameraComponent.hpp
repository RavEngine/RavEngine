#include "Component.hpp"
#include "Entity.hpp"
#include "GameplayStatics.hpp"

namespace RavEngine {
	class CameraComponent : public Component {
	public:
		CameraComponent(float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100);

		virtual ~CameraComponent() {}

		void AddHook(const WeakRef<RavEngine::Entity>& e) override;

		void RegisterAllAlternateTypes() override {}

		/**
		Enable / disable this camera
		@param newState the new enabled state for this camera. The renderer will choose the first active camera as the camera to use when drawing.
		*/
		void setActive(bool newState);

		/**
		@returns if this camera is active
		*/
		bool isActive() {
			return active;
		}

		/**
		Set the size of the camera. This will recalculate its projection.
		@param width the width of the target, in pixels
		@param height the height of the target, in pixels
		*/
		void SetTargetSize(unsigned int inwidth, unsigned int inheight);

		enum class Mode {
			Perspective,
			Orthographic
		};

		/**
		@return the projection matrix to use when rendering objects
		*/
		matrix4 GenerateCameraMatrix() {
			Ref<Entity> entity = owner.get();
			auto transform = entity->transform();

			//negate to convert world matrix to camera space
			auto pos = - transform->GetWorldPosition();
			auto rot = glm::inverse(transform->GetWorldRotation());

			auto projection = glm::perspective(glm::radians(FOV), (float)width / height, nearClip, farClip);
			projection *= (glm::translate(matrix4(), pos) * glm::toMat4(rot));

			return projection;
		}

	protected:
		bool active = false;

		//camera details
		float FOV;
		float nearClip;
		float farClip;
		float zoom = 1.5;

		unsigned int width = 800;
		unsigned int height = 480;

		Mode projection = Mode::Perspective;
	};
}
