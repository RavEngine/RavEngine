#pragma once
#include <chrono>
#include <concurrentqueue.h>
#include "SpinLock.hpp"
#include <taskflow/taskflow.hpp>
#include "NetworkManager.hpp"
#if !RVE_SERVER
#include <RGL/Types.hpp>
#include "RenderTargetCollection.hpp"
#include "AudioSnapshot.hpp"
#endif
#include <optional>
#include "GetApp.hpp"

#define SINGLE_THREADED 0

namespace RavEngine {

struct RenderEngine;
struct VirtualFilesystem;
struct FrameData;
struct Window;
struct AudioPlayer;

	struct AppConfig {
		enum class RenderBackend {
			Metal,
			DirectX12,
			Vulkan,
            WebGPU,
			AutoSelect
		} preferredBackend = RenderBackend::AutoSelect;
	};

	typedef std::chrono::high_resolution_clock clocktype;
	typedef std::chrono::duration<double, std::micro> timeDiff;
	typedef std::chrono::seconds deltaSeconds;
	typedef std::chrono::time_point<clocktype> timePoint;
	
	class InputManager;
	class World;

	class App {
		friend class NetworkManager;
#if RVE_SERVER
        constexpr static std::chrono::duration<double> min_tick_time{1.0/60};
#endif
        
#if !RVE_SERVER
        std::unique_ptr<RenderEngine> Renderer;
#endif
        std::unique_ptr<VirtualFilesystem> Resources;
	public:
        App();
		virtual ~App();

		// set this to true in app constructor if XR is desired
		bool wantsXR = false;
        
        /**
         Override this method to provide a custom fatal handler
         */
        virtual void OnFatal(const std::string_view msg){}
        
        /**
         Override to be notified if too much audio work was submitted. The default implementation logs a warning.
         */
        virtual void OnDropAudioWorklets(uint32_t ndropped);

		// Override to disable audio. If false, audio backend will not be initialized and audio player threads will not be created.
		virtual bool NeedsAudio() const {
			return true;
		}

		bool GetAudioActive() const;

		// Override to disable networking. If true, networking backend and associated threads will be created. 
		virtual bool NeedsNetworking() const {
			return false;
		}
		
		/**
		 Signal to gracefully shut down the application
		 */
		void Quit();

		/**
		Invoked automatically. Passes command line arguments.
		*/
		int run(int argc, char** argv);

		constexpr static float evalNormal = 60;	//normal speed is 60 hz
		
		/**
		 @return the current time, measured in seconds since the application launched
		 */
        inline double GetCurrentTime(){
			return time;
		};
		
		//number of cores on device
		const int numcpus = std::thread::hardware_concurrency();
		
		//global thread pool, threads = logical processors on CPU
        tf::Executor executor{
#if SINGLE_THREADED
        1 // use main thread only on emscripten
#else
            std::max<size_t>(std::thread::hardware_concurrency() - 2, 2)    // for audio - TODO: make configurable
#endif
        };
		
		//networking interface
		NetworkManager networkManager;
        
        inline VirtualFilesystem& GetResources(){
            return *Resources.get();
        }
#if !RVE_SERVER
        inline RenderEngine& GetRenderEngine(){
            return *Renderer.get();
        }
#endif
#if !RVE_SERVER
        inline auto& GetAudioPlayer(){
            return player;
        }
#endif
        
        inline bool HasRenderEngine(){
#if !RVE_SERVER
            return static_cast<bool>(Renderer);
#else
            return false;
#endif
        }
		
		/**
		 Dispatch a task to be executed on the main thread.
		 @param f the block to execute
		 @note To pass parameters, do not reference! Instead, you must explicitly copy the values you want to pass:
		 @code
 int x = 5; int y = 6;
 RavEngine::GetApp()->DispatchMainThread([=]{
	std::cout << x << y << std::endl;
 });
		 @endcode
		 */
        template<typename T>
		constexpr inline void DispatchMainThread(const T& f){
			main_tasks.enqueue(f);
		}

		/**
		@return the current application tick rate
		*/
        float CurrentTPS();
        float GetCurrentFPSScale() const{
            return currentScale;
        }
#if !RVE_SERVER
		Ref<InputManager> inputManager;
#endif

		/**
		Set the current world to tick automatically
		@param newWorld the new world
		*/
		void SetRenderedWorld(Ref<World> newWorld);
		
		/**
		 Add a world to be ticked
		 @param world the world to tick
		 */
		void AddWorld(Ref<World> world);

		/**
		Remove a world from the tick list
		@param world the world to tick
		*/
		void RemoveWorld(Ref<World> world);

		/**
		* Unload all worlds
		*/
        void RemoveAllWorlds();
		
		/**
		 Replace a loaded world with a different world, transferring render state if necessary
		 @param oldWorld the world to replace
		 @param newWorld the world to replace with. Cannot be already loaded.
		 */
		void AddReplaceWorld(Ref<World> oldWorld, Ref<World> newWorld);
		
#if !RVE_SERVER

		/**
		 Set the window titlebar text
		 @param title the text for the titlebar
		 @note Do not call this every frame. To update periodically with data such as frame rates, use a scheduled system.
		 */
        void SetWindowTitle(const char* title);
#endif
		
		std::optional<Ref<World>> GetWorldByName(const std::string& name);

		auto GetCurrentRenderWorld()  {
			return renderWorld;
		}
#if !RVE_SERVER
        inline void SwapCurrrentAudioSnapshot(){
            audiomtx1.lock();
            std::swap(acurrent,ainactive);
			newAudioAvailable = true;
            audiomtx1.unlock();
        }
        inline void SwapRenderAudioSnapshotIfNeeded(){
			audiomtx1.lock();
			if (newAudioAvailable) {
				std::swap(ainactive, arender);
				newAudioAvailable = false;
			}
			audiomtx1.unlock();
        }
        inline AudioSnapshot* GetCurrentAudioSnapshot(){
            return acurrent;
        }
        inline AudioSnapshot* GetRenderAudioSnapshot(){
            return arender;
        }
#endif

	private:
        
        void Tick();
        
        float currentScale = 0.01f;
        
		Ref<World> renderWorld;
	
		ConcurrentQueue<Function<void(void)>> main_tasks;
		
		locked_hashset<Ref<World>,SpinLock> loadedWorlds;
#if !RVE_SERVER
        AudioSnapshot a1, a2, a3, *acurrent = &a1, *ainactive = &a2, *arender = &a3;
        SpinLock audiomtx1;
		bool newAudioAvailable = false;
#endif
	protected:
#if !RVE_SERVER
		RGLDevicePtr device;
		std::unique_ptr<Window> window;
		RenderViewCollection mainWindowView;

		std::vector<RenderViewCollection> xrRenderViewCollections;
#endif

		virtual AppConfig OnConfigure(int argc, char** argv) { return AppConfig{}; }
#if !RVE_SERVER
		//plays the audio generated in worlds
		std::unique_ptr<AudioPlayer> player;
#endif
		
		/**
		The startup hook.
		@param argc the number of command line arguments
		@param argv the command line arguments
		*/
		virtual void OnStartup(int argc, char** argv) {}
		/**
		Invoked before destructor when the application is expected to shut down. You can return exit codes from here.
		*/
		virtual int OnShutdown() { return 0; };

		//last frame time, frame delta time, framerate scale, maximum frame time
		timePoint lastFrameTime;
		timeDiff deltaTimeMicroseconds{0};
		const timeDiff maxTimeStep = std::chrono::milliseconds((long)1000);
		
		double time = 0;
	public:
#if !RVE_SERVER
		auto GetDevice() {
			return device;
		}
        const auto& GetMainWindow() const{
            return window;
        }
#endif
	};
}


