#include "Skybox.hpp"
#include "MeshAsset.hpp"

using namespace RavEngine;
using namespace std;

STATIC(Skybox::defaultSkyMesh);

void Skybox::Init() {
	defaultSkyMesh = MeshAsset::Manager::GetMesh("skydome.obj");
}

void Skybox::Teardown() {
	defaultSkyMesh.reset();
}

Skybox::Skybox() : skyMat(std::make_shared<DefaultSkyMaterialInstance>(Material::Manager::Get<DefaultSkyMaterial>())), skyMesh(defaultSkyMesh) {}
