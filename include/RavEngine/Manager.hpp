#pragma once
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"
#include "Map.hpp"
#include "Vector.hpp"
#include <tuple>

namespace RavEngine {

    struct CacheBase {
        using unique_key_t = uint32_t;
    };

/**
 Defines a generic non-owning cache.
 If the key type is not a parameter in the construction of your object, set the final template parameter to false
 */
template<typename key_t, typename T, bool keyIsConstructionParam = true>
struct GenericWeakReadThroughCache : public CacheBase{

protected:
    static UnorderedMap<std::tuple<key_t, unique_key_t>,WeakRef<T>> items;
    static SpinLock mtx;
public:
    /**
     Load object from cache. If the object is not cached in memory, it will be loaded from disk.
     @param str the name of the mesh
     @param extras additional arguments to pass to meshasset constructor
     @note The cache does not pay attention to the values of `extras`. If you need to load a new asset, use GetWithKey where you can pass a unique key identifer. The unique key defaults to zero if accessed via this method.
     */
    template<typename ... A>
    static inline Ref<T> Get(const key_t& str, A ... extras){
        return GetWithKey(str, 0, extras...);
    }

    /**
     Load object from cache. If the object is not cached in memory, it will be loaded from disk.
     @param str the name of the mesh
     @param unique_key a differentiator to force a new load and identify it later
     @param extras additional arguments to pass to meshasset constructor
     @note The cache does not pay attention to the values of `extras`. If you need to load a new asset, use a different unique key identifier.
     */
    template<typename ... A>
    static inline Ref<T> GetWithKey(const key_t& str, unique_key_t unique_key, A ... extras) {
        auto key = std::tuple<key_t, unique_key_t>(str, unique_key);
       
        //TODO: optimize
        std::lock_guard guard(mtx);
        if (items.contains(key)) {
            auto value = items.at(key).lock();
            if (auto ptr = value) {
                return ptr;
            }
        }
        Ref<T> m;
        if constexpr (keyIsConstructionParam) {
            m = std::make_shared<T>(str, extras...);
        }
        else {
            m = std::make_shared<T>(extras...);
        }
        items[key] = m;
        return m;
    }

    /**
     Reduce the size of the cache by removing expired pointers
     */
    static void Compact(){
        RavEngine::Vector<std::tuple<std::string, unique_key_t>> toremove;
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
        std::lock_guard guard(mtx);
        items.clear();
    }
};
}
template<typename key, typename T, bool keyIsConstructionParam>
RavEngine::SpinLock RavEngine::GenericWeakReadThroughCache<key,T,keyIsConstructionParam>::mtx;

template<typename key,typename T, bool keyIsConstructionParam>
RavEngine::UnorderedMap<std::tuple<key, RavEngine::CacheBase::unique_key_t>, WeakRef<T>> RavEngine::GenericWeakReadThroughCache<key, T, keyIsConstructionParam>::items;

