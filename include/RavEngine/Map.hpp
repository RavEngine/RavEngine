#pragma once
#include <mutex>
#include <phmap.h>

namespace RavEngine {
    template<typename T, typename U, typename lock=std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
    using locked_hashmap = phmap::parallel_flat_hash_map<T,U, hash, eq, phmap::priv::Allocator<std::pair<const T, U>>, 4, lock>;

    template<typename T, typename U, typename lock = std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
    using locked_node_hashmap = phmap::parallel_node_hash_map<T, U, hash, eq, phmap::priv::Allocator<std::pair<const T, U>>, 4, lock>;

    template<typename T, typename lock=std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
    using locked_hashset = phmap::parallel_flat_hash_set<T, hash, eq, phmap::priv::Allocator<T>,4,lock>;

    template<typename T, typename lock = std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
    using locked_node_hashset = phmap::parallel_node_hash_set<T, hash, eq, phmap::priv::Allocator<T>, 4, lock>;

    template<typename T, typename U>
    using UnorderedMap = phmap::flat_hash_map<T,U>;

    template<typename T, typename U>
    using UnorderedNodeMap = phmap::node_hash_map<T,U>;

    template<typename T>
    using UnorderedSet = phmap::flat_hash_set<T>;
}
