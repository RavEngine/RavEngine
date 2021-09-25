#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"
#include <boost/any.hpp>
#include <boost/container_hash/hash.hpp>
#include "DataStructures.hpp"

namespace RavEngine {
struct CacheKey{
    size_t hash = 0;
    RavEngine::Vector<boost::any> values;
    RavEngine::Vector<std::function<bool(const boost::any&)>> compareFuncs;
    
    inline bool operator==(const CacheKey& other) const{
        // ensure hashes match
        if (hash != other.hash){
            return false;
        }
        
        // type-erasure equality, for if there are hash collisions
        for(size_t i = 0; i < values.size(); i++){
            auto& func = compareFuncs[i];
            if (func(other.values[i]) == false){
                return false;
            }
        }
        return true;
    }
    
    template<typename T>
    inline void AddValue(const T& value){
        values.push_back(value);
        compareFuncs.push_back([value](const boost::any& v) -> bool{
            const auto& other = boost::any_cast<const T&>(v);
            return value == other;
        });
    }
};

/**
 Defines a generic non-owning cache.
 If the key type is not a parameter in the construction of your object, set the final template parameter to false
 */
template<typename key_t, typename T, bool keyIsConstructionParam = true>
struct GenericWeakCache{
    
private:
    template<typename V>
    static inline void addOne(const V& value, CacheKey& key){
        boost::hash_combine(key.hash,value);
        key.AddValue(value);
    }
    
protected:
    static UnorderedMap<CacheKey,WeakRef<T>> items;
    static SpinLock mtx;
public:
    /**
     Load object from cache. If the object is not cached in memory, it will be loaded from disk.
     @param str the name of the mesh
     @param extras additional arguments to pass to meshasset constructor
     @note All parameters must be hashable by boost::hash. In addition, the cache will retain copies of the data passed to differentiate constructed objects. For this reason, do not use large data structures, Ref/WeakRef, or unique_ptr as construction arguments,
     */
    template<typename ... A>
    static inline Ref<T> Get(const key_t& str, A ... extras){
        // calculate the hash
        CacheKey key;
        addOne(str,key);
        
        // TODO: optimize - calculate hash only unless there's a collision?
        (addOne(extras,key),...);
        
        //TODO: optimize
        mtx.lock();
        if (items.contains(key)){
            if (auto ptr = items.at(key).lock()){
                mtx.unlock();
                return ptr;
            }
        }
        Ref<T> m;
        if constexpr(keyIsConstructionParam){
            m = std::make_shared<T>(str,extras...);
        }
        else{
            m = std::make_shared<T>(extras...);
        }
        items.insert(std::make_pair(key,m));
        mtx.unlock();
        return m;
    }
    
    /**
     Reduce the size of the cache by removing expired pointers
     */
    static void Compact(){
        boost::container::vector<CacheKey> toremove;
        for(const auto& entry : items){
            if (entry.second.expired()){
                toremove.push_back(entry.first);
            }
        }
        for(const auto& c : toremove){
            items.erase(c);
        }
    }
    
    /**
     * Remove all items from the cache
     */
    static void Clear(){
        mtx.lock();
        items.clearn();
        mtx.unlock();
    }
};
}
template<typename key, typename T, bool keyIsConstructionParam>
RavEngine::SpinLock RavEngine::GenericWeakCache<key,T,keyIsConstructionParam>::mtx;

template<typename key,typename T, bool keyIsConstructionParam>
RavEngine::UnorderedMap<RavEngine::CacheKey,WeakRef<T>> RavEngine::GenericWeakCache<key,T,keyIsConstructionParam>::items;

namespace std{
    template<>
    struct hash<RavEngine::CacheKey>{
        inline bool operator()(const RavEngine::CacheKey& key) const{
            return key.hash;    // hash is precomputed
        }
    };
}
