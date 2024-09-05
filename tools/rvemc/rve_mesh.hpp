#pragma once
#include <vector>
#include <array>
#include <glm/mat4x4.hpp>

namespace RavEngine{

struct MeshVertex{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec3 bitangent;
    glm::vec2 uv;
};

struct SerializedMeshDataHeader{
    const std::array<char, 4> header = {'r','v','e','m'};
    uint32_t numVertices = 0;
    uint32_t numIndicies = 0;
};

struct MeshData{
    std::vector<MeshVertex> vetices;
    std::vector<uint32_t> indices;
};

}
