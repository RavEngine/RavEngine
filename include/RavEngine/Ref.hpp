#pragma once
#include <memory>

template<typename T>
using Ref = std::shared_ptr<T>;

namespace RavEngine{
    template<typename T, typename ... A>
    static inline Ref<T> New(A&& ... args){
        return std::make_shared<T>(args...);
    }
}
