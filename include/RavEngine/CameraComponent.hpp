#pragma once
#include "Entity.hpp"
#include "Transform.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {
	class RenderEngine;
	class CameraComponent : public ComponentWithOwner, public Disableable {
	protected:
		friend class RenderEngine;
	public:
        CameraComponent(entity_t owner, float inFOV = 60, float inNearClip = 0.1, float inFarClip = 100) : FOV(inFOV), nearClip(inNearClip), farClip(inFarClip), ComponentWithOwner(owner){}

		virtual ~CameraComponent() {}

		/**
		Enable / disable this camera
		@param newState the new enabled state for this camera. The renderer will choose the first active camera as the camera to use when drawing.
		*/
        constexpr inline void SetActive(bool newState){
            active = newState;
        }

		/**
		@returns if this camera is active
		*/
		constexpr inline bool IsActive() const{
			return active;
		}

		enum class Mode {
			Perspective,
			Orthographic
		};

		/**
		@return the projection matrix to use when rendering objects. For internal use only.
		*/
		constexpr inline matrix4 GenerateProjectionMatrix(uint32_t width, uint32_t height) const{
			switch(projection){
				case Mode::Perspective:
					return matrix4(glm::perspective(deg_to_rad(FOV), (float)width / height, nearClip, farClip));
					break;
				case Mode::Orthographic:
					return matrix4(glm::ortho(0.0f,static_cast<float>(width),static_cast<float>(height),0.0f,nearClip,farClip));
					break;
                default:
                    assert(false);
                    return matrix4(1);  // this should never happen
			}
		}

		/**
		@return the View matrix for this camera to use when rendering objects. For internal use only.
		*/
		inline matrix4 GenerateViewMatrix() const{
			
            return glm::inverse(GetOwner().GetTransform().CalculateWorldMatrix());
		}
        
        /**
         Project a screen point to a worldspace coordinate
         @param point in pixel coordinates
         @return vector3 representing world-space projected point
         */
        inline vector3 ScreenPointToWorldPoint(const vector3& point, uint32_t width, uint32_t height) const{
            auto projmat = GenerateProjectionMatrix(width, height);
            auto viewmat = GenerateViewMatrix();
            auto G = glm::inverse(projmat * viewmat);
            return glm::unProject(point, G, projmat, vector4(0,0,width,height));
        }

		/**
		 Project a screen point to a worldspace coordinate
		 @param point in [0,1] space
		 @return vector3 representing world-space projected point
		 */
		inline vector3 NormalizedScreenPointToWorldPoint(const vector3& point, uint32_t width, uint32_t height) const {
			return ScreenPointToWorldPoint(vector3(point.x * width, point.y * height, point.z), width, height);
		}

		inline std::pair<vector3,vector3> ScreenPointToRay(vector2 point, uint32_t width, uint32_t height) const {
			vector2 scaledWindowsSize = vector2(width, height);
			point.y = height - point.y;	// invert Y
			point = (point / scaledWindowsSize ) * 2.0f - vector2(1.0f);

			auto viewProjection = GenerateProjectionMatrix(width, height) * GenerateViewMatrix();
			auto invViewProjection = glm::inverse(viewProjection);

			vector4 originClipSpace{ point,-1,1 };
			vector4 destClipSpace{ point,1,1 };
			vector4 originClipSpaceWS = invViewProjection * originClipSpace;
			vector4 destClipSpaceWS = invViewProjection * destClipSpace;
			vector3 originClipSpaceWS3 = vector3(originClipSpaceWS) / originClipSpaceWS.w;
			vector3 destClipSpaceWS3 = vector3(destClipSpaceWS) / destClipSpaceWS.w;

			return { originClipSpaceWS3, glm::normalize(destClipSpaceWS3 - originClipSpaceWS3) };
		}
		
		//camera details
		float FOV;
		float nearClip;
		float farClip;

	protected:
        Mode projection = Mode::Perspective;
		bool active = false;
	};
}
