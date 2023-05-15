#include "App.hpp"
#include "Debug.hpp"
#include <chrono>
#include <date/date.h>

using namespace RavEngine;
SpinLock Debug::mtx;

void RavEngine::Debug::LogHelper(FILE* output, const char* message, const char* type) {
	auto date = date::format("%F %T", std::chrono::system_clock::now());
	mtx.lock();
	fprintf(output, "[%s] %s - %s\n", date.c_str(), type, message);
	mtx.unlock();
}

void Debug::InvokeUserHandler(const char *msg){
    GetApp()->OnFatal(msg);
}
