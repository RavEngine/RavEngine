#pragma once
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"
#include "mathtypes.hpp"
#include "Transform.hpp"
#include "ComponentWithOwner.hpp"

namespace RavEngine {
    class StaticMesh : public ComponentWithOwner, public Disableable{
    private:
        std::tuple<Ref<MeshAsset>, Ref<MaterialInstance>> tuple;
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
			return std::get<0>(tuple);
		}

        inline void SetMesh(Ref<MeshAsset> m) {
            std::get<0>(tuple) = m;
            //TODO: notify world render datastructure of this change
        }

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(Ref<MaterialInstance> mat){
            updateMaterialInWorldRenderData(mat);
			std::get<1>(tuple) = mat;
		}

        /**
        @returns the currently assigned material
        */
        inline auto GetMaterial() const{
			return std::get<1>(tuple);
        }
		
        constexpr inline const auto& getTuple() const{
			return tuple;
		}
		
		// shadow Disableable::SetEnabled
		void SetEnabled(bool);
    };
}
