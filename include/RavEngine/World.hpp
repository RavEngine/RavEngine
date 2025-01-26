#pragma once
//
//  World.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "DataStructures.hpp"
#include "Vector.hpp"
#include "Queue.hpp"
#include "Map.hpp"
#include "SpinLock.hpp"
#include <taskflow/taskflow.hpp>
#include "Types.hpp"
#include "PolymorphicIndirection.hpp"
#include "SparseSet.hpp"
#if !RVE_SERVER
    #include "VRAMSparseSet.hpp"
    #include "BuiltinMaterials.hpp"
    #include "Light.hpp"
    #include "BufferedVRAMVector.hpp"
    #include "BufferPool.hpp"
#else
    #include "Ref.hpp"
#endif
#include "Debug.hpp"
#include "Function.hpp"
#include "Utilities.hpp"
#include "CallableTraits.hpp"
#include "Format.hpp"
#include "Queue.hpp"
#include "Layer.hpp"
#include "Profile.hpp"

namespace RavEngine {
	struct Entity;
	class InputManager;
	struct Entity;
	struct PhysicsCallback;
	struct StaticMesh;
	struct SkinnedMeshComponent;
    struct RenderEngine;
    struct Skybox;
    struct PhysicsSolver;
    class MeshAsset;
    class MeshAssetSkinned;
    class SkeletonAsset;
    struct InstantaneousAudioSource;
    struct InstantaneousAmbientAudioSource;
    struct AudioSourceComponent;
    struct InstantaneousAudioSourceToPlay;
    struct AudioMeshComponent;
    struct MeshCollectionStatic;
    struct MeshCollectionSkinned;
    class World;

    template <typename T, typename... Ts>
    struct Index;

    template <typename T, typename... Ts>
    struct Index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

    template <typename T, typename U, typename... Ts>
    struct Index<T, U, Ts...> : std::integral_constant<std::size_t, 1 + Index<T, Ts...>::value> {};

    template <typename T, typename... Ts>
    constexpr std::size_t Index_v = Index<T, Ts...>::value;

    template <typename T>
    class HasDestroy
    {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C> static YesType& test( decltype(&C::Destroy) ) ;
        template <typename C> static NoType& test(...);


    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template <typename T>
    class HasQueryTypes
    {
    private:
        typedef char YesType[1];
        typedef char NoType[2];

        template <typename C> static YesType& test( decltype(&C::GetQueryTypes) ) ;
        template <typename C> static NoType& test(...);


    public:
        enum { value = sizeof(test<T>(0)) == sizeof(YesType) };
    };

    template<typename T>
    concept SystemHasBefore = requires(T&& a) {
        a.before((World*)nullptr);
    };

    template<typename T>
    concept SystemHasAfter = requires(T && a) {
        a.after((World*)nullptr);
    };

    struct WorldDataProvider {
        World* world;
    };

    struct ValidatorProvider {

    };

    // if the System does not need Engine data, 
    // the FuncMode will default to this type
    struct DataProviderNone {};

    template <typename T>
    concept IsEngineDataProvider = 
        (std::is_convertible_v<T, WorldDataProvider> || std::is_convertible_v<T, ValidatorProvider>) 
        && not (std::is_convertible_v<T, DataProviderNone>);

	class World : public std::enable_shared_from_this<World> {
		friend class AudioPlayer;
		friend class App;
        friend class PhysicsBodyComponent;
        Queue<entity_id_t> available;
        Vector<uint8_t> versions;
        pos_t numEntities = 0;
        ConcurrentQueue<entity_id_t> destroyedAudioSources, destroyedMeshSources;

        class InstantaneousAudioSourceFreeList {
            entity_t nextID = {INVALID_ENTITY - 1};
            ConcurrentQueue<entity_t> freeList;
        public:
            auto GetNextID() {
                entity_t id;
                if (freeList.try_dequeue(id)) {
                    return id;
                }
                else {
                    id = nextID;
                    nextID.id--;
                }
                return id;
            }
            void ReturnID(entity_t id) {
                freeList.enqueue(id);
            }
        } instantaneousAudioSourceFreeList;
        
        friend class Entity;
        friend class Registry;
    public:
        template<typename T>
        class EntitySparseSet{
            unordered_vector<T> dense_set;
            UnorderedVector<entity_id_t> aux_set;
            Vector<entity_id_t> sparse_set{INVALID_ENTITY};
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename ... A>
            inline T& Emplace(entity_id_t local_id, A&& ... args){
                auto& ret = dense_set.emplace(std::forward<A>(args)...);
                aux_set.emplace(local_id);
                if (local_id >= sparse_set.size()){
                    sparse_set.resize(closest_multiple_of<int>(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                }
                
				sparse_set[local_id] = static_cast<typename decltype(sparse_set)::value_type>(dense_set.size()-1);
                return ret;
            }
            
            inline void Destroy(entity_id_t local_id){
                assert(local_id < sparse_set.size());
                assert(HasComponent(local_id)); // Cannot destroy a component on an entity that does not have one!
                // call the destructor
                if constexpr(HasDestroy<T>::value){
                    auto& oldvalue = GetComponent(local_id);
                    oldvalue.Destroy();
                }
                dense_set.erase(dense_set.begin() + sparse_set[local_id]);
                aux_set.erase(aux_set.begin() + sparse_set[local_id]);

                if (sparse_set[local_id] < aux_set.size()) {    // did a move happen during this deletion?
                    // update the location it points
                    auto owner = aux_set[sparse_set[local_id]];
                    sparse_set[owner] = sparse_set[local_id];
                    
                }
                sparse_set[local_id] = INVALID_INDEX;
            }

            inline T& GetComponent(entity_id_t local_id){
                assert(HasComponent(local_id));
                return dense_set[sparse_set[local_id]];
            }
            
            inline auto SparseToDense(entity_t local_id){
                return sparse_set[local_id.id];
            }
            
            inline T& GetFirst(){
                assert(!dense_set.empty());
                return dense_set[0];
            }
            
            inline bool HasComponent(entity_id_t local_id) const{
                return local_id < sparse_set.size() && sparse_set[local_id] != INVALID_ENTITY;
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
            T& Get(entity_id_t idx){
                return dense_set[idx];
            }
            
            auto GetOwner(entity_id_t idx) const{
                return aux_set[idx];
            }
            
            auto DenseSize() const{
                return dense_set.size();
            }
            
            auto GetDenseData() const{
                return dense_set.data();
            }
            
            inline const decltype(dense_set)& GetDense() const{
                return dense_set;
            }
        };
    private:
        class AnySparseSet{
            constexpr static size_t buf_size = sizeof(EntitySparseSet<size_t>);   // we use size_t here because all SparseSets are the same size
            std::array<char, buf_size> buffer;
            Function<void(AnySparseSet*,entity_t,World*)> _impl_destroyFn;
            Function<void(AnySparseSet*)> _impl_deallocFn;
        public:
            // avoid capture overhead by wrapping
            void destroyFn(entity_t id, World* world){
                _impl_destroyFn(this, id, world);
            }
            void deallocFn(){
                _impl_deallocFn(this);
            }
            
            template<typename T>
            inline EntitySparseSet<T>* GetSet() {
                return reinterpret_cast<EntitySparseSet<T>*>(buffer.data());
            }
            
            // the discard parameter is here to make the template work
            template<typename T>
            AnySparseSet(T* discard) :
                _impl_destroyFn([](AnySparseSet* thisptr, entity_t local_id, World* wptr){
                    auto ptr = thisptr->GetSet<T>();
                    if (ptr->HasComponent(local_id.id)){
                        wptr->DestroyComponent<T>(local_id);
                    }
                }),
                _impl_deallocFn([](AnySparseSet* thisptr) {
                    thisptr->GetSet<T>()->~EntitySparseSet<T>();
                })
            {
                static_assert(sizeof(EntitySparseSet<T>) <= buf_size);
                new (buffer.data()) EntitySparseSet<T>();
            }

            ~AnySparseSet() {
                deallocFn();
            }
        };
        
		locked_node_hashmap<RavEngine::ctti_t, AnySparseSet,SpinLock> componentMap;

#if !RVE_SERVER
        friend class StaticMesh;
        friend class SkinnedMeshComponent;
        friend class RenderEngine;
        // renderer-friendly representation of static meshes
        struct MDICommandBase {
            RGLBufferPtr indirectBuffer, cullingBuffer, indirectStagingBuffer, cuboBuffer, cuboStagingBuffer;
            ~MDICommandBase();
        };

        struct MDIICommand : public MDICommandBase {
            struct command {
                WeakRef<MeshCollectionStatic> mesh;
                using set_t = BufferedVRAMSparseSet<entity_id_t,entity_id_t>;
                set_t entities{"Static Mesh Private Entities Buffer"};
                command(decltype(mesh) mesh, set_t::index_type index, const set_t::value_type& first_value) : mesh(mesh) {
                    entities.Emplace(index, first_value);
                }
            };
            unordered_vector<command> commands;
        };

        struct MDIICommandSkinned : public MDICommandBase {
            struct command {
                WeakRef<MeshCollectionSkinned> mesh;
                WeakRef<SkeletonAsset> skeleton;
                using set_t = BufferedVRAMSparseSet<entity_id_t,entity_id_t>;
                set_t entities{"SKinned Mesh Private Entities Buffer"};
                command(decltype(mesh) mesh, decltype(skeleton) skeleton, set_t::index_type index, const set_t::value_type& first_value) : mesh(mesh), skeleton(skeleton) {
                    entities.Emplace(index, first_value);
                }
            };
            unordered_vector<command> commands;
        };
    
        struct DirLightUploadData {
            glm::vec3 color;
            glm::vec3 direction;
            float intensity;
            int castsShadows;
            int shadowmapBindlessIndex[MAX_CASCADES]{0};
            float cascadeDistances[MAX_CASCADES]{0};
            renderlayer_t shadowLayers;
            renderlayer_t illuminationLayers;
            uint32_t numCascades;
        };
        
        struct DirLightUploadDataPassVarying{
            glm::mat4 lightViewProj[MAX_CASCADES];
        };
        
        struct DirLightUploadDataPassVaryingHostOnly{
            glm::mat4 lightview[MAX_CASCADES];
            glm::mat4 lightProj[MAX_CASCADES];
        };
        
        struct AmbientLightUploadData{
            glm::vec3 color;
            float intensity;
            renderlayer_t illuminationLayers;
        };
        
        struct PointLightUploadData {
            glm::vec3 position;
            glm::vec3 color;
            float intensity;
            int castsShadows;
            int shadowmapBindlessIndex;
            renderlayer_t shadowLayers;
            renderlayer_t illuminationLayers;
        };

        struct SpotLightDataUpload {
            glm::mat4 lightViewProj;
            glm::mat4 worldTransform;
            glm::vec3 color;
            float intensity;
            float coneAngle;
            float penumbraAngle;
            int castsShadows;
            uint32_t shadowmapBindlessIndex;
            renderlayer_t shadowLayers;
            renderlayer_t illuminationLayers;
        };

        // data for the render engine
        struct RenderData{
            
            BufferedVRAMSparseSet<entity_id_t, DirLightUploadData> directionalLightData{"Directional Light Uniform Private Buffer"};
            
            BufferedVRAMSparseSet<entity_id_t, AmbientLightUploadData> ambientLightData{"Ambient Light Private Buffer"};
            
            BufferedVRAMSparseSet<entity_id_t, PointLightUploadData> pointLightData{"Point Light Private Buffer"};
             
            BufferedVRAMSparseSet<entity_id_t, SpotLightDataUpload> spotLightData{"Point Light Private Buffer"};
            
            BufferedVRAMVector<DirLightUploadDataPassVarying> directionalLightPassVarying{"Directional Light Pass Varying Buffer"};
            
            Vector<DirLightUploadDataPassVaryingHostOnly> directionalLightPassVaryingHostOnly;
            
            // uses world-local ID
            BufferedVRAMVector<renderlayer_t> renderLayers{32};

            BufferedVRAMVector<perobject_t> perObjectAttributes{ 32 };

            // uses world-local ID
            
            BufferedVRAMVector<matrix4> worldTransforms{"World Transform Private Buffer"};

            locked_node_hashmap<Ref<MaterialInstance>, MDIICommand, phmap::NullMutex> staticMeshRenderData;
            locked_node_hashmap<Ref<MaterialInstance>, MDIICommandSkinned, phmap::NullMutex> skinnedMeshRenderData;

            RGLBufferPtr cuboBuffer;

            SharedBufferPool stagingBufferPool;
        };

        RenderData renderData;

        void updateStaticMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionStatic> mesh);
        void updateSkinnedMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionSkinned> mesh, Ref<SkeletonAsset> skeleton);
        void StaticMeshChangedVisibility(const StaticMesh*);
        void SkinnedMeshChangedVisibility(const SkinnedMeshComponent*);
#endif
        
    public:
        /**
         @return the internal "Version" of the entity. This is for detecting use-after-destroy bugs.
         */
        auto VersionForEntity(entity_id_t id) const{
            return versions.at(id);
        }
        
        /**
         @return true if the entity handle has the correct version (is not stale), false otherwise.
         */
        bool CorrectVersion(entity_t id) const{
            return id.version == VersionForEntity(id.id);
        }
        
        struct PolymorphicIndirection{
            struct elt{
                Function<void*(entity_t)> getfn;
                ctti_t full_id = 0;
                template<typename T>
                elt(World* world, T* discard) : full_id(CTTI<T>()){
                    auto setptr = world->componentMap.at(CTTI<T>()).template GetSet<T>();
                    getfn = [setptr](entity_t local_id) -> void*{
                        auto& thevalue = setptr->GetComponent(local_id.id);
                        return &(thevalue);
                    };
                }
                
                template<typename T>
                T* Get(entity_t local_id){
                    return static_cast<T*>(getfn(local_id));
                }
                
                inline bool operator==(const elt& other) const{
                    return full_id == other.full_id;
                }
            };
            UnorderedVector<elt> elts;
            entity_t owner = {INVALID_ENTITY};
            World* world = nullptr;
            
            template<typename T>
            inline void push(){
                T* discard = nullptr;
                assert(std::find(elts.begin(),elts.end(),elt(world,discard)) ==elts.end());  // no duplicates
                elts.emplace(world,discard);
            }
            
            template<typename T>
            inline void erase(){
                T* discard = nullptr;
                elt value_erase(world,discard);
                auto it = std::find(elts.begin(),elts.end(),value_erase);
                assert(it != elts.end());
                elts.erase(it);
            }
            
            inline bool empty() const{
                return elts.empty();
            }
            
            PolymorphicIndirection(entity_t owner, World* world) : owner(owner), world(world){}
            
            /**
             Because there are multiple, one cannot simply "get" here.
             Instead, pass a function to invoke with each requested result.
             */
            template<typename T, typename func_t>
            inline void for_each(const func_t& f){
                for(auto& e : elts){
                    auto ptr = e.Get<T>(owner);
                    f(ptr);
                }
            }

            template<typename T>
            inline auto GetAll() {
                return PolymorphicGetResult<T,PolymorphicIndirection>{*this};
            }
            
            template<typename BaseIncludingArgument>
            inline auto HandleFor(int idx){
                auto& elt = elts.at(idx);
                return BaseIncludingArgument({owner,world},elt.full_id);
            }
			
			auto begin(){
				return elts.begin();
			}
			auto end(){
				return elts.end();
			}
			auto begin() const{
				return elts.begin();
			}
			auto end() const{
				return elts.end();
			}
        };
    private:
        int nCreatedThisTick = 0;
        class SparseSetForPolymorphic{
            using U = PolymorphicIndirection;
            unordered_vector<U> dense_set;
            Vector<entity_id_t> sparse_set{INVALID_ENTITY};
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename T>
            inline void Emplace(entity_t local_id_in, World* world){
                auto local_id = local_id_in.id;
                //if a record for this does not exist, create it
                if (!HasForEntity(local_id)){
                    dense_set.emplace(local_id_in,world);
                    if (local_id >= sparse_set.size()){
                        sparse_set.resize(closest_multiple_of<entity_id_t>(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                    }
                    sparse_set[local_id] = static_cast<decltype(sparse_set)::value_type>(dense_set.size()-1);
                }
                
                // then push the Elt into it
                GetForEntity(local_id).push<T>();
            }
            
            template<typename T>
            inline void Destroy(entity_id_t local_id){
                // get the record, then call erase on it
                assert(HasForEntity(local_id));
                GetForEntity(local_id).erase<T>();
                    
                // if that makes the container empty, then delete it from the Sets
               if (GetForEntity(local_id).empty()){
                   auto denseidx = sparse_set[local_id];
                   dense_set.erase(dense_set.begin() + denseidx);

                   if (denseidx < dense_set.size()) {    // did a move happen during this deletion?
                       auto ownerOfMoved = dense_set[denseidx].owner;
                       sparse_set[ownerOfMoved.id] = denseidx;
                   }
                   sparse_set[local_id] = INVALID_INDEX;
               }
            }

            inline U& GetForEntity(entity_id_t local_id){
                assert(HasForEntity(local_id));
                return dense_set[sparse_set[local_id]];
            }

            inline auto SparseToDense(entity_id_t local_id){
                return sparse_set[local_id];
            }

            inline auto GetOwnerForDenseIdx(entity_id_t dense_idx) {
                assert(dense_idx < dense_set.size());
                // get the indirection object which is storing the owner
                return dense_set[dense_idx].owner;
            }

            
            inline bool HasForEntity(entity_id_t local_id) const{
                return local_id < sparse_set.size() && sparse_set[local_id] != INVALID_ENTITY;
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
            U& Get(entity_id_t idx){
                return dense_set[idx];
            }
            
            auto DenseSize() const{
                return dense_set.size();
            }
            
            auto GetDenseData() const{
                return dense_set.data();
            }
            
            inline const decltype(dense_set)& GetDense() const{
                return dense_set;
            }
        };

        UnorderedNodeMap<ctti_t,SparseSetForPolymorphic> polymorphicQueryMap;

        inline void DestroyEntity(entity_t local_id){
            // go down the list of all component types registered in this world
            // and call destroy if the entity has that component type
            // possible optimization: vector of vector<ctti_t> to make this faster?
            NetworkingDestroy(local_id);
            for(auto& pair : componentMap){
                pair.second.destroyFn(local_id,this);
            }
            // unset localToGlobal
            available.push(local_id.id);
            versions[local_id.id]++;       // we increment it here so that if we have a use-after free, it's detected before the ID is recycled
        }
        
        template<typename T>
        inline EntitySparseSet<T>* MakeIfNotExists(){
            return (*componentMap.try_emplace(RavEngine::CTTI<T>(),static_cast<T*>(nullptr)).first).second.template GetSet<T>();
        }
        
        struct EntityRedir{
            World* owner;
            entity_t id;
            operator Entity() const;
        };
                
        template<typename T, typename ... A>
        inline T& EmplaceComponent(entity_t local_id, A&& ... args){
            auto ptr = MakeIfNotExists<T>();
            //constexpr bool isMoving = sizeof ... (A) == 1; && (std::is_rvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value || std::is_lvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value);
                        
            // does this component have alternate query types
            if constexpr (HasQueryTypes<T>::value){
                // polymorphic recordkeep
                const auto ids = T::GetQueryTypes();
                for(const auto id : ids){
                    polymorphicQueryMap[id].template Emplace<T>(local_id, this);
                }
            }
#if !RVE_SERVER
            // if it's a light, register it in the container
            if constexpr (std::is_same_v<T, DirectionalLight>){
                renderData.directionalLightData.Emplace(local_id.id);
            }
            else if constexpr (std::is_same_v<T, AmbientLight>){
                renderData.ambientLightData.Emplace(local_id.id);
            }
            else if constexpr (std::is_same_v<T, PointLight>){
                renderData.pointLightData.Emplace(local_id.id);
            }
            else if constexpr (std::is_same_v<T, SpotLight>){
                renderData.spotLightData.Emplace(local_id.id);
            }
#endif
            //detect if T constructor's first argument is an Entity, if it is, then we need to pass that before args (pass local_id again)
            if constexpr(std::is_constructible<T,Entity, A...>::value || (sizeof ... (A) == 0 && std::is_constructible<T,Entity>::value)){
                return ptr->Emplace(local_id.id, EntityRedir{this,local_id}, std::forward<A>(args)...);
            }
            else{
                return ptr->Emplace(local_id.id,std::forward<A>(args)...);
            }
        }

        template<typename T>
        inline T& GetComponent(entity_t local_id) {
            assert(CorrectVersion(local_id));
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->GetComponent(local_id.id);
        }
        
        template<typename T>
        inline auto GetAllComponentsPolymorphic(entity_t local_id){
            assert(CorrectVersion(local_id));
            return polymorphicQueryMap.at(CTTI<T>()).GetForEntity(local_id.id).template GetAll<T>();
        }

        template<typename T>
        inline bool HasComponent(entity_t local_id) {
            assert(CorrectVersion(local_id));
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->HasComponent(local_id.id);
        }
        
        template<typename T>
        inline bool HasComponentOfBase(entity_t local_id){
            assert(CorrectVersion(local_id));
            return polymorphicQueryMap.at(CTTI<T>()).HasForEntity(local_id);
        }

        void DestroyStaticMeshRenderData(const StaticMesh& mesh, entity_t local_id);
        void DestroySkinnedMeshRenderData(const SkinnedMeshComponent& mesh, entity_t local_id);
        
        template<typename T>
        inline void DestroyComponent(entity_t local_id){

            auto setptr = componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>();

            if constexpr (std::is_same_v<T, StaticMesh>) {
                // remove the entry from the render data structure
                auto& comp = setptr->GetComponent(local_id.id);
                DestroyStaticMeshRenderData(comp, local_id);
            }
            else if constexpr (std::is_same_v<T, SkinnedMeshComponent>) {
                auto& comp = setptr->GetComponent(local_id.id);
                DestroySkinnedMeshRenderData(comp, local_id);
            }
#if !RVE_SERVER
            // if it's a light, register it in the container
            if constexpr (std::is_same_v<T, DirectionalLight>){
                renderData.directionalLightData.EraseAtSparseIndex(local_id.id);
            }
            else if constexpr (std::is_same_v<T, AmbientLight>){
                renderData.ambientLightData.EraseAtSparseIndex(local_id.id);
            }
            else if constexpr (std::is_same_v<T, PointLight>){
                renderData.pointLightData.EraseAtSparseIndex(local_id.id);
            }
            else if constexpr (std::is_same_v<T, SpotLight>){
                renderData.spotLightData.EraseAtSparseIndex(local_id.id);
            }
            else if constexpr (std::is_same_v<T, AudioSourceComponent>) {
                destroyedAudioSources.enqueue(local_id.id);
            }
            else if constexpr (std::is_same_v<T, AudioMeshComponent>) {
                destroyedMeshSources.enqueue(local_id.id);
            }
#endif
            
            setptr->Destroy(local_id.id);
            // does this component have alternate query types
            if constexpr (HasQueryTypes<T>::value) {
                // polymorphic recordkeep
                const auto ids = T::GetQueryTypes();
                for (const auto id : ids) {
                    polymorphicQueryMap[id].template Destroy<T>(local_id.id);
                }
            }
        }
                
        template<typename T>
        inline EntitySparseSet<T>* GetRange(){
            auto& set = componentMap.at(RavEngine::CTTI<T>());
            return set.template GetSet<T>();
        }
        
        template<typename T, bool isPolymorphic = false>
        inline void FilterValidityCheck(entity_id_t id, void* set, bool& satisfies) const{
            // in this order so that the first one the entity does not have aborts the rest of them
            if constexpr (!isPolymorphic) {
                satisfies = satisfies && static_cast<EntitySparseSet<T>*>(set)->HasComponent(id);
            }
            else {
                satisfies = satisfies && static_cast<SparseSetForPolymorphic*>(set)->HasForEntity(id);
            }
        }
        
        template<typename T>
        inline T& FilterComponentGet(entity_id_t idx, void* ptr){
            return static_cast<EntitySparseSet<T>*>(ptr)->GetComponent(idx);
        }

        template<typename T>
        inline auto FilterComponentBaseMultiGet(entity_id_t idx, void* ptr) {
            auto& c_ptr = static_cast<SparseSetForPolymorphic*>(ptr)->GetForEntity(idx);
            return c_ptr.template GetAll<T>();
        }
        
        template<typename T>
        inline T& FilterComponentGetDirect(entity_id_t denseidx, void* ptr){
            return static_cast<EntitySparseSet<T>*>(ptr)->Get(denseidx);
        }
       
        template<typename T, bool isPolymorphic = false>
        inline void* FilterGetSparseSet(){
            if constexpr (!isPolymorphic){
                return MakeIfNotExists<T>();
            }
            else{
                return &polymorphicQueryMap[CTTI<T>()];
            }
        }
        
        template<typename T>
        inline EntitySparseSet<T>* FilterGetSparseSetTyped(){
            return MakeIfNotExists<T>();
        }
        
        entity_t CreateEntity();
        
        template<typename func, bool polymorphic>
        struct FuncMode{
            func& f;
            static constexpr bool isPolymorphic(){
                return polymorphic;
            }
        };
        
        template<typename func, bool polymorphic>
        struct FuncModeCopy{
            func f;
            static constexpr bool isPolymorphic(){
                return polymorphic;
            }
        };
        
        template<typename funcmode_t, size_t n_types, typename DataProviderType = DataProviderNone>
        struct FilterOneMode{
            using DataProvider_t = DataProviderType;
            funcmode_t& fm;
            const std::array<void*,n_types>& ptrs;
            FilterOneMode(funcmode_t& fm_i, const std::array<void*,n_types>& ptrs_i, const DataProviderType) : fm(fm_i),ptrs(ptrs_i){}
            static constexpr decltype(n_types) nTypes(){
                return n_types;
            }
            static constexpr bool isPolymorphic(){
                return funcmode_t::isPolymorphic();
            }
        };
        
        // for when the object needs to own the data, for outliving scopes
        template<typename funcmode_t, size_t n_types, typename DataProviderType = DataProviderNone>
        struct FilterOneModeCopy{
            using DataProvider_t = DataProviderType;
            funcmode_t fm;
            const std::array<void*,n_types> ptrs;
            FilterOneModeCopy(const funcmode_t& fm_i, const std::array<void*, n_types>& ptrs_i, const DataProviderType) : fm(fm_i), ptrs(ptrs_i) {}
            static constexpr decltype(n_types) nTypes(){
                return n_types;
            }
            static constexpr bool isPolymorphic(){
                return funcmode_t::isPolymorphic();
            }
        };

        template< bool isPolymorphic, typename ... A>
        bool DoesEntitySatisfyFilter(entity_id_t owner, const auto& pointerArray) {
            bool satisfies = true;
            uint32_t i = 0;
            ((FilterValidityCheck<A, isPolymorphic>(owner, pointerArray[i], satisfies), i++), ...);
            return satisfies;
        }

        template<typename provider_t>
        auto MakeEngineDataProvider(){
            provider_t instance;
            if constexpr (std::is_convertible_v<provider_t, WorldDataProvider>) {
                instance.world = this;
            }
            return instance;
        };
                
        template<typename ... A, typename filterone_t>
        inline void FilterOne(filterone_t& fom, entity_id_t i){
            using primary_t = typename std::tuple_element<0, std::tuple<A...> >::type;
            using dataProviderType = filterone_t::DataProvider_t;
            if constexpr(filterone_t::nTypes() == 1){
                if constexpr(!filterone_t::isPolymorphic()){
                    if constexpr (IsEngineDataProvider<dataProviderType>) {
                        const auto dp = MakeEngineDataProvider<dataProviderType>();
                        fom.fm.f(dp, FilterComponentGetDirect<primary_t>(i, fom.ptrs[Index_v<primary_t, A...>]));
                    }
                    else {
                        fom.fm.f(FilterComponentGetDirect<primary_t>(i, fom.ptrs[Index_v<primary_t, A...>]));
                    }
                    
                }
                else{
                    //polymorphic query needs to be handled differently, because there can be multiple of a type on an entity
                    auto ptr_c = static_cast<SparseSetForPolymorphic*>(fom.ptrs[Index_v<primary_t, A...>]);
                    auto& indirection_obj = ptr_c->Get(i);
                    indirection_obj.for_each<primary_t>([&](auto comp_ptr){
                        if constexpr (IsEngineDataProvider<dataProviderType>) {
                            const auto dp = MakeEngineDataProvider<dataProviderType>();
                            fom.fm.f(dp, *comp_ptr);
                        }
                        else {
                            fom.fm.f(*comp_ptr);
                        }
                    });
                }
            }
            else{
                entity_id_t owner;
                if constexpr (!filterone_t::isPolymorphic()) {
                    owner = static_cast<EntitySparseSet<primary_t>*>(fom.ptrs[0])->GetOwner(i);
                }
                else {
                    owner = static_cast<SparseSetForPolymorphic*>(fom.ptrs[0])->GetOwnerForDenseIdx(i).id;
                }
                if (EntityIsValid(owner)){
                    bool satisfies = DoesEntitySatisfyFilter<filterone_t::isPolymorphic(), A...>(owner, fom.ptrs);
                    if (satisfies){
                        if constexpr (!filterone_t::isPolymorphic()){
                            if constexpr (IsEngineDataProvider<dataProviderType>) {
                                const auto dp = MakeEngineDataProvider<dataProviderType>();
                                fom.fm.f(dp, FilterComponentGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
                            }
                            else {
                                fom.fm.f(FilterComponentGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
                            }
                            
                        }
                        else{
                            // Because there can be multiple base types per entity, per each Filter type in A,
                            // the user's function must take vectors of A, and decide how to process 
                            // multi-case
                            if constexpr (IsEngineDataProvider<dataProviderType>) {
                                const auto dp = MakeEngineDataProvider<dataProviderType>();
                                fom.fm.f(dp, FilterComponentBaseMultiGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
                            }
                            else {
                                fom.fm.f(FilterComponentBaseMultiGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
                            }
                           
                        }
                    }
                }
            }
        }
                
        template<typename ... A, typename funcmode>
        inline auto GenFilterData(const funcmode& fn){
            constexpr auto n_types = sizeof ... (A);
            static_assert(n_types > 0, "Must supply a type to query for");
            
            using primary_t = typename std::tuple_element<0, std::tuple<A...> >::type;

            struct FilterData{
                std::array<void*, n_types> ptrs;
                
                inline auto getMainFilter() const{
                    if constexpr (!funcmode::isPolymorphic()){
                        return static_cast<EntitySparseSet<primary_t>*>(ptrs[0]);
                    }
                    else{
                        return static_cast<EntitySparseSet<PolymorphicIndirection>*>(ptrs[0]);
                    }
                }
            } data {FilterGetSparseSet<A,funcmode::isPolymorphic()>()...};
                        
            return data;
        }

        // CTTI calls will fail on Polymorphic arguments, so
        // need to extract their inner types
        template<typename T>
        struct remove_polymorphic_arg {
            using type = T;
        };

        template <typename T>
        struct remove_polymorphic_arg<PolymorphicGetResult<T, World::PolymorphicIndirection>> {
            using type = T;
        };
        template<typename T> using remove_polymorphic_arg_t = typename remove_polymorphic_arg<T>::type;

        // "unit test" to sanity check the templates above
        static_assert(std::is_same_v<remove_polymorphic_arg_t<float>, remove_polymorphic_arg_t<PolymorphicGetResult<float, World::PolymorphicIndirection>>>, "template failed");

        template<typename funcmode_t>
        inline void FilterGeneric(const funcmode_t& fm) {
            using argtypes = decltype(arguments(fm.f));
            // step 1: get it as types
            [this,&fm] <typename... Ts>(std::type_identity<std::tuple<Ts...>>) -> void
            {
                using argtypes_noref = std::tuple<remove_polymorphic_arg_t<std::remove_const_t<std::remove_reference_t<Ts>>>...>;
                // step 2: get it as non-reference types, and slice off the first argument
                // because it's a float and we don't want it
                [this,&fm]<typename ... A>(std::type_identity<std::tuple<A...>>) -> void
                {
                    auto fd = GenFilterData<A...>(fm);
                    auto mainFilter = fd.getMainFilter();
                    FilterOneMode fom(fm, fd.ptrs, DataProviderNone{});
                    for (entity_id_t i = 0; i < mainFilter->DenseSize(); i++) {
                        FilterOne<A...>(fom, i);
                    }
                }(std::type_identity<argtypes_noref>{});
            }(std::type_identity<argtypes>{});
        }
        void NetworkingSpawn(ctti_t,Entity&);
        void NetworkingDestroy(entity_t);
        void SetupPerEntityRenderData(entity_t);
    public:
        
        /**
         Specify render layers for an entity
         @param globalid the entity ID
         @param layers the render layer bitmask
         */
        void SetEntityRenderlayer(entity_t globalid, renderlayer_t layers);

        /**
         Specify per-object rendering attributes
         @param globalid the entity ID
         @param layers the attribute bitmask
         */
        void SetEntityAttributes(entity_t globalid, perobject_t attributes);

        /**
         @param globalid the entity ID
         @return the per-object rendering attributes bitmask
         */
        perobject_t GetEntityAttributes(entity_t globalid);
        
        /**
         Add an entity of type @code T @endcode to the world. Invokes the @code Create @endcode function of T.
         @param args parameters to pass to @code Create @endcode
         @return a handle to the constructed entity.
         */
        template<typename T, typename ... A>
        inline T Instantiate(A&& ... args){
            auto id = CreateEntity();
            T en;
            en.id = id;
            en.world = this;
#if !RVE_SERVER
            SetupPerEntityRenderData(id);
#endif
            en.Create(args...);
            NetworkingSpawn(CTTI<T>(),en);
            return en;
        }
        
        /**
         Iterate the world, invoking a function for all entities with the requested components
         @param f the function to invoke. The parameters of @code f @endcode are the types used to query the scene
         */
        template<typename func>
        inline void Filter(func&& f){
            FilterGeneric(FuncMode<func, false>{ f });
        }
        
        /**
         Iterate the world, invoking a function for all entities with the requested components via a polymoprhic query.
         @param f the function to invoke. The parameters of @code f @endcode are the types used to query the scene
         */
        template<typename func>
        inline void FilterPolymorphic(func&& f){
            FilterGeneric(FuncMode<func, true>{ f });
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
        
        virtual ~World();
        
		constexpr static uint8_t id_size = 8;
		Ref<Skybox> skybox;
    private:
        bool graphWasModified = false;

        template<bool isSerial, bool polymorphic, typename T, typename ... Args>
        inline auto EmplaceSystemGeneric(Args&& ... args){
            graphWasModified = true;
            using argtypes = functor_args_t<T>;


            
            return
            // step 1: get it as types
            [this]<typename T1, typename... Ts>(std::type_identity<std::tuple<T1, Ts...>>, auto&& ... args) -> auto
            {
                using argtypes_noref_noDP = std::tuple<remove_polymorphic_arg_t<std::remove_const_t<std::remove_reference_t<T1>>>,remove_polymorphic_arg_t<std::remove_const_t<std::remove_reference_t<Ts>>>...>;
                using argtypes_noref_DP = std::tuple<remove_polymorphic_arg_t<std::remove_const_t<std::remove_reference_t<Ts>>>...>;
                // step 2: get it as non-reference types, and slice off the first argument
                // if it is an engine data provider
                
                auto innerfn = [this]<typename ... A, typename ... OrigTs, typename DataProviderT>(std::type_identity<std::tuple<A...>>, std::type_identity<std::tuple<OrigTs...>>, std::type_identity<DataProviderT>, auto&& ... args) -> auto
                {
                    
                    // use `A...` here
                    
                    auto ptr = &ecsRangeSizes[CTTI<T>()];
                    
                    FuncModeCopy<T,polymorphic> fm{T(std::forward<Args>(args)...)};
                    
                    auto fd = GenFilterData<A...>(fm);
                    
                    FilterOneModeCopy fom(std::move(fm), fd.ptrs, DataProviderT{});
                    
                    auto setptr = fd.getMainFilter();

                                   
                    // value update
                    auto range_update = ECSTasks.emplace([this,ptr,setptr](){
                        *ptr = static_cast<pos_t>(setptr->DenseSize());
                    }).name(Format("{} range update",type_name<T>()));
                    
                    tf::Task do_task;
                    std::optional<tf::Task> before, after;
                    if constexpr (isSerial) {
                        do_task = ECSTasks.emplace([this, ptr, fom] {
                            RVE_PROFILE_FN_N(type_name<T>().data());
                            if constexpr (SystemHasBefore<T>) {
                                fom.fm.f.before(this);
                            }
                            for (pos_t i = 0; i < std::ref(*ptr); i++) {
                                FilterOne<A...>(fom, i);
                            }
                            if constexpr (SystemHasAfter<T>) {
                                fom.fm.f.after(this);
                            }
                        }).name(Format("{} serial", type_name<T>().data()));
                    }
                    else {
                        do_task = ECSTasks.for_each_index(pos_t(0), std::ref(*ptr), pos_t(1), [this, fom](auto i) mutable {
                            FilterOne<A...>(fom, i);
                            }).name(Format("{}", type_name<T>().data()));
                        if constexpr (SystemHasBefore<T>) {
                            before.emplace(ECSTasks.emplace([fom, this] {
                                fom.fm.f.before(this);
                            }).name(Format("{}::before()", type_name<T>().data())));
                            range_update.precede(before.value());
                            do_task.succeed(before.value());
                        }
                        if constexpr (SystemHasAfter<T>) {
                            after.emplace(ECSTasks.emplace([fom, this] {
                                fom.fm.f.after(this);
                            }).name(Format("{}::after()", type_name<T>().data())));
                            do_task.precede(after.value());
                        }
                    }
                    
                    range_update.precede(do_task);
                    
                    SystemTasks tasks{
                        .rangeUpdate = range_update,
                        .do_task = do_task,
                        .preHook = before,
                        .postHook = after
                    };
                    if (typeToName.contains(CTTI<T>())) {
                        Debug::Fatal("System {}/{} has already been loaded!",CTTI<T>(),type_name<T>());
                    };

                    if constexpr (IsEngineDataProvider<DataProviderT> && std::is_convertible_v<DataProviderT, WorldDataProvider>) {
                        // validate: if the system uses the WorldProvider, it must be a serial system
                        static_assert(isSerial, "Systems that use WorldDataProvider must be serial");
                        tasks.usesWorldDataProvider = true;
                    }

                    typeToName[CTTI<T>()] = type_name<T>();
                    // enter parameter types into tracking structure
                    constexpr static auto enterType = []<typename pT>(std::vector<ctti_t>&reads, std::vector<ctti_t>&writes, auto & typeToName) {
                        using rpT = std::remove_reference_t<pT>;
                        constexpr auto name = type_name<rpT>();
                        constexpr auto name_system = type_name<T>();
                        constexpr auto id = CTTI<rpT>();
                        typeToName[id] = name;
                        if constexpr (std::is_const_v<rpT>) {
                            using nc_rpT = std::remove_const_t<rpT>;
                            constexpr auto id = CTTI<nc_rpT>();
                            typeToName[id] = name;
                            reads.push_back(id);
                        }
                        else {
                            writes.push_back(id);
                        }
                    };
                    (enterType.template operator()<OrigTs>(tasks.readDependencies, tasks.writeDependencies, typeToName), ...);
                    
                    typeToSystem[CTTI<T>()] = tasks;                    
                    return tasks;
                    
                };
                // based on if the system needs a dataprovider as arg 0
                if constexpr (IsEngineDataProvider<T1>) {
                    return innerfn(std::type_identity<argtypes_noref_DP>{}, std::type_identity<std::tuple<Ts...>>{}, std::type_identity<T1>{}, std::forward<Args>(args)...);
                }
                else {
                    return innerfn(std::type_identity<argtypes_noref_noDP>{}, std::type_identity<std::tuple<T1, Ts...>>{}, std::type_identity<DataProviderNone>{}, std::forward<Args>(args)...);
                }
                
            }(std::type_identity<argtypes>{}, std::forward<Args>(args)...);
        }
        
        template<bool isSerial, bool polymorphic,typename T, typename interval_t, typename ... Args>
        inline void EmplaceTimedSystemGeneric(const interval_t interval, Args&& ... args){
            auto task = EmplaceSystemGeneric<isSerial, polymorphic,T>(args...);
            
            auto c_interval = std::chrono::duration_cast<decltype(TimedSystemEntry::interval)>(interval);
            auto ts = &timedSystemRecords[CTTI<T>()];
            
            //conditional task - returns out-of-range if condition fails so that the task does not run
            auto condition = ECSTasks.emplace([this,ts,c_interval](){
                if (time_now - ts->last_timestamp > c_interval){
                    ts->last_timestamp = time_now;
                        return 0;
                }
                return 1;
            }).name("Check time");
            condition.precede(task.rangeUpdate);
        }

        void CheckSystems();
        
    public:
        /**
         Specify a dependency. T depends on U
         */
        template<typename T, typename U>
        inline void CreateDependency(){
            // T depends on (runs after) U
            auto& tPair = typeToSystem.at(CTTI<T>());
            auto& uPair = typeToSystem.at(CTTI<U>());

            tf::Task dependecyRoot;
            if (auto postHook = uPair.postHook) {
                dependecyRoot = postHook.value();
            }
            else {
                dependecyRoot = uPair.do_task;
            }

            tf::Task dependencyLeaf;
            if (auto preHook = tPair.preHook) {
                dependencyLeaf = preHook.value();
            }
            else {
                dependencyLeaf = tPair.do_task;
            }
            
            dependencyLeaf.succeed(dependecyRoot);
            graphWasModified = true;
        }
     
        /**
            Remove a system from the world by type.
         */
        template<typename T>
        void RemoveSystem() {
            auto& tpair = typeToSystem.at(CTTI<T>());
            ECSTasks.erase(tpair.rangeUpdate);
            ECSTasks.erase(tpair.do_task);
            if (auto before = tpair.preHook) {
                ECSTasks.erase(before.value());
            }
            if (auto after = tpair.postHook) {
                ECSTasks.erase(after.value());
            }
            typeToSystem.erase(CTTI<T>());

            // graphWasModified is not set to true here, because removing systems cannot introduce new hazards to an already-safe graph.
        }
        
        /**
         Instantiate a parallel system. T is the class/struct type of the system.
         @param args values to pass to the system constructor
        */
        template<typename T, typename ... Args>
        auto EmplaceSystem(Args&& ... args) {
            return EmplaceSystemGeneric<false, false, T>(std::forward<Args>(args)...);
        }

        /**
            Instantiate a parallel system that runs on an interval. T is the class/struct type of the system.
         @note The interval is is not a guarentee. I the engine is running too slowly to satisfy the interval, it will be called on a best-effort basis.
         @param interval the frequency of execution of the system.
          @param args values to pass to the system constructor
         */
        template<typename T, typename interval_t, typename ... Args>
        auto EmplaceTimedSystem(const interval_t interval, Args&& ... args){
            return EmplaceTimedSystemGeneric<false, false,T>(interval, std::forward<Args>(args)...);
        }
        
        /**
         Instantiate a parallel system that performs polymorphic queries to get component data. T is the class/struct type of the system.
         @param args values to pass to the system constructor
         */
        template<typename T, typename ... Args>
        auto EmplacePolymorphicSystem(Args&& ... args){
            return EmplaceSystemGeneric<false,true,T>(std::forward<Args>(args)...);
        }
        
        /**
         Instantiate a parallel system that performs polymorphic queries to get component data, and runs on an interval. T is the class/struct type of the system.
         @note The interval is is not a guarentee. I the engine is running too slowly to satisfy the interval, it will be called on a best-effort basis.
         @param args values to pass to the system constructor
         */
        template<typename T, typename interval_t, typename ... Args>
        auto EmplacePolymorphicTimedSystem(const interval_t interval, Args&& ... args){
            return EmplaceTimedSystemGeneric<true, true,T>(interval, std::forward<Args>(args)...);
        }

        /**
        Instantiate a parallel system. T is the class/struct type of the system.
        @param args values to pass to the system constructor
       */
        template<typename T, typename ... Args>
        auto EmplaceSerialSystem(Args&& ... args) {
            return EmplaceSystemGeneric<true, false, T>(std::forward<Args>(args)...);
        }

        /**
            Instantiate a parallel system that runs on an interval. T is the class/struct type of the system.
         @note The interval is is not a guarentee. I the engine is running too slowly to satisfy the interval, it will be called on a best-effort basis.
         @param interval the frequency of execution of the system.
          @param args values to pass to the system constructor
         */
        template<typename T, typename interval_t, typename ... Args>
        auto EmplaceSerialTimedSystem(const interval_t interval, Args&& ... args) {
            return EmplaceTimedSystemGeneric<true, false, T>(interval, std::forward<Args>(args)...);
        }

        /**
         Instantiate a parallel system that performs polymorphic queries to get component data. T is the class/struct type of the system.
         @param args values to pass to the system constructor
         */
        template<typename T, typename ... Args>
        auto EmplaceSerialPolymorphicSystem(Args&& ... args) {
            return EmplaceSystemGeneric<true, true, T>(std::forward<Args>(args)...);
        }

        /**
         Instantiate a parallel system that performs polymorphic queries to get component data, and runs on an interval. T is the class/struct type of the system.
         @note The interval is is not a guarentee. I the engine is running too slowly to satisfy the interval, it will be called on a best-effort basis.
         @param args values to pass to the system constructor
         */
        template<typename T, typename interval_t, typename ... Args>
        auto EmplaceSerialPolymorphicTimedSystem(const interval_t interval, Args&& ... args) {
            return EmplaceTimedSystemGeneric<true, true, T>(interval, std::forward<Args>(args)...);
        }
        
        const auto& getTypeToSystem() const {
            return typeToSystem;
        }
	private:
		std::atomic<bool> isRendering = false;
        char worldIDbuf [id_size]{0};
		tf::Taskflow masterTasks;
#if !RVE_SERVER
        tf::Taskflow renderTasks;
        tf::Taskflow audioTasks;
        tf::Task audioTaskModule;
#endif
        tf::Taskflow ECSTasks;
        tf::Task ECSTaskModule;
        
        struct TypeErasureIterator{
            constexpr static auto size = sizeof(EntitySparseSet<size_t>::const_iterator);
            
            char begin[size];
            char end[size];
            
            template<typename T>
            TypeErasureIterator(const T begin, const T end){
                // copy-construct into the buffers
                new (begin) decltype(begin)(begin);
                new (end) decltype(end)(end);
            }
        };   
                
        struct TimedSystemEntry{
             std::chrono::duration<double, std::micro> interval;
             std::chrono::time_point<e_clock_t> last_timestamp = e_clock_t::now();
        };
        UnorderedNodeMap<ctti_t, TimedSystemEntry> timedSystemRecords;
        UnorderedNodeMap<ctti_t, pos_t> ecsRangeSizes;

        struct SystemTasks {
            tf::Task rangeUpdate, do_task;
            std::optional<tf::Task>preHook, postHook;
            std::vector<ctti_t> readDependencies;
            std::vector<ctti_t> writeDependencies;
            bool usesWorldDataProvider = false;
        };
        UnorderedMap<ctti_t, SystemTasks> typeToSystem;
        UnorderedMap<ctti_t, std::string_view> typeToName;
        				
		void SetupTaskGraph();
		
		std::chrono::time_point<e_clock_t> time_now = e_clock_t::now();
		float currentFPSScale = 0.01f;
		
		//Entity list
        struct dispatched_func{
            double runAtTime = 0;
            Function<void(void)> func;
            dispatched_func(const decltype(runAtTime) rt,const decltype(func)& func) : func(func), runAtTime(rt){}
            dispatched_func(const dispatched_func& other){
                runAtTime = other.runAtTime;
                func = other.func;
            }
            dispatched_func(){}
        };
        UnorderedSet<std::shared_ptr<dispatched_func>> async_tasks;
        decltype(async_tasks)::iterator async_begin, async_end;
        RavEngine::Vector<std::shared_ptr<dispatched_func>> ranFunctions;
	protected:
        
		//physics system
        friend class PhysicsLinkSystemWrite;
		std::unique_ptr<PhysicsSolver> Solver;
		
		//fire-and-forget audio
#if !RVE_SERVER
       

		LinkedList<InstantaneousAudioSourceToPlay> instantaneousToPlay;
		LinkedList<InstantaneousAmbientAudioSource> ambientToPlay;
#endif

		/**
		Called before ticking components and entities synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void PreTick(float fpsScale) {}
		void TickECS(float);
#if !RVE_SERVER

		void setupRenderTasks();
#endif
		
		/**
		 Called after physics and rendering synchronously
		 @param fpsScale the scale factor calculated
		 */
		virtual void PostTick(float fpsScale) {}
		
		bool physicsActive = false;
		
    public:

        /**
         @return the "first" of a component type. Do not make assumptions regarding ordering of components.
         */
        template<typename T>
        inline T& GetComponent(){
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->GetFirst();
        }
        
		std::string_view worldID{ worldIDbuf,id_size };
		std::atomic<bool> newFrame = false;

        /**
         @return the current tick scale factor. If the engine is running above expected speed, this value will be (0,1). If the engine is running below expected speed, this value will be > 1.
         */
		inline float GetCurrentFPSScale() const {
			return currentFPSScale;
		}
        
        /**
         Log the task graph in graphivz format.
         */
        inline void ExportTaskGraph(std::ostream& out){
            masterTasks.dump(out);
        }
				
		/**
		Initializes the physics-related Systems.
		@return true if the systems were loaded, false if they were not loaded because they are already loaded
		*/
		bool InitPhysics();
		
		/**
		Evaluate the world given a scale factor. One tick = 1/GetApp()->EvalNormal
		@param the tick fraction to evaluate
		@note the GameplayStatics CurrentWorld is ticked automatically in the App
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
		Constructor that takes a custom skybox. This constructor will bypass loading the default skybox.
		@param sk the skybox to use
		*/
		World(const decltype(skybox)& sk) : World() {
			skybox = sk;
		}
#if !RVE_SERVER
        void PlaySound(const InstantaneousAudioSource& ias);

        void PlayAmbientSound(const InstantaneousAmbientAudioSource& iaas);
#endif
		
		/**
		 Called by GameplayStatics when the final world is being deallocated
		 */
        inline void DeallocatePhysics();

		/**
		* Called when this world is made the active world for the App
		*/
		virtual void OnActivate() {}

		/**
		* Called when this world was the active world for the App but has been replaced by a different world
		*/
		virtual void OnDeactivate() {}
        
        template<typename T>
        static inline PointerInputBinder<T> GetInput(T* ptr){
            return PointerInputBinder<T>(ptr);
        }
        
        /**
         Dispatch a function to run in a given number of seconds in the future
         @param func the function to run
         @param delaySeconds the delay in the future to run
         @note You must ensure data your function references is kept loaded when this function runs. For example, to keep an entity loaded, capture by value an owning pointer to it. In addition, do not make assumptions about what thread your dispatched function runs on.
         */
        void DispatchAsync(const Function<void(void)>& func, double delaySeconds);
        
        /**
         @return the internal data structure storing all components of a given type.
         @note Do not assume ordering of components within the structure.
         */
        template<typename T>
        inline auto GetAllComponentsOfType(){
            EntitySparseSet<T>* ret = nullptr;
            if (componentMap.find(CTTI<T>()) != componentMap.end()){
                ret = GetRange<T>();
            }
            return ret;
        }
	};
}
