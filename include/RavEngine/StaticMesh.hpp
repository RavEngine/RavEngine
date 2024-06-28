#pragma once
#if !RVE_SERVER

#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"
#include "mathtypes.hpp"
#include "Transform.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {
    struct MeshCollectionStatic;

    class StaticMesh : public ComponentWithOwner, public Disableable{
    private:
        Ref<MeshCollectionStatic> mesh;
        Ref<MaterialInstance> material;
        StaticMesh(entity_t owner, decltype(mesh) m) : ComponentWithOwner(owner){
            SetMesh(m);
        }
        void updateMaterialInWorldRenderData(decltype(material) mat);
        inline void SetMesh(decltype(mesh) m) {
            mesh = m;
            //TODO: notify world render datastructure of this change
        }
    public:

        StaticMesh(entity_t owner, Ref<MeshCollectionStatic> m, decltype(material) mat) : StaticMesh(owner, m) {
            SetMaterial(mat);
		}
        
		virtual ~StaticMesh(){}
		
		inline auto GetMesh() const{
            return mesh;
		} 

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(decltype(material) mat){
            updateMaterialInWorldRenderData(mat);
			material = mat;
		}

        /**
        @returns the currently assigned material
        */
        inline auto GetMaterial() const{
            return material;
        }
    
		
		// shadow Disableable::SetEnabled
		void SetEnabled(bool);
    };
}
#endif
