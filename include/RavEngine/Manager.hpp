#pragma once
#include <phmap.h>
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"
#include <any>
#include <boost/container/vector.hpp>
#include <boost/container_hash/hash.hpp>

namespace RavEngine {
struct CacheKey{
    size_t hash = 0;
    boost::container::vector<std::any> values;
    boost::container::vector<std::function<bool(const std::any&)>> compareFuncs;
    
    inline bool operator==(const CacheKey& other) const{
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
        compareFuncs.push_back([value](const std::any& v) -> bool{
            const auto& other = std::any_cast<const T&>(v);
            
            return value == other;
        });
    }
};

template<typename key_t, typename T>
struct GenericWeakManager{
    
private:
    template<typename V>
    static inline void addOne(const V& value, CacheKey& key){
        boost::hash_combine(key.hash,value);
        key.AddValue(value);
    }
    
protected:
    static phmap::flat_hash_map<CacheKey,WeakRef<T>> items;
    static SpinLock mtx;
public:
    /**
     Load object from cache. If the object is not cached in memory, it will be loaded from disk.
     @param str the name of the mesh
     @param extras additional arguments to pass to meshasset constructor
     @note All parameters must be hashable by boost::hash. 
     */
    template<typename ... A>
    static inline Ref<T> Get(const key_t& str, A ... extras){
        // calculate the hash
        CacheKey key;
        boost::hash_combine(key.hash,str);
        
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
        Ref<T> m = std::make_shared<T>(str,extras...);
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
};
}
template<typename key, typename T>
RavEngine::SpinLock RavEngine::GenericWeakManager<key,T>::mtx;

template<typename key,typename T>
phmap::flat_hash_map<RavEngine::CacheKey,WeakRef<T>> RavEngine::GenericWeakManager<key,T>::items;

namespace std{
    template<>
    struct hash<RavEngine::CacheKey>{
        inline bool operator()(const RavEngine::CacheKey& key) const{
            return key.hash;    // hash is precomputed
        }
    };
}
