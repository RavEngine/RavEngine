#pragma once

#if !RVE_SERVER
#include <SDL3/SDL_main.h>
#endif

#define START_APP(APP) extern "C" int main(int argc, char** argv){auto a = std::make_unique<APP>(); return a->run(argc, argv);}
