#pragma once
#include "RenderableComponent.hpp"
#include <vector>
#include "Material.hpp"

namespace LLGL {
    class Commands;
}

namespace RavEngine {
    class StaticMesh : public RenderableComponent {
    public:
        StaticMesh();
        virtual ~StaticMesh();

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<Material> mat);

        void Draw(LLGL::CommandBuffer*) override;

        /**
        @returns the currently assigned material
        */
        /*Ref<Material> GetMaterial() {
            return material;
        }*/

        void AddHook(const WeakRef<RavEngine::Entity>&) override;

    protected:

        //the default material
        Ref<Material> material;

        LLGL::Buffer* vertexBuffer = nullptr;
        LLGL::Buffer* indexBuffer = nullptr;

        //index and vertex buffers, stores actual data
        //std::vector<Vertex> vb;
        //std::vector<unsigned int> ib;
    };
}