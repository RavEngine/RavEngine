#pragma once
#include <phmap.h>
#include <CTTI.hpp>

namespace RavEngine {
class NetworkReplicable{
    typedef phmap::flat_hash_map<std::string,std::string> map_t;
    
    struct encoding{
        map_t values;   //serialized component state
        ctti_t id;      //your component's CTTI ID, must be set!
    };
public:
    /**
    Encode the values your component needs to send over the network into this structure, using key-value pairs
     */
    virtual encoding Serialize() = 0;
    
    /**
     When passed a map of key-value pairs, use to restore the state of the component
     @param values the encoded values sent over the network in Serialize
     */
    virtual void Deserialize(const map_t& values) = 0;
};
}
