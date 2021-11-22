#pragma once

namespace RavEngine{
    template<typename T>
    struct RemoveAction{
        inline static constexpr bool HasCustomAction(){
            return false;
        }
        inline void DoAction(T* ptr){}
    };
}
