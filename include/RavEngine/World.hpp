#pragma once
//
//  World.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "SharedObject.hpp"
#include "PhysicsSolver.hpp"
#include "RenderEngine.hpp"
#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include <plf_list.h>
#include "AudioSource.hpp"
#include "FrameData.hpp"
#include <taskflow/taskflow.hpp>
#include "SystemManager.hpp"
#include "Skybox.hpp"

namespace RavEngine {
	class Entity;
	class InputManager;

	class World : public ComponentStore<phmap::NullMutex>, public virtual_enable_shared_from_this<World>{
		friend class AudioPlayer;
		friend class App;
	public:
		constexpr static uint8_t id_size = 8;

		Skybox skybox;

	private:
		std::atomic<bool> isRendering = false;
		char worldIDbuf [id_size];
		struct SyncOp{
			Ref<Component> c;
			ctti_t id;
			bool add;
		};
		ConcurrentQueue<SyncOp> toSync;
		
		tf::Taskflow masterTasks;
		
		ComponentStore<phmap::NullMutex>::entry_type::const_iterator geobegin,geoend, skinnedgeobegin, skinnedgeoend;
		iter_map iterator_map;
		struct systaskpair{
			tf::Task task;
			const SystemEntry<World>* system;
			bool isTimed = false;
		};
		phmap::flat_hash_map<ctti_t, systaskpair> graphs;
		
		void CreateFrameData();
		
		void RebuildTaskGraph();
		
		std::chrono::time_point<clock_t> time_now = clock_t::now();
		float currentFPSScale = 0.01;
		
		//Entity list
		typedef locked_hashset<Ref<Entity>, SpinLock> EntityStore;
	protected:
		void CTTI_Add(Ref<Component> c, ctti_t id) override{
			toSync.enqueue({c,id,true});
		}
		
		void CTTI_Remove(Ref<Component> c, ctti_t id) override{
			toSync.enqueue({c,id,false});
		}
		
		EntityStore Entities, to_destroy;

		//physics system
		PhysicsSolver Solver;
		
		//fire-and-forget audio
		plf::list<InstantaneousAudioSource> instantaneousToPlay;

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void pretick(float fpsScale) {}
		void TickECS(float);
		
		void FillFramedata();
		
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void posttick(float fpsScale) {}
		
		void OnAddComponent(Ref<Component>) override;
		void OnRemoveComponent(Ref<Component>) override;
        
        /**
         * Tick a System on all available threads. Blocks until all have finished.
         * @param system the System to tick
         * @param scale the frame rate scale to pass on to the System's tick method
         */
        void TickSystem(Ref<System> system, float scale);
		
		bool physicsActive = false;
		
		void Destroy_pending();

    public:
		std::string_view worldID{ worldIDbuf,id_size };
		std::atomic<bool> newFrame = false;

		inline float getCurrentFPSScale() const {
			return currentFPSScale;
		}
		
		SystemManager<World> systemManager;
		
		/**
		* Initializes the physics-related Systems.
		* @return true if the systems were loaded, false if they were not loaded because they are already loaded
		*/
		bool InitPhysics();
		
		/**
		* Evaluate the world given a scale factor. One tick = 1/App::EvalNormal
		* @param the tick fraction to evaluate
		* @note the GameplayStatics CurrentWorld is ticked automatically in the App
		*/
		void Tick(float);

		World();
		
		/**
		 Constructor useful for setting the world name
		 @param name the string name for this world. Note that if the name has more characters than id_size, only the first id_size characters will be included.
		 */
		World(const std::string& name) : World(){
			auto len = name.size();
			std::memcpy((char*)worldID.data(), name.data(), std::min(len,static_cast<decltype(len)>(id_size)));
		}

		/**
		* Constructor that takes a custom skybox. This constructor will bypass loading the default skybox.
		* @param sk the skybox to use
		*/
		World(const Skybox& sk) : World() {
			skybox = sk;
		}

		//spawn function (takes an Entity)
		bool Spawn(Ref<Entity>);

		bool Destroy(Ref<Entity>);

		void PlaySound(const InstantaneousAudioSource& ias){
			instantaneousToPlay.push_back(ias);
		}
		
		/**
		 Called by GameplayStatics when the final world is being deallocated
		 */
		inline void DeallocatePhysics() {
			Solver.DeallocatePhysx();
		}

		virtual ~World() {
			std::cout << "world destructor @ " << this << std::endl;
		}

		/**
		* Called when this world is made the active world for the App
		*/
		virtual void OnActivate() {}

		/**
		* Called when this world was the active world for the App but has been replaced by a different world
		*/
		virtual void OnDeactivate() {}
	};
}
