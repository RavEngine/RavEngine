#include "App.hpp"
#include "Debug.hpp"
#include <chrono>
#include <date/date.h>
#include <SDL_log.h>

using namespace RavEngine;
SpinLock Debug::mtx;

void RavEngine::Debug::LogHelper(FILE* output, const std::string_view message, const char* type) {
	auto date = date::format("%F %T", std::chrono::system_clock::now());
	mtx.lock();
	SDL_Log("[%s] %s - %s\n", date.c_str(), type, message.data());
	mtx.unlock();
}

void Debug::InvokeUserHandler(const std::string_view msg){
    GetApp()->OnFatal(msg);
}
