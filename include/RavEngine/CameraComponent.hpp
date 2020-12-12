#pragma once
#include "Component.hpp"
#include "Entity.hpp"
#include "Queryable.hpp"

namespace RavEngine {
	class RenderEngine;
	class CameraComponent : public Component, public Queryable<CameraComponent> {
	protected:
		friend class RenderEngine;
	public:
		CameraComponent(float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100);

		virtual ~CameraComponent() {}

		/**
		Enable / disable this camera
		@param newState the new enabled state for this camera. The renderer will choose the first active camera as the camera to use when drawing.
		*/
		void setActive(bool newState);

		/**
		@returns if this camera is active
		*/
		inline bool isActive() {
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
		@return the projection matrix to use when rendering objects. For internal use only.
		*/
		inline matrix4 GenerateProjectionMatrix() {
			auto projection = matrix4(glm::perspective(glm::radians(FOV), (float)width / height, nearClip, farClip));
			return projection;
		}

		/**
		@return the View matrix for this camera to use when rendering objects. For internal use only.
		*/
		inline matrix4 GenerateViewMatrix() {
			Ref<Entity> entity(owner);
			
			return glm::inverse(entity->transform()->CalculateWorldMatrix());
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
