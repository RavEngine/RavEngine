#pragma once
#include <phmap.h>
#include "Ref.hpp"
#include "WeakRef.hpp"
#include "SpinLock.hpp"

namespace RavEngine {
template<typename key, typename T>
struct GenericWeakManager{
protected:
    static phmap::flat_hash_map<key,WeakRef<T>> items;
    static SpinLock mtx;
public:
    /**
     Load object from cache. If the object is not cached in memory, it will be loaded from disk.
     @param str the name of the mesh
     @param extras additional arguments to pass to meshasset constructor
     @note Using this with the specific model loading constructor is not supported and will produce unexpected results.
     */
    template<typename ... A>
    static inline Ref<T> Get(const key& str, A ... extras){
        //TODO: optimize
        mtx.lock();
        if (items.contains(str)){
            if (auto ptr = items.at(str).lock()){
                mtx.unlock();
                return ptr;
            }
        }
        Ref<T> m = std::make_shared<T>(str,extras...);
        items.insert(std::make_pair(str,m));
        mtx.unlock();
        return m;
    }
};
}
template<typename key, typename T>
RavEngine::SpinLock RavEngine::GenericWeakManager<key,T>::mtx;

template<typename key,typename T>
phmap::flat_hash_map<key,WeakRef<T>> RavEngine::GenericWeakManager<key,T>::items;
