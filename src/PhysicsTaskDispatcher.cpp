#include "PhysicsTaskDispatcher.hpp"
#include "App.hpp"
#include "GetApp.hpp"
#include "PhysicsSolver.hpp"

using namespace RavEngine;

void PhysicsTaskDispatcher::submitTask(physx::PxBaseTask &task){
    // immediately place it on the task queue as background async
    physx::PxBaseTask* taskptr = &task;
    auto& executor = GetApp()->executor;
    executor.silent_async([taskptr]{
        taskptr->run();
        taskptr->release();
    });
}

uint32_t PhysicsTaskDispatcher::getWorkerCount() const {
    return GetApp()->executor.num_workers();
}
