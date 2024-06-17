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
        MeshMaterial material;
        StaticMesh(entity_t owner, decltype(mesh) m) : ComponentWithOwner(owner){
            SetMesh(m);
        }
        void updateMaterialInWorldRenderData(MeshMaterial mat);
        inline void SetMesh(decltype(mesh) m) {
            mesh = m;
            //TODO: notify world render datastructure of this change
        }
    public:

        template<typename T> requires std::is_same_v<T, LitMeshMaterialInstance> || std::is_same_v<T, UnlitMeshMaterialInstance>
        StaticMesh(entity_t owner, Ref<MeshCollectionStatic> m, T mat) : StaticMesh(owner, m) {
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
        template<typename T> requires std::is_same_v<T, LitMeshMaterialInstance> || std::is_same_v<T, UnlitMeshMaterialInstance>
		inline void SetMaterial(T mat){
            bool isLit = std::is_same_v<T, LitMeshMaterialInstance>;
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
