#pragma once
#include <typeindex>

namespace RavEngine{
class Type{
protected:
    std::type_index type;
    
public:
    template<typename T>
    Type() : type(typeid(T)){}
    
    template<typename T>
    Type(T* obj) : type(typeid(obj)){}
    
    Type(const std::type_index& t) : type(t){}
    
    bool operator=(const Type& other) const{
        return other.type == type;
    }
    
    std::size_t Hash() const{
       return type.hash_code();
    }
};
}

namespace std{
struct hash {
    std::size_t operator()(const RavEngine::Type& k) const
    {
        return k.Hash();
    }
};
}
