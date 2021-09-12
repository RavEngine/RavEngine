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
		void SetActive(bool newState);

		/**
		@returns if this camera is active
		*/
		inline bool IsActive() const{
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
		inline matrix4 GenerateProjectionMatrix() const{
			switch(projection){
				case Mode::Perspective:
					return matrix4(glm::perspective(glm::radians(FOV), (float)width / height, nearClip, farClip));
					break;
				case Mode::Orthographic:
					return matrix4(glm::ortho(0.0f,static_cast<float>(width),static_cast<float>(height),0.0f,nearClip,farClip));
					break;
			}
		}

		/**
		@return the View matrix for this camera to use when rendering objects. For internal use only.
		*/
		inline matrix4 GenerateViewMatrix() const{
			Ref<Entity> entity(owner);
			
			return glm::inverse(entity->GetTransform()->CalculateWorldMatrix());
		}
        
        /**
         Project a screen point to a worldspace coordinate
         @param x x pos of point in [0,1]] space
         @param y y pos of point in [0,1] space
         @param z depth of the point from the camera
         @return vector3 representing world-space projected point
         */
        inline vector3 ScreenPointToWorldPoint(float x, float y, float z) const{
            auto projmat = GenerateProjectionMatrix();
            auto viewmat = GenerateViewMatrix();
            auto G = glm::inverse(projmat * viewmat);
            return glm::unProject(vector3(x * width,y * height,z), G, projmat, vector4(0,0,width,height));
        }
		
		//camera details
		float FOV;
		float nearClip;
		float farClip;

	protected:
		bool active = false;

		unsigned int width = 800;
		unsigned int height = 480;

		Mode projection = Mode::Perspective;
	};
}
