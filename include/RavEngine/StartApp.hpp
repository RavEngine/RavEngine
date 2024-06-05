#pragma once

#if defined(WINAPI_FAMILY) && (WINAPI_FAMILY == WINAPI_FAMILY_APP)
//On UWP, we need to not have SDL_main otherwise we'll get a linker error
#define SDL_MAIN_HANDLED
#define SDL_WINRT_METADATA_FILE_AVAILABLE	// we are using C++20 so we can't have /ZW

#endif
#include <SDL3/SDL_main.h>

#ifdef _WINRT
// UWP startup requires extra effort
#undef main
#define START_APP(APP) \
int DoProgram(int argc, char** argv){\
auto a = std::make_unique<APP>(); return a->run(argc, argv);\
}\
int main(int argc, char** argv) { \
	return SDL_RunApp(0, NULL, DoProgram, NULL);\
}
#else
#define START_APP(APP) extern "C" int main(int argc, char** argv){auto a = std::make_unique<APP>(); return a->run(argc, argv);}
#endif
