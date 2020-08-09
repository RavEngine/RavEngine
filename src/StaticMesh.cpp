#include "StaticMesh.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"

//default cube
static PosColorVertex vertices[] = {
     {-1.0f,  1.0f,  1.0f, 0xff000000 },
     { 1.0f,  1.0f,  1.0f, 0xff0000ff },
     {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
     { 1.0f, -1.0f,  1.0f, 0xff00ffff },
     {-1.0f,  1.0f, -1.0f, 0xffff0000 },
     { 1.0f,  1.0f, -1.0f, 0xffff00ff },
     {-1.0f, -1.0f, -1.0f, 0xffffff00 },
     { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static int triList[] = {
     0, 1, 2,
     1, 3, 2,
     4, 6, 5,
     5, 6, 7,
     0, 2, 4,
     4, 2, 6,
     1, 5, 3,
     5, 7, 3,
     0, 4, 1,
     4, 5, 1,
     2, 3, 6,
     6, 3, 7,
};

StaticMesh::StaticMesh() : Component(), material(new Material()) {
   //make vertex declaration
 /*  pcvDecl.begin()
       .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
       .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
       .end();

   vbh = bgfx::createVertexBuffer(bgfx::makeRef(vertices, sizeof(vertices)), pcvDecl);
   ibh = bgfx::createIndexBuffer(bgfx::makeRef(triList, sizeof(triList)));*/
}

void StaticMesh::Draw() {
    //bgfx::setVertexBuffer(0, vbh);
    //bgfx::setIndexBuffer(ibh);

    ////convert transform to bgfx matrix
    //auto owning = Ref<Entity>(owner);
    //auto transform = owning->transform();
    //float matrix[16];
    //transform->WorldMatrixToArray(matrix);

    //bgfx::setTransform(matrix);

    //render the object with its material
    GetMaterial()->Submit();
}