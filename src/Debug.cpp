#include "App.hpp"
#include "Debug.hpp"

using namespace RavEngine;
SpinLock Debug::mtx;

void Debug::InvokeUserHandler(const char *msg){
    GetApp()->OnFatal(msg);
}
