#pragma once
#include <RGL/Types.hpp>
#include <RGL/Synchronization.hpp>

namespace RGL{

struct FenceMTL : public IFence{
    void Wait() final;
    void Reset() final;
    void Signal() final;
    virtual ~FenceMTL(){}
};

}
