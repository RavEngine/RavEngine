#pragma once

#include <mutex>
#include <phmap.h>
#include <concurrentqueue.h>

namespace RavEngine{
	template<typename T, typename U, typename lock=std::mutex, typename hash = phmap::container_internal::hash_default_hash<T>, typename eq = phmap::container_internal::hash_default_eq<T>>
	using locked_hashmap = phmap::parallel_flat_hash_map<T,U, hash, eq, phmap::container_internal::Allocator<std::pair<const T, U>>, 4, lock>;

	template<typename T, typename lock=std::mutex, typename hash = phmap::container_internal::hash_default_hash<T>, typename eq = phmap::container_internal::hash_default_eq<T>>
	using locked_hashset = phmap::parallel_flat_hash_set<T, hash, eq, phmap::container_internal::Allocator<T>,4,lock>;
	
	template<typename T>
	using ConcurrentQueue = moodycamel::ConcurrentQueue<T>;
}
