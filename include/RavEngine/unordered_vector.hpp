#pragma once
#include <vector>
#include <phmap.h>
#include <functional>

namespace RavEngine {

/**
 The Unordered Vector provides:
 - O(1) erase by iterator
 All other complexities for valid behaviors are identical to a regular vector. Elements must be moveable.
 Note that the order of elements cannot be guareneed.
 */
template<typename T, typename vec = std::vector<T>>
class unordered_vector{
    vec underlying;
public:
    typedef typename decltype(underlying)::iterator iterator_type;
    typedef typename decltype(underlying)::const_iterator const_iterator_type;
    typedef typename decltype(underlying)::size_type index_type;
    typedef typename decltype(underlying)::size_type size_type;

    /**
     Erase by iterator. Complexity is O(1).
     @param it the iterator to erase
     */
    inline void erase(iterator_type it){
        *it = std::move(underlying.back());
        underlying.pop_back();
	}
	
    /**
     Erase by iterator. Complexity is O(n)
     @param value the data to erase
     */
	inline void erase(const T& value){
        auto it = std::find(underlying.begin(),underlying.end(),value);
        underlying.erase(it);
	}
	
    /**
     Add an item to the container
     @param value the data to add
     */
	inline void insert(const T& value){
        underlying.push_back(value);
	}
    
    inline T& operator[](index_type idx){
        return underlying[idx];
    }
    
    const inline T& operator[](index_type idx) const{
        return underlying[idx];
    }
    
    inline T& at(index_type idx){
        return underlying.at(idx);
    }
    
    const inline T& at(index_type idx) const{
        return underlying.at(idx);
    }
    
    inline const_iterator_type begin() const{
        return underlying.begin();
    }
    
    inline const_iterator_type end() const{
        return underlying.end();
    }
    
    inline iterator_type end(){
        return underlying.end();
    }
    
    inline iterator_type begin(){
        return underlying.begin();
    }
    
    inline size_type size() const{
        return underlying.size();
    }
    
    inline void reserve(){
        underlying.reserve();
    }
    
    inline void resize(){
        underlying.resize();
    }
	
};

/**
 The Unordered Cached Vector provides:
 - O(1) erase by value
 - O(1) erase by iterator
 All other complexities are identical to a regular vector. Elements must be moveable and hashable.
 Note that the order of elements cannot be guareneed.
 */
template<typename T>
class unordered_cached_vector : public std::vector<T>{
protected:
	phmap::flat_hash_map<size_t, typename std::vector<T>::size_type> offsets;
	
public:
    
    /**
     @return the hash for an element. This container does not need to include the element
     */
    inline constexpr size_t hash_for(const T& value) const{
        auto hasher = std::hash<T>();
        return hasher(value);
    }
	
    /**
     Erase by iterator
     @param it the iterator to remove
     */
    constexpr inline void erase(const typename std::vector<T>::iterator& it){
		// remove from the offset cache
		auto hasher = std::hash<T>();
		auto hash = hasher(*it);
		if (offsets.contains(hash)) {	// only erase if the container has the value
			offsets.erase(hash);

			// pop from back
			*it = std::move(this->back());

			// update offset cache
			hash = hasher(*it);
			offsets[hash] = std::distance(this->begin(), it);
			this->pop_back();
		}	
	}
    
    /**
     Erase by element hash
     @param size_t hash the hash of the element to remove
     */
    constexpr inline void erase_by_hash(size_t hash){
        auto it = this->begin() + offsets[hash];
        erase(it);
    }
	
    /**
     Erase by value
     @param value item to remove
     */
	inline void erase(const T& value){
		auto valuehash = std::hash<T>()(value);
		if (offsets.contains(valuehash)) {
			auto it = this->begin() + offsets[valuehash];
			erase(it);
		}
	}
	
	inline void insert(const T& value){
		auto hashcode = std::hash<T>()(value);
		offsets.emplace(hashcode,this->size());
		this->push_back(value);
	}
	
	inline void contains(const T& value){
		auto valuehash = std::hash<T>()(value);
		return offsets.contains(valuehash);
	}
};

template<typename T>
class unordered_deduplicating_vector : public unordered_cached_vector<T>{
public:
	
	inline void insert(const T& value){
		auto hashcode = std::hash<T>()(value);
		if (!this->offsets.contains(hashcode)){
			this->offsets.emplace(hashcode,this->size());
			this->push_back(value);
		}
	}
};

}
