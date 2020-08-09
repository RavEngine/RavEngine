
#include "Component.hpp"
#include <vector>
#include "Material.hpp"

struct PosColorVertex
{
    float x;
    float y;
    float z;
    uint32_t abgr;
};


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

    void Draw();

protected:
    //std::vector<PosColorVertex> vertices;
    //std::vector<int> triList;

    //the default material
    Ref<Material> material;
};