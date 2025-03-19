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
		struct AudioBitsetUnpack {
			uint8_t current = 0, inactive = 0, render = 0;
		};
		static AudioBitsetUnpack UnpackAudioBitset(uint8_t audioCurrentBitset) {
			uint8_t currentIdx = (audioCurrentBitset & audioCurrentMask) >> audioCurrentShift;
			uint8_t inactiveIdx = (audioCurrentBitset & audioInactiveMask) >> audioInactiveShift;
			uint8_t renderIdx = (audioCurrentBitset & audioRenderMask) >> audioRenderShift;

			return { currentIdx, inactiveIdx, renderIdx };
		}

		static uint8_t RepackAudioBitset(AudioBitsetUnpack values) {
			uint8_t audioNextBitset = 0;
			audioNextBitset |= values.current << audioCurrentShift;
			audioNextBitset |= values.inactive << audioInactiveShift;
			audioNextBitset |= values.render << audioRenderShift;

			return audioNextBitset;
		}

        inline void SwapCurrrentAudioSnapshot(){
			// swapping currentIdx with inactiveIdx
			uint8_t audioCurrentBitset = audioswapbitset.load();
            uint8_t audioNextBitset = audioCurrentBitset;
            
            auto makeNextBitset = [&audioCurrentBitset]{
                auto indices = UnpackAudioBitset(audioCurrentBitset);

                std::swap(indices.current, indices.inactive);

                uint8_t audioNextBitset = RepackAudioBitset(indices);

                // set the audioAvailable flag
                audioNextBitset |= 1 << audioAvailableShift;
                
                return audioNextBitset;
            };
            audioNextBitset = makeNextBitset();
	
            while (!audioswapbitset.compare_exchange_weak(audioCurrentBitset, audioNextBitset, std::memory_order_release, std::memory_order_relaxed)){
                audioNextBitset = makeNextBitset();
            }
        }
        inline void SwapRenderAudioSnapshotIfNeeded(){
			// swapping inactive with render
			auto audioCurrentBitset = audioswapbitset.load();
            if (NewAudioAvailable(audioCurrentBitset)) {
                uint8_t audioNextBitset = audioCurrentBitset;
                auto makeNextBitset = [&audioCurrentBitset] {
                    auto indices = UnpackAudioBitset(audioCurrentBitset);
                    std::swap(indices.inactive, indices.render);
                    // newAudioAvailable is set to false automatically because repackAudioBitset sets all bits to 0
                    uint8_t audioNextBitset = RepackAudioBitset(indices);
                    return audioNextBitset;
                };
                audioNextBitset = makeNextBitset();
                
                while (!audioswapbitset.compare_exchange_weak(audioCurrentBitset, audioNextBitset, std::memory_order_release, std::memory_order_relaxed)){
                    audioNextBitset = makeNextBitset();
                }
			}
        }
        inline AudioSnapshot* GetCurrentAudioSnapshot(){
			auto bitset = audioswapbitset.load();
			auto idx = (bitset & audioCurrentMask) >> audioCurrentShift;
			auto ptr = &audioSnapshots[idx];
			return ptr;
        }
        inline AudioSnapshot* GetRenderAudioSnapshot(){
			auto bitset = audioswapbitset.load();
			auto idx = (bitset & audioRenderMask) >> audioRenderShift;
			auto ptr = &audioSnapshots[idx];
			return ptr;
        }
		inline static bool NewAudioAvailable(uint8_t currentBitset) {
			return (currentBitset & audioAvailableMask); // 0 = false, any other = true
		}
#endif

	private:
        
        void Tick();
        
        float currentScale = 0.01f;
        
		Ref<World> renderWorld;
	
		ConcurrentQueue<Function<void(void)>> main_tasks;
		
		locked_hashset<Ref<World>,SpinLock> loadedWorlds;
#if !RVE_SERVER
        Array<AudioSnapshot, 3> audioSnapshots;
		constexpr static uint8_t
			audioCurrentShift = 0, audioInactiveShift = 2, audioRenderShift = 4, audioAvailableShift = 6,
			audioCurrentMask = 0b00000011,
			audioInactiveMask = (audioCurrentMask << audioInactiveShift),
			audioRenderMask = (audioCurrentMask << audioRenderShift),
			audioAvailableMask = 1 << audioAvailableShift
			;

		std::atomic<uint8_t> audioswapbitset = (0 << audioCurrentShift) | (1 << audioInactiveShift) | (2 << audioRenderShift);
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


