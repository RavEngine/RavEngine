#pragma once
#include "Entity.hpp"

namespace RavEngine {
    class CheckedComponentHandleBase {
    protected:
        Entity owner;
    public:
        CheckedComponentHandleBase(decltype(owner) owner) : owner(owner) {}

        inline operator bool() const {
            return IsValid();
        }

        inline void reset() {
            owner = Entity({ INVALID_ENTITY }, nullptr);
        }

        inline bool IsValid() const {
            return owner.IsValid();
        }
    };

    template<typename T>
    struct CheckedComponentHandle : public CheckedComponentHandleBase {
        CheckedComponentHandle(decltype(owner) owner) : CheckedComponentHandleBase(owner) {}

        template<typename ValidatorType>
        T* get(const ValidatorType& v) {
            static_assert(v.template IsValid<T>(), "Validator does not have the necessary types");
            return &owner.GetComponent<T>();
        }

        bool operator==(const CheckedComponentHandle<T>& other) const {
            return owner.id == other.owner.id;
        }
    };

}