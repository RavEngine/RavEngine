#pragma once
#include <vector>
#include <phmap.h>

namespace RavEngine {

template<typename T>
struct unordered_vector : public std::vector<T>{

	inline void erase(const typename std::vector<T>::iterator& it){
		*it = std::move(this->back());
		this->pop_back();
	}
	
	inline void erase(const T& value){
		auto it = std::find(this->begin(),this->end(),value);
		erase(it);
	}
	
	inline void insert(const T& value){
		this->push_back(value);
	}
	
};

template<typename T>
class unordered_cached_vector : public std::vector<T>{
protected:
	phmap::flat_hash_map<size_t, typename std::vector<T>::size_type> offsets;
	
public:
    
    /**
     @return the hash for an element. This container does not need to include the element
     */
    inline size_t hash_for(const T& value) const{
        auto hasher = std::hash<T>();
        return hasher(value);
    }
	
    /**
     Erase by iterator
     @param it the iterator to remove
     */
	inline void erase(const typename std::vector<T>::iterator& it){
		// remove from the offset cache
		auto hasher = std::hash<T>();
		auto hash = hasher(*it);
		offsets.erase(hash);
		
		// pop from back
		*it = std::move(this->back());
		this->pop_back();

		// update offset cache
		hash = hasher(*it);
		offsets[hash] = std::distance(this->begin(),it);
	}
    
    /**
     Erase by element hash
     @param size_t hash the hash of the element to remove
     */
    inline void erase_by_hash(size_t hash){
        auto it = this->begin() + offsets[hash];
        erase(it);
    }
	
    /**
     Erase by value
     @param value item to remove
     */
	inline void erase(const T& value){
		auto valuehash = std::hash<T>()(value);
		
		auto it = this->begin() + offsets[valuehash];
		erase(it);
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
