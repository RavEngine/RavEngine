#include "StaticMesh.hpp"
#include "Entity.hpp"
#include "mathtypes.hpp"
#include <SDL_stdinc.h>
#include <fstream>
#include <sstream>


using namespace RavEngine;
using namespace std;

StaticMesh::StaticMesh() : Component() {
	/*vb = {
		{{1, 0}, 0xffff0000u},
		{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}, 0xff00ff00u},
		{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}, 0xff0000ffu},
	};

	ib = { 0, 1, 2 };*/

}

RavEngine::StaticMesh::~StaticMesh()
{

}

void RavEngine::StaticMesh::SetMaterial(Ref<MaterialInstance<Material>> mat)
{
	material = mat;
}

void RavEngine::StaticMesh::AddHook(const WeakRef<RavEngine::Entity>& e)
{
}
