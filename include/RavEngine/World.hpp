#pragma once
//
//  World.hpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//

#include "DataStructures.hpp"
#include "SpinLock.hpp"
#include <taskflow/taskflow.hpp>
#include "Types.hpp"
#include "AddRemoveAction.hpp"
#include "PolymorphicIndirection.hpp"
#include "SparseSet.hpp"
#include "SharedObject.hpp"
#if !RVE_SERVER
    #include "VRAMSparseSet.hpp"
    #include "BuiltinMaterials.hpp"
    #include "Light.hpp"
#else
    #include "Ref.hpp"
#endif
#include "Function.hpp"
#include "Utilities.hpp"
#include "CallableTraits.hpp"
#include "Format.hpp"
#include "Queue.hpp"

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

	class World : public RavEngine::virtual_enable_shared_from_this<World> {
		friend class AudioPlayer;
		friend class App;
        friend class PhysicsBodyComponent;
        Vector<entity_t> localToGlobal;
        Queue<entity_t> available;
        ConcurrentQueue<entity_t> destroyedAudioSources, destroyedMeshSources;

        class InstantaneousAudioSourceFreeList {
            entity_t nextID = INVALID_ENTITY - 1;
            ConcurrentQueue<entity_t> freeList;
        public:
            auto GetNextID() {
                entity_t id;
                if (freeList.try_dequeue(id)) {
                    return id;
                }
                else {
                    id = nextID;
                    nextID--;
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
        auto& GetLocalToGlobal() {
            return localToGlobal;
        }

        template<typename T>
        class EntitySparseSet{
            unordered_vector<T> dense_set;
            UnorderedVector<entity_t> aux_set;
            Vector<entity_t> sparse_set{INVALID_ENTITY};
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename ... A>
            inline T& Emplace(entity_t local_id, A&& ... args){
                auto& ret = dense_set.emplace(args...);
                aux_set.emplace(local_id);
                if (local_id >= sparse_set.size()){
                    sparse_set.resize(closest_multiple_of<int>(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                }
                
				sparse_set[local_id] = static_cast<typename decltype(sparse_set)::value_type>(dense_set.size()-1);
                return ret;
            }
            
            inline void Destroy(entity_t local_id){
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

            inline T& GetComponent(entity_t local_id){
                assert(HasComponent(local_id));
                return dense_set[sparse_set[local_id]];
            }
            
            inline auto SparseToDense(entity_t local_id){
                return sparse_set[local_id];
            }
            
            inline T& GetFirst(){
                assert(!dense_set.empty());
                return dense_set[0];
            }
            
            inline bool HasComponent(entity_t local_id) const{
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
            T& Get(entity_t idx){
                return dense_set[idx];
            }
            
            auto GetOwner(entity_t idx) const{
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
            Function<void(AnySparseSet*, entity_t, entity_t, World*)> _impl_moveFn;
        public:
            // avoid capture overhead by wrapping
            void destroyFn(entity_t id, World* world){
                _impl_destroyFn(this, id, world);
            }
            void deallocFn(){
                _impl_deallocFn(this);
            }
            void moveFn(entity_t id_a, entity_t id_b, World* world){
                _impl_moveFn(this, id_a, id_b, world);
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
                    if (ptr->HasComponent(local_id)){
                        wptr->DestroyComponent<T>(local_id);
                    }
                }),
                _impl_deallocFn([](AnySparseSet* thisptr) {
                    thisptr->GetSet<T>()->~EntitySparseSet<T>();
                }),
                _impl_moveFn([](AnySparseSet* thisptr, entity_t localID, entity_t otherLocalID, World* otherWorld){
                    auto sp = thisptr->GetSet<T>();
                    if (sp->HasComponent(localID)){
                        auto& comp = sp->GetComponent(localID);
                        otherWorld->EmplaceComponent<T>(otherLocalID, std::move(comp));
                        // then delete it from here
                        sp->Destroy(localID);
                    }
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
            RGLBufferPtr indirectBuffer, cullingBuffer, indirectStagingBuffer;
            ~MDICommandBase();
        };

        struct MDIICommand : public MDICommandBase {
            struct command {
                WeakRef<MeshCollectionStatic> mesh;
                using set_t = VRAMSparseSet<entity_t,entity_t>;
                set_t entities;
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
                using set_t = VRAMSparseSet<entity_t,entity_t>;
                set_t entities;
                command(decltype(mesh) mesh, decltype(skeleton) skeleton, set_t::index_type index, const set_t::value_type& first_value) : mesh(mesh), skeleton(skeleton) {
                    entities.Emplace(index, first_value);
                }
            };
            unordered_vector<command> commands;
        };
    
        struct DirLightUploadData {
            glm::mat4 lightViewProj;
            glm::vec3 color;
            glm::vec3 direction;
            float intensity;
            int castsShadows;
            int shadowmapBindlessIndex;
        };

        struct DirLightAuxData {
            float shadowDistance = 0;
        };

        struct PointLightUploadData {
            glm::vec3 position;
            glm::vec3 color;
            float intensity;
            int castsShadows;
            int shadowmapBindlessIndex;
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
        };

        // data for the render engine
        struct RenderData{
            template<typename uploadType, typename auxType>
            struct LightDataType{
                using upload_data_t = uploadType;
                using aux_data_t = auxType;
                
                uploadType uploadData;
                auxType auxData;
                
                constexpr static bool hasAuxData = ! std::is_same_v<auxType,void*>;
                
                void DefaultAddAt(entity_t sparseIndex){
                    uploadData.Emplace(sparseIndex);
                    if constexpr(hasAuxData){
                        auxData.Emplace(sparseIndex);
                    }
                }
                
                void EraseAtSparseIndex(entity_t sparseIndex){
                    uploadData.EraseAtSparseIndex(sparseIndex);
                    if constexpr(hasAuxData){
                        auxData.EraseAtSparseIndex(sparseIndex);
                    }
                }
                
            };
            
            
            LightDataType<VRAMSparseSet<entity_t, DirLightUploadData>, UnorderedSparseSet<entity_t, DirLightAuxData>> directionalLightData;
            
            LightDataType<VRAMSparseSet<entity_t, glm::vec4>, void*> ambientLightData;
            
            LightDataType<VRAMSparseSet<entity_t, PointLightUploadData>, void*> pointLightData;
             
            LightDataType<VRAMSparseSet<entity_t, SpotLightDataUpload>, void*> spotLightData;

            // uses world-local ID
            VRAMVector<matrix4> worldTransforms;

            locked_node_hashmap<Ref<MaterialInstance>, MDIICommand, phmap::NullMutex> staticMeshRenderData;
            locked_node_hashmap<Ref<MaterialInstance>, MDIICommandSkinned, phmap::NullMutex> skinnedMeshRenderData;
        };

        std::optional<RenderData> renderData;

        void updateStaticMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionStatic> mesh);
        void updateSkinnedMeshMaterial(entity_t localId, Ref<MaterialInstance> oldMat, Ref<MaterialInstance> newMat, Ref<MeshCollectionSkinned> mesh, Ref<SkeletonAsset> skeleton);
        void StaticMeshChangedVisibility(const StaticMesh*);
        void SkinnedMeshChangedVisibility(const SkinnedMeshComponent*);
#endif
        
    public:
        struct PolymorphicIndirection{
            struct elt{
                Function<void*(entity_t)> getfn;
                ctti_t full_id = 0;
                template<typename T>
                elt(World* world, T* discard) : full_id(CTTI<T>()){
                    auto setptr = world->componentMap.at(CTTI<T>()).template GetSet<T>();
                    getfn = [setptr](entity_t local_id) -> void*{
                        auto& thevalue = setptr->GetComponent(local_id);
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
            entity_t owner = INVALID_ENTITY;
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
                return BaseIncludingArgument(owner,elt.full_id);
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
            Vector<entity_t> sparse_set{INVALID_ENTITY};
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename T>
            inline void Emplace(entity_t local_id, World* world){
                //if a record for this does not exist, create it
                if (!HasForEntity(local_id)){
                    dense_set.emplace(local_id,world);
                    if (local_id >= sparse_set.size()){
                        sparse_set.resize(closest_multiple_of<entity_t>(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                    }
                    sparse_set[local_id] = static_cast<decltype(sparse_set)::value_type>(dense_set.size()-1);
                }
                
                // then push the Elt into it
                GetForEntity(local_id).push<T>();
            }
            
            template<typename T>
            inline void Destroy(entity_t local_id){
                // get the record, then call erase on it
                assert(HasForEntity(local_id));
                GetForEntity(local_id).erase<T>();
                    
                // if that makes the container empty, then delete it from the Sets
               if (GetForEntity(local_id).empty()){
                   auto denseidx = sparse_set[local_id];
                   dense_set.erase(dense_set.begin() + denseidx);

                   if (denseidx < dense_set.size()) {    // did a move happen during this deletion?
                       auto ownerOfMoved = dense_set[denseidx].owner;
                       sparse_set[ownerOfMoved] = denseidx;
                   }
                   sparse_set[local_id] = INVALID_INDEX;
               }
            }

            inline U& GetForEntity(entity_t local_id){
                assert(HasForEntity(local_id));
                return dense_set[sparse_set[local_id]];
            }

            inline auto SparseToDense(entity_t local_id){
                return sparse_set[local_id];
            }

            inline entity_t GetOwnerForDenseIdx(entity_t dense_idx) {
                assert(dense_idx < dense_set.size());
                // get the indirection object which is storing the owner
                return dense_set[dense_idx].owner;
            }

            
            inline bool HasForEntity(entity_t local_id) const{
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
            U& Get(entity_t idx){
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
            available.push(local_id);
            localToGlobal[local_id] = INVALID_ENTITY;
        }
        
        template<typename T>
        inline EntitySparseSet<T>* MakeIfNotExists(){
            return (*componentMap.try_emplace(RavEngine::CTTI<T>(),static_cast<T*>(nullptr)).first).second.template GetSet<T>();
        }
        
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
                if (renderData) {
                    renderData->directionalLightData.DefaultAddAt(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, AmbientLight>){
                if (renderData) {
                    renderData->ambientLightData.DefaultAddAt(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, PointLight>){
                if (renderData) {
                    renderData->pointLightData.DefaultAddAt(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, SpotLight>){
                if (renderData) {
                    renderData->spotLightData.DefaultAddAt(local_id);
                }
            }
#endif
            //detect if T constructor's first argument is an entity_t, if it is, then we need to pass that before args (pass local_id again)
            if constexpr(std::is_constructible<T,entity_t, A...>::value || (sizeof ... (A) == 0 && std::is_constructible<T,entity_t>::value)){
                return ptr->Emplace(local_id, localToGlobal[local_id], args...);
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
        inline auto GetAllComponentsPolymorphic(entity_t local_id){
            return polymorphicQueryMap.at(CTTI<T>()).GetForEntity(local_id).template GetAll<T>();
        }

        template<typename T>
        inline bool HasComponent(entity_t local_id) {
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->HasComponent(local_id);
        }
        
        template<typename T>
        inline bool HasComponentOfBase(entity_t local_id){
            return polymorphicQueryMap.at(CTTI<T>()).HasForEntity(local_id);
        }

        void DestroyStaticMeshRenderData(const StaticMesh& mesh, entity_t local_id);
        void DestroySkinnedMeshRenderData(const SkinnedMeshComponent& mesh, entity_t local_id);
        
        template<typename T>
        inline void DestroyComponent(entity_t local_id){

            auto setptr = componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>();
            // perform special cases
            if constexpr (RemoveAction<T>::HasCustomAction()){
                auto& comp = setptr->GetComponent(local_id);
                RemoveAction<T> obj;
                obj.DoAction(&comp);
            }

            if constexpr (std::is_same_v<T, StaticMesh>) {
                // remove the entry from the render data structure
                auto& comp = setptr->GetComponent(local_id);
                DestroyStaticMeshRenderData(comp, local_id);
            }
            else if constexpr (std::is_same_v<T, SkinnedMeshComponent>) {
                auto& comp = setptr->GetComponent(local_id);
                DestroySkinnedMeshRenderData(comp, local_id);
            }
#if !RVE_SERVER
            // if it's a light, register it in the container
            if constexpr (std::is_same_v<T, DirectionalLight>){
                if (renderData) {
                    renderData->directionalLightData.EraseAtSparseIndex(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, AmbientLight>){
                if (renderData) {
                    renderData->ambientLightData.EraseAtSparseIndex(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, PointLight>){
                if (renderData) {
                    renderData->pointLightData.EraseAtSparseIndex(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, SpotLight>){
                if (renderData) {
                    renderData->spotLightData.EraseAtSparseIndex(local_id);
                }
            }
            else if constexpr (std::is_same_v<T, AudioSourceComponent>) {
                destroyedAudioSources.enqueue(local_id);
            }
            else if constexpr (std::is_same_v<T, AudioMeshComponent>) {
                destroyedMeshSources.enqueue(local_id);
            }
#endif
            
            setptr->Destroy(local_id);
            // does this component have alternate query types
            if constexpr (HasQueryTypes<T>::value) {
                // polymorphic recordkeep
                const auto ids = T::GetQueryTypes();
                for (const auto id : ids) {
                    polymorphicQueryMap[id].template Destroy<T>(local_id);
                }
            }
        }
                
        template<typename T>
        inline EntitySparseSet<T>* GetRange(){
            auto& set = componentMap.at(RavEngine::CTTI<T>());
            return set.template GetSet<T>();
        }
        
        template<typename T, bool isPolymorphic = false>
        inline void FilterValidityCheck(entity_t id, void* set, bool& satisfies) const{
            // in this order so that the first one the entity does not have aborts the rest of them
            if constexpr (!isPolymorphic) {
                satisfies = satisfies && static_cast<EntitySparseSet<T>*>(set)->HasComponent(id);
            }
            else {
                satisfies = satisfies && static_cast<SparseSetForPolymorphic*>(set)->HasForEntity(id);
            }
        }
        
        template<typename T>
        inline T& FilterComponentGet(entity_t idx, void* ptr){
            return static_cast<EntitySparseSet<T>*>(ptr)->GetComponent(idx);
        }

        template<typename T>
        inline auto FilterComponentBaseMultiGet(entity_t idx, void* ptr) {
            auto& c_ptr = static_cast<SparseSetForPolymorphic*>(ptr)->GetForEntity(idx);
            return c_ptr.template GetAll<T>();
        }
        
        template<typename T>
        inline T& FilterComponentGetDirect(entity_t denseidx, void* ptr){
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
        
        template<typename funcmode_t, size_t n_types>
        struct FilterOneMode{
            funcmode_t& fm;
            const std::array<void*,n_types>& ptrs;
            FilterOneMode(funcmode_t& fm_i, const std::array<void*,n_types>& ptrs_i) : fm(fm_i),ptrs(ptrs_i){}
            static constexpr decltype(n_types) nTypes(){
                return n_types;
            }
            static constexpr bool isPolymorphic(){
                return funcmode_t::isPolymorphic();
            }
        };
        
        // for when the object needs to own the data, for outliving scopes
        template<typename funcmode_t, size_t n_types>
        struct FilterOneModeCopy{
            funcmode_t fm;
            const std::array<void*,n_types> ptrs;
            FilterOneModeCopy(const funcmode_t& fm_i, const std::array<void*,n_types>& ptrs_i) : fm(fm_i),ptrs(ptrs_i){}
            static constexpr decltype(n_types) nTypes(){
                return n_types;
            }
            static constexpr bool isPolymorphic(){
                return funcmode_t::isPolymorphic();
            }
        };

        template< bool isPolymorphic, typename ... A>
        bool DoesEntitySatisfyFilter(entity_t owner, const auto& pointerArray) {
            bool satisfies = true;
            uint32_t i = 0;
            ((FilterValidityCheck<A, isPolymorphic>(owner, pointerArray[i], satisfies), i++), ...);
            return satisfies;
        }
                
        template<typename ... A, typename filterone_t>
        inline void FilterOne(filterone_t& fom, entity_t i){
            using primary_t = typename std::tuple_element<0, std::tuple<A...> >::type;
            if constexpr(filterone_t::nTypes() == 1){
                if constexpr(!filterone_t::isPolymorphic()){
                    fom.fm.f(FilterComponentGetDirect<primary_t>(i,fom.ptrs[Index_v<primary_t, A...>]));
                }
                else{
                    //polymorphic query needs to be handled differently, because there can be multiple of a type on an entity
                    auto ptr_c = static_cast<SparseSetForPolymorphic*>(fom.ptrs[Index_v<primary_t, A...>]);
                    auto& indirection_obj = ptr_c->Get(i);
                    indirection_obj.for_each<primary_t>([&](auto comp_ptr){
                        fom.fm.f(*comp_ptr);
                    });
                }
            }
            else{
                entity_t owner;
                if constexpr (!filterone_t::isPolymorphic()) {
                    owner = static_cast<EntitySparseSet<primary_t>*>(fom.ptrs[0])->GetOwner(i);
                }
                else {
                    owner = static_cast<SparseSetForPolymorphic*>(fom.ptrs[0])->GetOwnerForDenseIdx(i);
                }
                if (EntityIsValid(owner)){
                    bool satisfies = DoesEntitySatisfyFilter<filterone_t::isPolymorphic(), A...>(owner, fom.ptrs);
                    if (satisfies){
                        if constexpr (!filterone_t::isPolymorphic()){
                            fom.fm.f(FilterComponentGet<A>(owner,fom.ptrs[Index_v<A, A...>])...);
                        }
                        else{
                            // Because there can be multiple base types per entity, per each Filter type in A,
                            // the user's function must take vectors of A, and decide how to process 
                            // multi-case
                            fom.fm.f(FilterComponentBaseMultiGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
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
                    FilterOneMode fom(fm, fd.ptrs);
                    for (entity_t i = 0; i < mainFilter->DenseSize(); i++) {
                        FilterOne<A...>(fom, i);
                    }
                }(std::type_identity<argtypes_noref>{});
            }(std::type_identity<argtypes>{});
        }
        void NetworkingSpawn(ctti_t,Entity&);
        void NetworkingDestroy(entity_t);
    public:
        
        template<typename T, typename ... A>
        inline T Instantiate(A&& ... args){
            auto id = CreateEntity();
            T en;
            en.id = id;
            en.Create(args...);
            NetworkingSpawn(CTTI<T>(),en);
            return en;
        }
        
        template<typename func>
        inline void Filter(func&& f){
            FilterGeneric(FuncMode<func, false>{ f });
        }
        
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
        
        // return the new local id
        inline entity_t AddEntityFrom(World* other,entity_t other_local_id){
            auto newID = CreateEntity();
            
            other->EnumerateComponentsOn(other_local_id, [&](AnySparseSet& sp_erased){
                // call the moveFn to move the other entity data into this
                sp_erased.moveFn(other_local_id,newID,this);
            });
            other->localToGlobal[other_local_id] = INVALID_ENTITY;
            return newID;
        }
        
        virtual ~World();
        
		constexpr static uint8_t id_size = 8;
		Ref<Skybox> skybox;
        
    private:
      
        template<bool polymorphic, typename T, typename ... Args>
        inline std::pair<tf::Task,tf::Task> EmplaceSystemGeneric(Args&& ... args){
            
            using argtypes = functor_args_t<T>;
            
            return
            // step 1: get it as types
            [this]<typename... Ts>(std::type_identity<std::tuple<Ts...>>, auto&& ... args) -> auto
            {
                using argtypes_noref = std::tuple<remove_polymorphic_arg_t<std::remove_const_t<std::remove_reference_t<Ts>>>...>;
                // step 2: get it as non-reference types, and slice off the first argument
                // because it's a float and we don't want it
                return
                [this]<typename ... A>(std::type_identity<std::tuple<A...>>, auto&& ... args) -> auto
                {
                    
                    // use `A...` here
                    
                    auto ptr = &ecsRangeSizes[CTTI<T>()];
                    
                    FuncModeCopy<T,polymorphic> fm{T(args...)};
                    
                    auto fd = GenFilterData<A...>(fm);
                    
                    FilterOneModeCopy fom(std::move(fm),fd.ptrs);
                    
                    auto setptr = fd.getMainFilter();
                    
                    // value update
                    auto range_update = ECSTasks.emplace([this,ptr,setptr](){
                        *ptr = static_cast<pos_t>(setptr->DenseSize());
                    }).name(Format("{} range update",type_name<T>()));
                    
                    auto do_task = ECSTasks.for_each_index(pos_t(0),std::ref(*ptr),pos_t(1),[this,fom](auto i) mutable{
                        FilterOne<A...>(fom,i);
                    }).name(Format("{}",type_name<T>().data()));
                    range_update.precede(do_task);
                    
                    auto pair = std::make_pair(range_update,do_task);
                    
                    typeToSystem[CTTI<T>()] = pair;
                    
                    return pair;
                    
                }(std::type_identity<argtypes_noref>{},args...);
            }(std::type_identity<argtypes>{},args...);
        }
        
        template< bool polymorphic,typename T, typename interval_t, typename ... Args>
        inline void EmplaceTimedSystemGeneric(const interval_t interval, Args&& ... args){
            auto task = EmplaceSystemGeneric<polymorphic,T>(args...);
            
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
            condition.precede(task.first);
        }
        
    public:
        template<typename T, typename U>
        inline void CreateDependency(){
            // T depends on (runs after) U
            auto& tPair = typeToSystem.at(CTTI<T>());
            auto& uPair = typeToSystem.at(CTTI<U>());
            
            tPair.second.succeed(uPair.second);
        }
        
        template<typename T, typename ... Args>
        inline auto EmplaceSystem(Args&& ... args){
            return EmplaceSystemGeneric<false,T>(args...);
        }

        template<typename T>
        inline void RemoveSystem() {
            auto& tpair = typeToSystem.at(CTTI<T>());
            ECSTasks.erase(tpair.first);
            ECSTasks.erase(tpair.second);
            typeToSystem.erase(CTTI<T>());
        }
        
        template<typename T, typename interval_t, typename ... Args>
        inline void EmplaceTimedSystem(const interval_t interval, Args&& ... args){
            EmplaceTimedSystemGeneric<false,T>(interval,args...);
        }
        
        template<typename T, typename ... Args>
        inline auto EmplacePolymorphicSystem(Args&& ... args){
            return EmplaceSystemGeneric<true,T>(args...);
        }
        
        template<typename T, typename interval_t, typename ... Args>
        inline void EmplacePolymorphicTimedSystem(const interval_t interval, Args&& ... args){
            EmplaceTimedSystemGeneric<true,T>(interval,args...);
        }
        
	private:
		std::atomic<bool> isRendering = false;
        char worldIDbuf [id_size]{0};
		tf::Taskflow masterTasks;
#if !RVE_SERVER
        tf::Taskflow renderTasks;
        tf::Taskflow audioTasks;
        tf::Task renderTaskModule;
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
        UnorderedMap<ctti_t, std::pair<tf::Task,tf::Task>> typeToSystem;
        				
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

        // returns the "first" of a component type
        template<typename T>
        inline T& GetComponent(){
            return componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>()->GetFirst();
        }
        
		std::string_view worldID{ worldIDbuf,id_size };
		std::atomic<bool> newFrame = false;

		inline float GetCurrentFPSScale() const {
			return currentFPSScale;
		}
        
        inline void ExportTaskGraph(std::ostream& out){
            masterTasks.dump(out);
        }
				
		/**
		* Initializes the physics-related Systems.
		* @return true if the systems were loaded, false if they were not loaded because they are already loaded
		*/
		bool InitPhysics();
		
		/**
		* Evaluate the world given a scale factor. One tick = 1/GetApp()->EvalNormal
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
