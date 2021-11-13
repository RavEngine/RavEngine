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
#include "AudioSource.hpp"
#include "FrameData.hpp"
#include <taskflow/taskflow.hpp>
#include "Skybox.hpp"
#include "Types.hpp"
#include "Reflection.hpp"

namespace RavEngine {
	class Entity;
	class InputManager;
    struct Entity;

    template <typename T, typename... Ts>
    struct Index;

    template <typename T, typename... Ts>
    struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

    template <typename T, typename U, typename... Ts>
    struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

    template <typename T, typename... Ts>
    constexpr std::size_t Index_v = Index<T, Ts...>::value;

	class World {
		friend class AudioPlayer;
		friend class App;
        
        std::vector<entity_t> localToGlobal;
        std::queue<entity_t> available;
        
        friend class Entity;
        friend class Registry;
    public:
        template<typename T>
        class SparseSet{
            unordered_vector<T> dense_set;
            unordered_vector<entity_t> aux_set;
            std::vector<entity_t> sparse_set;
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename ... A>
            inline T& Emplace(entity_t local_id, A ... args){
                dense_set.emplace(args...);
                aux_set.emplace(local_id);
                sparse_set.resize(local_id+1,INVALID_ENTITY);  //ensure there is enough space for this id
                
                sparse_set[local_id] = dense_set.size()-1;
                return dense_set[dense_set.size()-1];
            }
            
            inline void Destroy(entity_t local_id){
                assert(local_id < sparse_set.size());
                assert(HasComponent(local_id)); // Cannot destroy a component on an entity that does not have one!
                // call the destructor
                dense_set.erase(dense_set.begin() + sparse_set[local_id]);
                aux_set.erase(aux_set.begin() + sparse_set[local_id]);

                if (!aux_set.empty()) {
                    // update the location it points
                    auto owner = aux_set[sparse_set[local_id]];
                    sparse_set[owner] = local_id;
                    
                }
                sparse_set[local_id] = INVALID_INDEX;
            }

            inline T& GetComponent(entity_t local_id){
                assert(HasComponent(local_id));
                return dense_set[sparse_set[local_id]];
            }
            
            inline bool HasComponent(entity_t local_id) const{
                return sparse_set[local_id] != INVALID_ENTITY;
            }
            
            auto begin(){
                return dense_set.begin();
            }
            
            auto end(){
                return dense_set.end();
            }
            
            auto begin() const{
                return dense_set.begin();
            }
            
            auto end() const{
                return dense_set.end();
            }
            
            // get by dense index, not by entity ID
            T& Get(entity_t idx){
                return dense_set[idx];
            }
            
            auto GetOwner(entity_t idx) const{
                return aux_set[idx];
            }
            
            auto DenseSize() const{
                return dense_set.size();
            }
            
            inline const decltype(dense_set)& GetDense() const{
                return dense_set;
            }
        };
    private:
        struct SparseSetErased{
            constexpr static size_t buf_size = sizeof(SparseSet<size_t>);   // we use size_t here because all SparseSets are the same size
            std::array<char, buf_size> buffer;
            std::function<void(entity_t id)> destroyFn;
            std::function<void(void)> deallocFn;
            std::function<void(entity_t, entity_t, World*)> moveFn;
            
            template<typename T>
            inline SparseSet<T>* GetSet() {
                return reinterpret_cast<SparseSet<T>*>(buffer.data());
            }
            
            // the discard parameter is here to make the template work
            template<typename T>
            SparseSetErased(T* discard) :
                destroyFn([&](entity_t local_id){
                    auto ptr = GetSet<T>();
                    if (ptr->HasComponent(local_id)){
                        ptr->Destroy(local_id);
                    }
                }),
                deallocFn([&]() {
                    GetSet<T>()->~SparseSet<T>();
                }),
                moveFn([&](entity_t localID, entity_t otherLocalID, World* otherWorld){
                    auto sp = GetSet<T>();
                    if (sp->HasComponent(localID)){
                        auto& comp = sp->GetComponent(localID);
                        otherWorld->EmplaceComponent<T,true>(otherLocalID, std::move(comp));
                        // then delete it from here
                        sp->Destroy(localID);
                    }
                })
            {
                static_assert(sizeof(SparseSet<T>) <= buf_size);
                new (buffer.data()) SparseSet<T>();
            }

            ~SparseSetErased() {
                deallocFn();
            }
        };
        
        std::unordered_map<RavEngine::ctti_t, SparseSetErased> componentMap;

        inline void Destroy(entity_t local_id){
            // go down the list of all component types registered in this world
            // and call destroy if the entity has that component type
            // possible optimization: vector of vector<ctti_t> to make this faster?
            for(const auto& pair : componentMap){
                pair.second.destroyFn(local_id);
            }
            // unset localToGlobal
            localToGlobal[local_id] = INVALID_ENTITY;
        }
        
        template<typename T>
        inline SparseSet<T>* MakeIfNotExists(){
            auto id = RavEngine::CTTI<T>();
            auto it = componentMap.find(id);
            if (it == componentMap.end()){
                T* discard; // to make the template work
                it = componentMap.emplace(std::make_pair(id,discard)).first;
            }
            auto ptr = (*it).second.template GetSet<T>();
            assert(ptr != nullptr);
            return ptr;
        }
        
        template<typename T, bool isMoving = false, typename ... A>
        inline T& EmplaceComponent(entity_t local_id, A ... args){
            auto ptr = MakeIfNotExists<T>();
            
            //detect if T constructor's first argument is an entity_t, if it is, then we need to pass that before args (pass local_id again)
            constexpr static auto data_ctor_nparams = refl::fields_number_ctor<T>(sizeof ... (A));
            
            if constexpr(data_ctor_nparams > 0 ){
                //constexpr bool isMoving = sizeof ... (A) == 1; && (std::is_rvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value || std::is_lvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value);
                
              
                using data_ctor_type = refl::as_tuple<T,sizeof ... (A)>;
                if constexpr(!isMoving && std::is_same<typename std::tuple_element<0, data_ctor_type>::type, entity_t>::value){
                    return ptr->Emplace(local_id, local_id, args...);
                }
                else{
                    return ptr->Emplace(local_id,args...);
                }
            }
            else{
                return ptr->Emplace(local_id,args...);
            }
        }

        template<typename T>
        inline T& GetComponent(entity_t local_id) {
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->GetComponent(local_id);
        }

        template<typename T>
        inline bool HasComponent(entity_t local_id) {
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->HasComponent(local_id);
        }
        
        template<typename T>
        inline void DestroyComponent(entity_t local_id){
            componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->Destroy(local_id);
        }
        
        template<typename T>
        inline SparseSet<T>* GetRange(){
            auto& set = componentMap.at(RavEngine::CTTI<T>());
            return set.template GetSet<T>();
        }
        
        template<typename T>
        inline void FilterValidityCheck(entity_t id, void* set, bool& satisfies){
            // in this order so that the first one the entity does not have aborts the rest of them
            satisfies = satisfies && static_cast<SparseSet<T>*>(set)->HasComponent(id);
        }
        
        template<typename T>
        inline T& FilterComponentGet(entity_t idx, void* ptr){
            return static_cast<SparseSet<T>*>(ptr)->Get(idx);
        }
       
        template<typename T>
        inline void* FilterGetSparseSet(){
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>();
        }
        
        entity_t CreateEntity();

    public:
        template<typename T, typename ... A>
        inline T CreatePrototype(A ... args){
            auto id = CreateEntity();
            T en;
            en.id = id;
            en.Create(args...);
            return en;
        }
        
        template<typename ... A, typename func>
        inline void Filter(const func& f){
            constexpr auto n_types = sizeof ... (A);
            static_assert(n_types > 0, "Must supply a type to query for");
            
            using primary_t = typename std::tuple_element<0, std::tuple<A...> >::type;
            
            if constexpr (n_types == 1){
                auto mainFilter = GetRange<primary_t>();
                for(size_t i = 0; i < mainFilter->DenseSize(); i++){
                    auto& item = mainFilter->Get(i);
                    f(GetCurrentFPSScale(),item);
                }
            }
            else{
                std::array<void*, n_types> ptrs{ FilterGetSparseSet<A>()...};
                auto mainFilter = static_cast<SparseSet<primary_t>*>(ptrs[0]);
                // does this entity have all of the other required components?
                for(size_t i = 0; i < mainFilter->DenseSize(); i++){
                    const auto owner = mainFilter->GetOwner(i);
                    if (EntityIsValid(owner)){
                        bool satisfies = true;
                        (FilterValidityCheck<A>(owner, ptrs[Index_v<A, A...>], satisfies), ...);
                        if (satisfies){
                            f(GetCurrentFPSScale(),FilterComponentGet<A>(i,ptrs[Index_v<A, A...>])...);
                        }
                    }
                }
            }
        }
        
        // this does not check if the entity actually has the component
        // instead it iterates over keys in the hashtable
        template<typename func_t>
        inline void EnumerateComponentsOn(entity_t local_id, const func_t& fn){
            for(auto& componentRow : componentMap){
                auto& sp_erased = componentRow.second;
                fn(sp_erased);
            }
        }
        
        // return the new local id
        inline entity_t AddEntityFrom(World* other,entity_t other_local_id){
            auto newID = CreateEntity();
            
            other->EnumerateComponentsOn(other_local_id, [&](SparseSetErased& sp_erased){
                // call the moveFn to move the other entity data into this
                sp_erased.moveFn(other_local_id,newID,this);
            });
            other->localToGlobal[other_local_id] = INVALID_ENTITY;
            return newID;
        }
        
        ~World();
        
		constexpr static uint8_t id_size = 8;
		Ref<Skybox> skybox;
        
        template<typename T, typename ... A>
        inline void EmplaceSystem(const T& system){
            //TODO: FIX (parallelize this with for_each)
            // need 2 things:
                // iterator update
                // for-each w/ function
            
            ECSTasks.emplace([this,system](){
                Filter<A...>(system);
            }).name(typeid(T).name());
        }
        
	private:
		std::atomic<bool> isRendering = false;
		char worldIDbuf [id_size];
		tf::Taskflow masterTasks;
        tf::Taskflow renderTasks;
        tf::Taskflow ECSTasks;
        tf::Task renderTaskModule;
        tf::Task ECSTaskModule;
        
        struct TypeErasureIterator{
            constexpr static auto size = sizeof(SparseSet<size_t>::const_iterator);
            
            char begin[size];
            char end[size];
            
            template<typename T>
            TypeErasureIterator(const T begin, const T end){
                // copy-construct into the buffers
                new (begin) decltype(begin)(begin);
                new (end) decltype(end)(end);
            }
        };
        
        
        SparseSet<struct StaticMesh>::const_iterator geobegin, geoend;
        SparseSet<struct Transform>::const_iterator transformbegin,transformend;
        SparseSet<struct InstancedStaticMesh>::const_iterator instancedBegin, instancedEnd;
        SparseSet<struct SkinnedMeshComponent>::const_iterator skinnedgeobegin, skinnedgeoend;
        
        		
		void CreateFrameData();
		
		void SetupTaskGraph();
		
		std::chrono::time_point<e_clock_t> time_now = e_clock_t::now();
		float currentFPSScale = 0.01f;
		
		//Entity list
		typedef locked_hashset<Ref<Entity>, SpinLock> EntityStore;
        struct dispatched_func{
            double runAtTime;
            Function<void(void)> func;
            dispatched_func(const decltype(runAtTime) rt,const decltype(func)& func) : func(func), runAtTime(rt){}
            dispatched_func(const dispatched_func& other){
                runAtTime = other.runAtTime;
                func = other.func;
            }
            dispatched_func(){}
        };
        UnorderedContiguousSet<std::shared_ptr<dispatched_func>> async_tasks;
        decltype(async_tasks)::iterator async_begin, async_end;
        RavEngine::Vector<size_t> ranFunctions;
	protected:
        // returns the "first" of a component type
        template<typename T>
        inline T& GetComponent(){
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->GetComponent(0);  //TODO: FIX 0
        }
        
		//physics system
		PhysicsSolver Solver;
		
		//fire-and-forget audio
		LinkedList<InstantaneousAudioSource> instantaneousToPlay;
		LinkedList<InstantaneousAmbientAudioSource> ambientToPlay;

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void PreTick(float fpsScale) {}
		void TickECS(float);
		
		void setupRenderTasks();
		
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void PostTick(float fpsScale) {}
		
		void OnAddComponent(Ref<Component>);
		void OnRemoveComponent(Ref<Component>);
		
		bool physicsActive = false;
		
    public:
		std::string_view worldID{ worldIDbuf,id_size };
		std::atomic<bool> newFrame = false;

		inline float GetCurrentFPSScale() const {
			return currentFPSScale;
		}
				
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

		World(bool skip = false);
		
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
		World(const decltype(skybox)& sk) : World() {
			skybox = sk;
		}

        inline void PlaySound(const InstantaneousAudioSource& ias){
			instantaneousToPlay.push_back(ias);
		}

        inline void PlayAmbientSound(const InstantaneousAmbientAudioSource& iaas) {
			ambientToPlay.push_back(iaas);
		}
		
		/**
		 Called by GameplayStatics when the final world is being deallocated
		 */
		inline void DeallocatePhysics() {
			Solver.DeallocatePhysx();
		}

		/**
		* Called when this world is made the active world for the App
		*/
		virtual void OnActivate() {}

		/**
		* Called when this world was the active world for the App but has been replaced by a different world
		*/
		virtual void OnDeactivate() {}
        
        /**
         Dispatch a function to run in a given number of seconds in the future
         @param func the function to run
         @param delaySeconds the delay in the future to run
         @note You must ensure data your function references is kept loaded when this function runs. For example, to keep an entity loaded, capture by value an owning pointer to it. In addition, do not make assumptions about what thread your dispatched function runs on.
         */
        void DispatchAsync(const Function<void(void)>& func, double delaySeconds);
        
        template<typename T>
        inline auto GetAllComponentsOfType(){
            std::optional<SparseSet<T>*> ret;
            if (componentMap.find(CTTI<T>()) != componentMap.end()){
                ret.emplace(GetRange<T>());
            }
            return ret;
        }
	};
}
