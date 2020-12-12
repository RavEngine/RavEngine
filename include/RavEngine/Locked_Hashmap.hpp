#pragma once

#include <mutex>
#include <phmap.h>

template<typename T, typename U, typename lock=std::mutex>
using locked_hashmap = phmap::parallel_flat_hash_map<T,U, phmap::container_internal::hash_default_hash<T>, phmap::container_internal::hash_default_eq<T>, phmap::container_internal::Allocator<std::pair<const T, U>>, 4, lock>;

template<typename T, typename lock=std::mutex>
using locked_hashset = phmap::parallel_flat_hash_set<T, phmap::container_internal::hash_default_hash<T>, phmap::container_internal::hash_default_eq<T>, phmap::container_internal::Allocator<T>,4,lock>;
