#pragma once
#include "RenderableComponent.hpp"
#include <vector>
#include "Material.hpp"
#include "MeshAsset.hpp"

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    public:
        StaticMesh(Ref<MeshAsset>);
		virtual ~StaticMesh(){}

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<MaterialInstanceBase> mat);

		/**
		 Render this Static Mesh
		 @note if there is no material or mesh assigned, no draw will occur.
		 */
        void Draw() override;

        /**
        @returns the currently assigned material
        */
        Ref<MaterialInstanceBase> GetMaterial() {
            return material;
        }

    protected:

        //the default material
        Ref<MaterialInstanceBase> material;
		
		Ref<MeshAsset> mesh;
    };
}
