
#include "Component.hpp"
#include <vector>
#include "Material.hpp"

namespace filament {
    class VertexBuffer;
    class IndexBuffer;
}

namespace RavEngine {
    class StaticMesh : public Component {
    public:
        StaticMesh();

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
        void SetMaterial(Ref<Material> mat) {
            material = mat;
        }

        /**
        @returns the currently assigned material
        */
        Ref<Material> GetMaterial() {
            return material;
        }

    protected:
        //std::vector<PosColorVertex> vertices;
        //std::vector<int> triList;

        //the default material
        Ref<Material> material;
    };
}