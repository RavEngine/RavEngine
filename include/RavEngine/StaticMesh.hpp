#pragma once
#include "RenderableComponent.hpp"
#include <vector>
#include "Material.hpp"
#include <bgfx/bgfx.h>

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    public:
        StaticMesh();
        virtual ~StaticMesh();

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<MaterialInstanceBase> mat);

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
		
		bgfx::VertexBufferHandle vertexBuffer;
		bgfx::IndexBufferHandle indexBuffer;
    };
}
