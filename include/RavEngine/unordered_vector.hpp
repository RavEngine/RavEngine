#pragma once
#include <vector>
#include <algorithm>

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
        *it = std::move(underlying.back());
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

}
