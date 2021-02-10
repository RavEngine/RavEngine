#pragma once
#include "RenderableComponent.hpp"
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    public:
        StaticMesh(Ref<MeshAsset>);
		virtual ~StaticMesh(){}
		
		Ref<MeshAsset> getMesh() {return mesh;}

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
		
        /**
        @returns the currently assigned material
        */
        inline Ref<PBRMaterialInstance> GetMaterial() const{
            return material;
        }

    protected:

        //the default material
        Ref<PBRMaterialInstance> material;
		
		Ref<MeshAsset> mesh;
    };
}
