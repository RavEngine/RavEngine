#include "Skybox.hpp"
#include "MeshAsset.hpp"

using namespace RavEngine;
using namespace std;

STATIC(SkyBox::defaultSkyMesh);

void SkyBox::Init() {
	defaultSkyMesh = make_shared<MeshAsset>("skydome.obj");
}

void SkyBox::Teardown() {
	defaultSkyMesh.reset();
}

SkyBox::SkyBox() : skyMat(std::make_shared<DefaultSkyMaterialInstance>(Material::Manager::AccessMaterialOfType<DefaultSkyMaterial>())), skyMesh(defaultSkyMesh) {}