#pragma once
#include "DataStructures.hpp"
#include "Mesh.hpp"

namespace RavEngine {
	using allocation_freelist_t = LinkedList<Range>;
	using allocation_allocatedlist_t = allocation_freelist_t;

	struct MeshRange {
	private:
		allocation_allocatedlist_t::iterator vertRange, indexRange;
		struct typedRange {
			allocation_allocatedlist_t::iterator iter;
		};

	public:
		MeshRange(const decltype(vertRange)& vr, const decltype(indexRange)& ir) : vertRange(vr), indexRange(ir) {}
		MeshRange(){}

		auto getVertRange() const {
			return vertRange;
		}
		auto getIndexRange() const {
			return indexRange;
		}

		uint32_t getIndexRangeStart() const {
			return indexRange->start;
		}

		uint32_t getIndexRangeByteStart() const {
			return indexRange->start * sizeof(uint32_t);
		}

		uint32_t getVertexRangeStart() const {
			return vertRange->start;
		}

		uint32_t getPositionByteStart() const {
			return vertRange->start * sizeof(VertexPosition_t);
		}
	};
}