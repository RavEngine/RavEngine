#include "App.hpp"
#include "Debug.hpp"
#include <chrono>
#include <date/date.h>
#if !RVE_SERVER
#include <SDL_log.h>
#endif

using namespace RavEngine;
SpinLock Debug::mtx;

void RavEngine::Debug::LogHelper(FILE* output, const std::string_view message, const char* type) {
	auto date = date::format("%F %T", std::chrono::system_clock::now());
	mtx.lock();
#if !RVE_SERVER
	SDL_Log("[%s] %s - %s\n", date.c_str(), type, message.data());
#else
    fprintf(output, "[%s] %s - %s\n", date.c_str(), type, message.data());
#endif
	mtx.unlock();
}

void Debug::InvokeUserHandler(const std::string_view msg){
    GetApp()->OnFatal(msg);
}
