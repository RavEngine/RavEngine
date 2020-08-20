
#include "SharedObject.hpp"
#include <unordered_map>
#include <mutex>

namespace RavEngine {
	/**
	Represents an instance of a material. Instances can be changed independently of one another, and are assigned to objects
	*/
	class MaterialInstance : public SharedObject {

	};

	/**
	Represents the interface to a shader. Subclass to create more types of material and expose more abilities.
	*/
	class Material : public SharedObject {
	public:
		

		/**
		Create the default material. Override this constructor in subclasses, and from that, invoke the protected constructor.
		*/
		Material();

		const std::string& GetName() {
			return name;
		}

	protected:
		std::string name;

		//create a material from a filament shader
		//trying to create a material that already exists will throw an exception
		Material(const std::string&, const std::string&);
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
		static void RegisterMaterial(Ref<Material>);
	};
}
