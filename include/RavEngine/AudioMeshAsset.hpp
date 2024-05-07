#pragma once
#include "Ref.hpp"

struct _IPLStaticMesh_t;
struct _IPLScene_t;

namespace RavEngine {
	class MeshAsset;

	struct AudioMeshAsset {
		AudioMeshAsset(Ref<MeshAsset> mesh);
		~AudioMeshAsset();
		auto GetRadius() const {
			return radius;
		}
		auto GetScene() const {
			return iplscene;
		}
	private:
		_IPLStaticMesh_t* staticMesh = nullptr;
		_IPLScene_t* iplscene = nullptr;
		float radius = 0;
	};

}