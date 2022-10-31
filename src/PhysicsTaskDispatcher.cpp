#include "PhysicsTaskDispatcher.hpp"

using namespace RavEngine;

void PhysicsTaskDispatcher::submitTask(physx::PxBaseTask &task){
    tasks.enqueue(&task);
}

uint32_t PhysicsTaskDispatcher::getWorkerCount() const {
    //TODO: return the number of threads in the task graph
    return 1;
}
