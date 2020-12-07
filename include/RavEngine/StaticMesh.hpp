#pragma once
#include "RenderableComponent.hpp"
#include <vector>
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    public:
        StaticMesh(Ref<MeshAsset>);
		virtual ~StaticMesh(){}

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<PBRMaterialInstance> mat);

		/**
		 Render this Static Mesh
		 @note if there is no material or mesh assigned, no draw will occur.
		 */
        void Draw(int view = 0) override;
		
		template<typename T>
		void Draw(Ref<MaterialInstance<T>> inst, int view = 0){
            static_assert(std::is_base_of<PBRMaterial, T>::value, "StaticMeshes must use a PBR material");
			auto owner = Ref<Entity>(getOwner());
			owner->transform()->Apply();
			inst->Draw(mesh->getVertexBuffer(), mesh->getIndexBuffer(), owner->transform()->GetCurrentWorldMatrix(),view);
		}

        /**
        @returns the currently assigned material
        */
        Ref<PBRMaterialInstance> GetMaterial() {
            return material;
        }

    protected:

        //the default material
        Ref<PBRMaterialInstance> material;
		
		Ref<MeshAsset> mesh;
    };
}
