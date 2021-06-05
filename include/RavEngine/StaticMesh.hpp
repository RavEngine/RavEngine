#pragma once
#include "RenderableComponent.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    protected:
        //the default material
        Ref<PBRMaterialInstance> material;

        Ref<MeshAsset> mesh;
    public:
		StaticMesh(decltype(mesh) m): RenderableComponent(), mesh(m){}
        StaticMesh(decltype(mesh) m, decltype(material) mat) : mesh(m), material(mat) {}
		virtual ~StaticMesh(){}
		
		Ref<MeshAsset> getMesh() {return mesh;}

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(Ref<PBRMaterialInstance> mat){
			material = mat;
		}

        /**
        @returns the currently assigned material
        */
        inline Ref<PBRMaterialInstance> GetMaterial() const{
            return material;
        }
    };
}
