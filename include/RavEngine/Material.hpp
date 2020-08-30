#pragma once
#include "SharedObject.hpp"
#include "RenderEngine.hpp"
#include <unordered_map>
#include <mutex>
#include "Gauss/Gauss.h"

namespace LLGL {
	class CommandBuffer;
	class Buffer;
	class ResourceHeap;
	class PipelineState;
}

namespace RavEngine {
	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public SharedObject {
	public:
		/**
		Create the default material. Override this constructor in subclasses, and from that, invoke the protected constructor.
		*/
		Material();

		~Material();

		const std::string& GetName() {
			return name;
		}

		/**
		Set the world space matrix to use when rendering this material
		*/
		void SetTransformMatrix(const matrix4&);

		/**
		Enqueue commands to execute on the GPU
		@param commands the command buffer to write to
		*/
		void Draw(LLGL::CommandBuffer* const commands, LLGL::Buffer* vertexBuffer, LLGL::Buffer* indexBuffer);

	protected:
		std::string name;

		//trying to create a material that already exists will throw an exception
		Material(const std::string& name, const std::string& vertShader, const std::string& fragShader);

		LLGL::ResourceHeap* resourceHeap = nullptr;
		LLGL::PipelineState* pipeline = nullptr;
		LLGL::Buffer* constantBuffer = nullptr;

		matrix4 transformMatrix;

		struct Settings {
			Gs::Matrix4f wvpMatrix; //todo: 16 byte pack alignment for constant buffers
		} settings;

	};

	/**
	Static singleton for managing materials
	*/
	class MaterialManager {
	protected:
		//materials are keyed by their shader name
		typedef std::unordered_map<std::string, Ref<RavEngine::Material>> MaterialStore;
		static MaterialStore materials;
		static std::mutex mtx;

		static matrix4 projectionMatrix;
	public:
		/**
		Gets a material with a given name, casted to a particular type.
		@param name the name of the material to query 
		@note Undefined Behavior occurs if the template parameter does not match the returned material or any of its base classes
		*/
		template<class T>
		static Ref<T> GetMaterialByName(const std::string& name) {
			mtx.lock();
			auto mat = materials.at(name);
			mtx.unlock();
			return mat;
		}
		static bool HasMaterialByName(const std::string&);
		static void UnregisterMaterialByName(const std::string&);

		//for internal use only
		static void RegisterMaterial(Ref<RavEngine::Material>);

		/**
		Set the current projection matrix. For internal use only.
		*/
		static void SetProjectionMatrix(const matrix4& mat) {
			projectionMatrix = mat;
		}

		/**
		Get a const-reference to the current global projection matrix
		For internal use only.
		*/
		static const matrix4& GetCurrentProjectionMatrix() {
			return projectionMatrix;
		}
	};
}
