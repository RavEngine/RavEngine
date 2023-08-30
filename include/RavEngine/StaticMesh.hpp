#pragma once
#if !RVE_SERVER

#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"
#include "mathtypes.hpp"
#include "Transform.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {
    class StaticMesh : public ComponentWithOwner, public Disableable{
    private:
        Ref<MeshAsset> mesh;
        Ref<MaterialInstance> material;
        StaticMesh(entity_t owner, Ref<MeshAsset> m) : ComponentWithOwner(owner){
            SetMesh(m);
        }
        void updateMaterialInWorldRenderData(Ref<MaterialInstance> to);
    public:

        StaticMesh(entity_t owner, Ref<MeshAsset> m, Ref<MaterialInstance> mat) : StaticMesh(owner, m) {
            SetMaterial(mat);
		}
		virtual ~StaticMesh(){}
		
		inline Ref<MeshAsset> GetMesh() const{
            return mesh;
		}

        inline void SetMesh(Ref<MeshAsset> m) {
            mesh = m;
            //TODO: notify world render datastructure of this change
        }

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(Ref<MaterialInstance> mat){
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
