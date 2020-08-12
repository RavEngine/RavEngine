//
//  RenderEngine.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug. All rights reserved.
//

#pragma once
#include "SharedObject.hpp"
#include "Entity.hpp"
#include <list>
#include "WeakRef.hpp"

namespace filament {
    class SwapChain;
    class Engine;
    class Renderer;
}
struct SDL_Window;

class RenderEngine : public SharedObject{
public:
    virtual ~RenderEngine(){};
    void Spawn(Ref<Entity> e);
    void Destroy(Ref<Entity> e);
    RenderEngine(const WeakRef<World>& w);
    void Draw();
	
	static const std::string currentBackend();
    WeakRef<World> world;

    static void Init();

protected:
    static SDL_Window* window;
    static filament::SwapChain* filamentSwapChain;
    static filament::Engine* filamentEngine;
    static filament::Renderer* filamentRenderer;
};
