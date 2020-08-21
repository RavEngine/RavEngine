#include "Component.hpp"
#include <vector>
#include "Material.hpp"
#include <utils/Entity.h>
#include <math/mat3.h>

namespace filament {
    class VertexBuffer;
    class IndexBuffer;
}


struct Vertex {
    filament::math::float3 position;
};

namespace RavEngine {
    class StaticMesh : public Component {
    public:
        StaticMesh();
        virtual ~StaticMesh();

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<MaterialInstance<Material>> mat);

        /**
        @returns the currently assigned material
        */
        /*Ref<Material> GetMaterial() {
            return material;
        }*/

        void AddHook(const WeakRef<RavEngine::Entity>&) override;

    protected:

        //the default material
        Ref<MaterialInstance<Material>> material;

        //index and vertex buffers, stores actual data
        std::vector<Vertex> vb;
        std::vector<unsigned int> ib;

        //stores pointers into vectors for rendering (does not duplicate data)
        filament::VertexBuffer* fvb = nullptr;
        filament::IndexBuffer* fib = nullptr;

        utils::Entity renderable;
    };
}