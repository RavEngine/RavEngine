#pragma once
#include "PhysXDefines.h"
#include <task/PxCpuDispatcher.h>
#include "DataStructures.hpp"
#include "SpinLock.hpp"

namespace RavEngine{
class PhysicsTaskDispatcher : public physx::PxCpuDispatcher{
private:
    ConcurrentQueue<physx::PxBaseTask*> tasks;
    
    friend class World;
    friend class PhysicsSolver;
    
    
public:
    // invoked by PhysX to enqueue a new task
    void submitTask( physx::PxBaseTask& task ) final;
    uint32_t getWorkerCount() const final;
    
    virtual ~PhysicsTaskDispatcher(){}
    
};
}
