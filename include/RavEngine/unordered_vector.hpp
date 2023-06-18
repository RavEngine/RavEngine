#pragma once
#include <vector>
#include <phmap.h>

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
    using value_type = T;

    /**
     Erase by iterator. Complexity is O(1).
     @param it the iterator to erase
     */
    inline const_iterator_type erase(iterator_type it){
        new (&(*it)) T(std::move(underlying.back()));
        underlying.pop_back();
        return it;
	}
    
    /**
     @return the underlying vector. Do not modify!
     */
    inline auto& get_underlying() {
        return underlying;
    }
	
    /**
     Erase by value. Complexity is O(n)
     @param value the data to erase
     */
	inline const_iterator_type erase(const T& value){
        auto it = std::find(underlying.begin(),underlying.end(),value);
        underlying.erase(it);
        return it;
	}
	
    /**
     Add an item to the container
     @param value the data to add
     @return a reference to the pushed item
     @note references may become invalid if an item is erased from the container
     */
	inline T& insert(const T& value){
        underlying.push_back(value);
        return underlying.back();
	}
    
    /**
     Add an item to the container
     @param value the data to add
     @return a reference to the emplaced item
     @note references may become invalid if an item is erased from the container
     */
    template<typename ... A>
    inline T& emplace(A&& ... args){
        underlying.emplace_back(args...);
        return underlying.back();
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
    
    inline void reserve(size_t num){
        underlying.reserve(num);
    }
    
    inline void resize(size_t num){
        underlying.resize(num);
    }
    
    inline bool empty() const{
        return underlying.empty();
    }
    
    inline void clear(){
        underlying.clear();
    }
    
    inline auto data() const{
        return underlying.data();
    }
};

/**
 The Unordered Contiguous Set provides:
 - O(1) erase by value
 - O(1) erase by iterator
 All other complexities are identical to a regular vector. Elements must be moveable and hashable. Hashes must not have collisions. An imperfect hash will result in unpredictable behavior.
 Note that the order of elements cannot be guareneed.
 */
template<typename T,typename vec = std::vector<T>, typename _hash = std::hash<T>>
class unordered_contiguous_set : public unordered_vector<T,vec>{
protected:
	phmap::flat_hash_map<size_t, typename unordered_vector<T,vec>::size_type> offsets;
	
public:
    typedef typename unordered_vector<T,vec>::iterator_type iterator_type;
    typedef typename unordered_vector<T,vec>::iterator_type iterator;
    typedef typename unordered_vector<T,vec>::const_iterator_type const_iterator_type;
    typedef typename unordered_vector<T,vec>::const_iterator_type const_iterator;       // for redundancy
    typedef typename unordered_vector<T,vec>::size_type index_type;
    typedef typename unordered_vector<T,vec>::size_type size_type;
    
    /**
     @return the hash for an element. This container does not need to include the element
     */
    inline constexpr size_t hash_for(const T& value) const{
        auto hasher = _hash();
        return hasher(value);
    }
	
    /**
     Erase by iterator
     @param it the iterator to remove
     */
    inline void erase(const typename unordered_vector<T,vec>::iterator_type& it){
		// remove from the offset cache
		auto hasher = _hash();
		auto hash = hasher(*it);
		if (offsets.contains(hash)) {	// only erase if the container has the value
			offsets.erase(hash);

			// pop from back
            auto i = unordered_vector<T,vec>::erase(it);

			// update offset cache
			hash = hasher(*i);
			offsets[hash] = std::distance(this->begin(), it);
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
		auto valuehash = _hash()(value);
		if (offsets.contains(valuehash)) {
			auto it = this->begin() + offsets[valuehash];
			erase(it);
		}
	}
	
	inline void insert(const T& value){
		auto hashcode = _hash()(value);
        if (!this->offsets.contains(hashcode)){
            offsets.emplace(hashcode,this->size());
            unordered_vector<T,vec>::insert(value);
        }
	}
	
	inline void contains(const T& value){
		auto valuehash = _hash()(value);
		return offsets.contains(valuehash);
	}
};

}
