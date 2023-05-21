#pragma once
#include "DataStructures.hpp"
namespace RavEngine {
	using allocation_freelist_t = LinkedList<Range>;
	using allocation_allocatedlist_t = allocation_freelist_t;

	struct MeshRange {
		allocation_allocatedlist_t::iterator vertRange, indexRange;
	};
}