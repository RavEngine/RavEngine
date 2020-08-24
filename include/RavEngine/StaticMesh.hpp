#pragma once
#include "Component.hpp"
#include <vector>
#include "Material.hpp"


struct Vertex {
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
    };
}