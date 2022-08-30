#pragma once
#include <mutex>
#include <phmap.h>
#include <concurrentqueue.h>
#include <boost/container/vector.hpp>
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/array.hpp>
#include <vector>
#include <plf_list.h>
#include <plf_colony.h>
#include "unordered_vector.hpp"
#include <queue>

namespace RavEngine{
	template<typename T, typename U, typename lock=std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
	using locked_hashmap = phmap::parallel_flat_hash_map<T,U, hash, eq, phmap::priv::Allocator<std::pair<const T, U>>, 4, lock>;

	template<typename T, typename U, typename lock = std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
	using locked_node_hashmap = phmap::parallel_node_hash_map<T, U, hash, eq, phmap::priv::Allocator<std::pair<const T, U>>, 4, lock>;

	template<typename T, typename lock=std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
	using locked_hashset = phmap::parallel_flat_hash_set<T, hash, eq, phmap::priv::Allocator<T>,4,lock>;

	template<typename T, typename lock = std::mutex, typename hash = phmap::priv::hash_default_hash<T>, typename eq = phmap::priv::hash_default_eq<T>>
	using locked_node_hashset = phmap::parallel_node_hash_set<T, hash, eq, phmap::priv::Allocator<T>, 4, lock>;
	
	template<typename T>
	using ConcurrentQueue = moodycamel::ConcurrentQueue<T>;

    template<typename T>
    using Vector = boost::container::vector<T>;

    template<typename T, typename U>
    using UnorderedMap = phmap::flat_hash_map<T,U>;

    template<typename T, typename U>
    using UnorderedNodeMap = phmap::node_hash_map<T,U>;

    template<typename T>
    using UnorderedSet = phmap::flat_hash_set<T>;

    template<typename T,size_t N>
    using SmallVector = boost::container::small_vector<T,N>;

    template<typename T,size_t N>
    using FixedVector = boost::container::static_vector<T,N>;

    template<typename T>
    using LinkedList = plf::list<T>;

    template<typename T>
    using Colony = plf::colony<T>;

    template<typename T, size_t N>
    using Array = std::array<T,N>;

    template<typename T>
    using UnorderedVector = unordered_vector<T,Vector<T>>;
    
    template<typename T>
    using UnorderedContiguousSet = unordered_contiguous_set<T,Vector<T>>;

    template<typename T>
    using Queue = std::queue<T>; 

// The stackarray creates a stack-resident array using a runtime-known size.
// There are no safety checks for overflowing the stack, and overflowing results in undefined behavior.
// Only use for small sizes. For larger sizes, use the maybestackarray instead
#if defined __APPLE__ || __STDC_VERSION__ >= 199901L    //check for C99
#define stackarray(name, type, size) type name[size]    //prefer C VLA on supported systems
#else
#define stackarray(name, type, size) type* name = (type*)alloca(sizeof(type) * size) //warning: alloca may not be supported in the future
#endif

template<typename T>
struct maybestackarray_freer {
    T* ptr = nullptr;
    ~maybestackarray_freer() {
        if (ptr != nullptr){
            delete[] ptr;
        }
    }
};

//The Maybestackarray creates an array using a runtime-known size.
// It will use stack if it can fit inside a threshold, and use heap memory if it does not.
// Do not call `delete` on this, the structure will free itself when its scope ends
#define maybestackarray(name, type, size) \
type* name;\
maybestackarray_freer<type> name ## _freer; \
if (size * sizeof(type) < 1500) name = (type*)alloca(sizeof(type) * size);\
else { name = new type[size];  name ## _freer.ptr = name; }
}
