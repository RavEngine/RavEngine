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
    class View;
    class Scene;
}
struct SDL_Window;
namespace RavEngine {
    class RenderEngine : public SharedObject {
    public:
        virtual ~RenderEngine() {};
        void Spawn(Ref<Entity> e);
        void Destroy(Ref<Entity> e);
        RenderEngine(const WeakRef<World>& w);
        void Draw();

        static const std::string currentBackend();
        WeakRef<World> world;

        static SDL_Window* const GetWindow() {
            return window;
        }

    protected:
        filament::View* filamentView = nullptr;
        filament::Scene* filamentScene = nullptr;

        static void Init();
        static void* getNativeWindow(SDL_Window*);
#ifdef __APPLE__
        static void* setUpMetalLayer(void*);
        static void* resizeMetalLayer(void* nativeView);
#endif

        static SDL_Window* window;
        static filament::SwapChain* filamentSwapChain;
        static filament::Engine* filamentEngine;
        static filament::Renderer* filamentRenderer;
    };
}