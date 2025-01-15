#pragma once
#include "Ref.hpp"

struct _IPLStaticMesh_t;
struct _IPLScene_t;

namespace RavEngine {
	class MeshAsset;

	/**
	An asset that enables sound reverbation modeling based on geometry data.
	*/
	struct AudioMeshAsset {
		/**
		@note mesh must have a host-memory copy of the geometry data (not the default)
		*/
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