#pragma once

namespace RavEngine {

template<typename T, typename PolymorphicIndirection_t>
    struct PolymorphicGetResult{
        PolymorphicIndirection_t& items;
        
        inline T& operator[](uint32_t idx) const{
            return *items.elts[idx].template Get<T>(items.owner);
        }
        
        inline T& at(uint32_t idx) const{
            return *items.elts.at(idx).template Get<T>(items.owner);
        }
        
        inline auto size() const{
            return items.elts.size();
        }
    };
}
