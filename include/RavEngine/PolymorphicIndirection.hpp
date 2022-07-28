#pragma once

namespace RavEngine {

template<typename T, typename PolymorphicIndirection_t>
    struct PolymorphicGetResult{
        using type = T;
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
        
        inline auto begin(){
            return items.elts.begin();
        }
        
        inline auto end(){
            return items.elts.end();
        }
        
        inline auto begin() const{
            return items.elts.begin();
        }
        
        inline auto end() const{
            return items.elts.end();
        }
        
        template<typename BaseIncludingArgument>
        inline auto HandleFor(int idx){
            return items.template HandleFor<BaseIncludingArgument>(idx);
        }
    };
}
