#pragma once
#include <queue>

namespace RGL {

	template<typename index_t, uint32_t totalCount>
	struct FreeList {

		index_t Allocate() {
			index_t allocatedIndex = 0;
			if (freeList.empty()) {
				if (nextFreeIndexNotInQueue == totalCount) {
					// full -- cannot allocate!
					throw std::out_of_range("Descriptor heap is full!");
				}
				// place the descriptor at the end
				allocatedIndex = nextFreeIndexNotInQueue;
				nextFreeIndexNotInQueue++;
			}
			else {
				// fill the hole
				allocatedIndex = freeList.front();
				freeList.pop();
			}

			return allocatedIndex;
		}

		void Deallocate(index_t index) {
			// if the index was the end, then decrement end
			if (index == nextFreeIndexNotInQueue) {
				nextFreeIndexNotInQueue--;
			}
			else {
				// add the index to the free list
				freeList.emplace(index);
			}
		}

	private:
		std::queue<index_t> freeList{};
		index_t nextFreeIndexNotInQueue = 0;	// __NOT__ one past the end
	};

}