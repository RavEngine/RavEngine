#if !RVE_SERVER
#include "AudioMeshAsset.hpp"
#include "Debug.hpp"
#include "MeshAsset.hpp"
#include "AudioPlayer.hpp"
#include "App.hpp"
#include <phonon.h>
#include <vector>

namespace RavEngine {
	AudioMeshAsset::AudioMeshAsset(Ref<MeshAsset> mesh) : staticMesh(nullptr)
	{
		Debug::Assert(mesh->hasSystemRAMCopy(), "MeshAsset does not have system RAM data");
		Debug::AssertSize<IPLint32>(mesh->GetNumVerts(), "Mesh has too many vertices");
		Debug::AssertSize<IPLint32>(mesh->GetNumIndices(), "Mesh has too many vertices");
		
		const auto& meshCPU = mesh->GetSystemCopy();

		std::vector<IPLVector3> vertices;
		{
			vertices.reserve(mesh->GetNumVerts());
			for (const auto& vert : meshCPU.vertices) {
				vertices.push_back(IPLVector3{
					.x = vert.position[0],
					.y = vert.position[1],
					.z = vert.position[2]
				});
			}
		}

		std::vector<IPLint32> indices;
		{
			indices.reserve(mesh->GetNumIndices());
			for (const auto& index : meshCPU.indices) {
				indices.push_back(index);
			}
		}

		// TODO: replace hardcoded data with parameters
		IPLMaterial materials[] = {
			{0.1f, 0.1f, 0.1f}
		};

		std::vector<IPLint32> materialIndices;
		materialIndices.resize(indices.size() / 3);	// triangle count
		for (auto& i : materialIndices) {
			i = 0;	// set all to use material 0
		}

		IPLStaticMeshSettings staticMeshSettings{
			.numVertices = IPLint32(vertices.size()),
			.numTriangles = IPLint32(indices.size() / 3),
			.numMaterials = std::size(materials),
			.vertices = vertices.data(),
			.triangles = reinterpret_cast<IPLTriangle*>(indices.data()),
			.materialIndices = materialIndices.data(),
			.materials = materials
		};

		IPLSceneSettings sceneSettings{
			.type = IPL_SCENETYPE_DEFAULT
		};
		auto context = GetApp()->GetAudioPlayer()->GetSteamAudioContext();
		iplSceneCreate(context, &sceneSettings, &iplscene);

		iplStaticMeshCreate(iplscene, &staticMeshSettings, &staticMesh);
	}

	AudioMeshAsset::~AudioMeshAsset()
	{
		iplStaticMeshRelease(&staticMesh);
		iplSceneRelease(&iplscene);
	}

}
#endif