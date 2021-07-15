#pragma once
#include "RenderableComponent.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    protected:
		std::tuple<Ref<MeshAsset>, Ref<PBRMaterialInstance>> tuple;
    public:
		StaticMesh(Ref<MeshAsset> m){
			std::get<0>(tuple) = m;
		}
        StaticMesh(Ref<MeshAsset> m, Ref<PBRMaterialInstance> mat) : StaticMesh(m) {
			std::get<1>(tuple) = mat;
		}
		virtual ~StaticMesh(){}
		
		Ref<MeshAsset> getMesh() {
			return std::get<0>(tuple);
		}

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(Ref<PBRMaterialInstance> mat){
			std::get<1>(tuple) = mat;
		}

        /**
        @returns the currently assigned material
        */
        inline Ref<PBRMaterialInstance> GetMaterial() const{
			return std::get<1>(tuple);
        }
		
		const auto& getTuple() const{
			return tuple;
		}
    };
}
