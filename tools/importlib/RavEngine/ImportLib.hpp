#pragma once

class aiMesh;

namespace RavEngine{
struct MeshPart;

    /**
    Convert an assimp mesh to a MeshPart
    @param mesh the assimp mesh to convert
    @param scaleMat the matrix to apply to each vertex of the mesh
    @return converted MeshPart
    */
    MeshPart AIMesh2MeshPart(const aiMesh* mesh, const matrix4& scaleMat);

}
