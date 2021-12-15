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
#include "AddRemoveAction.hpp"
#include "PolymorphicIndirection.hpp"

namespace RavEngine {
	class Entity;
	class InputManager;
    struct Entity;
    struct PhysicsCallback;

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

	class World {
		friend class AudioPlayer;
		friend class App;
        friend class PhysicsBodyComponent;
        Vector<entity_t> localToGlobal;
        Queue<entity_t> available;
        
        friend class Entity;
        friend class Registry;
    public:
        template<typename T>
        class SparseSet{
            unordered_vector<T> dense_set;
            UnorderedVector<entity_t> aux_set;
            Vector<entity_t> sparse_set{INVALID_ENTITY};
            
        public:
            
            using const_iterator = typename decltype(dense_set)::const_iterator_type;
            
            template<typename ... A>
            inline T& Emplace(entity_t local_id, A ... args){
                auto& ret = dense_set.emplace(args...);
                aux_set.emplace(local_id);
                if (local_id >= sparse_set.size()){
                    sparse_set.resize(closest_multiple_of(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                }
                
                sparse_set[local_id] = dense_set.size()-1;
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
        struct SparseSetErased{
            constexpr static size_t buf_size = sizeof(SparseSet<size_t>);   // we use size_t here because all SparseSets are the same size
            std::array<char, buf_size> buffer;
            std::function<void(entity_t id,World*)> destroyFn;
            std::function<void(void)> deallocFn;
            std::function<void(entity_t, entity_t, World*)> moveFn;
            
            template<typename T>
            inline SparseSet<T>* GetSet() {
                return reinterpret_cast<SparseSet<T>*>(buffer.data());
            }
            
            // the discard parameter is here to make the template work
            template<typename T>
            SparseSetErased(T* discard) :
                destroyFn([&](entity_t local_id, World* wptr){
                    auto ptr = GetSet<T>();
                    if (ptr->HasComponent(local_id)){
                        wptr->DestroyComponent<T>(local_id);
                    }
                }),
                deallocFn([&]() {
                    GetSet<T>()->~SparseSet<T>();
                }),
                moveFn([&](entity_t localID, entity_t otherLocalID, World* otherWorld){
                    auto sp = GetSet<T>();
                    if (sp->HasComponent(localID)){
                        auto& comp = sp->GetComponent(localID);
                        otherWorld->EmplaceComponent<T>(otherLocalID, std::move(comp));
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
        
        UnorderedNodeMap<RavEngine::ctti_t, SparseSetErased> componentMap;
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
        };
    private:
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
                        sparse_set.resize(closest_multiple_of(local_id+1,2),INVALID_ENTITY);  //ensure there is enough space for this id
                    }
                    sparse_set[local_id] = dense_set.size()-1;
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
            for(const auto& pair : componentMap){
                pair.second.destroyFn(local_id,this);
            }
            // unset localToGlobal
            available.push(local_id);
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
        
        template<typename T, typename ... A>
        inline T& EmplaceComponent(entity_t local_id, A ... args){
            auto ptr = MakeIfNotExists<T>();
            //constexpr bool isMoving = sizeof ... (A) == 1; && (std::is_rvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value || std::is_lvalue_reference<typename std::tuple_element<0, std::tuple<A...>>::type>::value);
                        
            // does this component have alternate query types
            if constexpr (HasQueryTypes<T>::value){
                // polymorphic recordkeep
                const auto ids = T::GetQueryTypes();
                for(const auto id : ids){
                    T* discard;
                    polymorphicQueryMap[id].template Emplace<T>(local_id, this);
                }
            }
            
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
        
        template<typename T>
        inline void DestroyComponent(entity_t local_id){

            auto setptr = componentMap.at(RavEngine::CTTI<T>()).template GetSet<T>();
            // perform special cases
            if constexpr (RemoveAction<T>::HasCustomAction()){
                auto& comp = setptr->GetComponent(local_id);
                RemoveAction<T> obj;
                obj.DoAction(&comp);
            }
            
            setptr->Destroy(local_id);
            // does this component have alternate query types
            if constexpr (HasQueryTypes<T>::value){
                // polymorphic recordkeep
                const auto ids = T::GetQueryTypes();
                for(const auto id : ids){
                    polymorphicQueryMap[id].template Destroy<T>(local_id);
                }
            }
        }
                
        template<typename T>
        inline SparseSet<T>* GetRange(){
            auto& set = componentMap.at(RavEngine::CTTI<T>());
            return set.template GetSet<T>();
        }
        
        template<typename T, bool isPolymorphic = false>
        inline void FilterValidityCheck(entity_t id, void* set, bool& satisfies){
            // in this order so that the first one the entity does not have aborts the rest of them
            if constexpr (!isPolymorphic) {
                satisfies = satisfies && static_cast<SparseSet<T>*>(set)->HasComponent(id);
            }
            else {
                satisfies = satisfies && static_cast<SparseSetForPolymorphic*>(set)->HasForEntity(id);
            }
        }
        
        template<typename T>
        inline T& FilterComponentGet(entity_t idx, void* ptr){
            return static_cast<SparseSet<T>*>(ptr)->GetComponent(idx);
        }

        template<typename T>
        inline auto FilterComponentBaseMultiGet(entity_t idx, void* ptr) {
            auto& c_ptr = static_cast<SparseSetForPolymorphic*>(ptr)->GetForEntity(idx);
            return c_ptr.template GetAll<T>();
        }
        
        template<typename T>
        inline T& FilterComponentGetDirect(entity_t denseidx, void* ptr){
            return static_cast<SparseSet<T>*>(ptr)->Get(denseidx);
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
        inline SparseSet<T>* FilterGetSparseSetTyped(){
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
                
        template<typename ... A, typename filterone_t>
        inline void FilterOne(filterone_t& fom, size_t i, float scale){
            using primary_t = typename std::tuple_element<0, std::tuple<A...> >::type;
            if constexpr(filterone_t::nTypes() == 1){
                if constexpr(!filterone_t::isPolymorphic()){
                    fom.fm.f(scale,FilterComponentGetDirect<primary_t>(i,fom.ptrs[Index_v<primary_t, A...>]));
                }
                else{
                    //polymorphic query needs to be handled differently, because there can be multiple of a type on an entity
                    auto ptr_c = static_cast<SparseSetForPolymorphic*>(fom.ptrs[Index_v<primary_t, A...>]);
                    auto& indirection_obj = ptr_c->Get(i);
                    indirection_obj.for_each<primary_t>([&](auto comp_ptr){
                        fom.fm.f(scale,*comp_ptr);
                    });
                }
            }
            else{
                entity_t owner;
                if constexpr (!filterone_t::isPolymorphic()) {
                    owner = static_cast<SparseSet<primary_t>*>(fom.ptrs[0])->GetOwner(i);
                }
                else {
                    owner = static_cast<SparseSetForPolymorphic*>(fom.ptrs[0])->GetOwnerForDenseIdx(i);
                }
                if (EntityIsValid(owner)){
                    bool satisfies = true;
                    (FilterValidityCheck<A,filterone_t::isPolymorphic()>(owner, fom.ptrs[Index_v<A, A...>], satisfies), ...);
                    if (satisfies){
                        if constexpr (!filterone_t::isPolymorphic()){
                            fom.fm.f(scale,FilterComponentGet<A>(owner,fom.ptrs[Index_v<A, A...>])...);
                        }
                        else{
                            // Because there can be multiple base types per entity, per each Filter type in A,
                            // the user's function must take vectors of A, and decide how to process 
                            // multi-case
                            fom.fm.f(scale, FilterComponentBaseMultiGet<A>(owner, fom.ptrs[Index_v<A, A...>])...);
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
                        return static_cast<SparseSet<primary_t>*>(ptrs[0]);
                    }
                    else{
                        return static_cast<SparseSet<PolymorphicIndirection>*>(ptrs[0]);
                    }
                }
            } data {FilterGetSparseSet<A,funcmode::isPolymorphic()>()...};
                        
            return data;
        }

        template<typename ... A, typename funcmode_t>
        inline void FilterGeneric(const funcmode_t& fm) {
            auto scale = GetCurrentFPSScale();
            auto fd = GenFilterData<A...>(fm);
            auto mainFilter = fd.getMainFilter();
            FilterOneMode fom(fm, fd.ptrs);
            for (entity_t i = 0; i < mainFilter->DenseSize(); i++) {
                FilterOne<A...>(fom, i, scale);
            }
        }
        void NetworkingSpawn(ctti_t,Entity&);
        void NetworkingDestroy(entity_t);
    public:
        
        template<typename T, typename ... A>
        inline T CreatePrototype(A ... args){
            auto id = CreateEntity();
            T en;
            en.id = id;
            en.Create(args...);
            NetworkingSpawn(CTTI<T>(),en);
            return en;
        }
        
        template<typename ... A, typename func>
        inline void Filter(func& f){
            FilterGeneric<A...>(FuncMode<func, false>{ f });
        }
        
        template<typename ... A, typename func>
        inline void FilterPolymorphic(func& f){
            FilterGeneric<A...>(FuncMode<func, true>{ f });
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
        
        template<bool polymorphic, typename T, typename ... A, typename ... Args>
        inline std::pair<tf::Task,tf::Task> EmplaceSystemGeneric(Args... args){
            T system(args...);
            
            auto ptr = &ecsRangeSizes[CTTI<T>()];
            
            FuncModeCopy<T,polymorphic> fm{system};
            
            auto fd = GenFilterData<A...>(fm);
            
            FilterOneModeCopy fom(fm,fd.ptrs);
            
            auto setptr = fd.getMainFilter();
            
            // value update
            auto range_update = ECSTasks.emplace([this,ptr,setptr](){
                *ptr = setptr->DenseSize();
            }).name(StrFormat("{} range update",type_name<T>()));
            
            auto do_task = ECSTasks.for_each_index(pos_t(0),std::ref(*ptr),pos_t(1),[this,fom](auto i) mutable{
                auto scale = GetCurrentFPSScale();
                FilterOne<A...>(fom,i,scale);
            }).name(StrFormat("{}",type_name<T>().data()));
            range_update.precede(do_task);
            
            auto pair = std::make_pair(range_update,do_task);
            
            typeToSystem[CTTI<T>()] = pair;
            
            return pair;
        }
        
        template<typename T, typename U>
        inline void CreateDependency(){
            // T depends on (runs after) U
            auto& tPair = typeToSystem.at(CTTI<T>());
            auto& uPair = typeToSystem.at(CTTI<U>());
            
            tPair.second.succeed(uPair.second);
        }
        
        template< bool polymorphic,typename T, typename ... A, typename interval_t, typename ... Args>
        inline void EmplaceTimedSystemGeneric(const interval_t interval, Args ... args){
            auto task = EmplaceSystemGeneric<polymorphic,T,A...>(args...);
            
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
        
        template<typename T, typename ... A, typename ... Args>
        inline auto EmplaceSystem(Args... args){
            return EmplaceSystemGeneric<false,T,A...>(args...);
        }

        template<typename T>
        inline void RemoveSystem() {
            auto& tpair = typeToSystem.at(CTTI<T>());
            ECSTasks.erase(tpair.first);
            ECSTasks.erase(tpair.second);
            typeToSystem.erase(CTTI<T>());
        }
        
        template<typename T, typename ... A, typename interval_t, typename ... Args>
        inline void EmplaceTimedSystem(const interval_t interval, Args ... args){
            EmplaceTimedSystemGeneric<false,T,A...>(interval,args...);
        }
        
        template<typename T, typename ... A, typename ... Args>
        inline auto EmplacePolymorphicSystem(Args ... args){
            return EmplaceSystemGeneric<true,T,A...>(args...);
        }
        
        template<typename T, typename ... A, typename interval_t, typename ... Args>
        inline void EmplacePolymorphicTimedSystem(const interval_t interval, Args ... args){
            EmplaceTimedSystemGeneric<true,T,A...>(interval,args...);
        }
        
	private:
		std::atomic<bool> isRendering = false;
		char worldIDbuf [id_size];
		tf::Taskflow masterTasks;
        tf::Taskflow renderTasks;
        tf::Taskflow audioTasks;
        tf::Taskflow ECSTasks;
        tf::Task renderTaskModule;
        tf::Task ECSTaskModule;
        tf::Task audioTaskModule;
        
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
        SparseSet<struct InstancedStaticMesh>::const_iterator instancedBegin, instancedEnd;
        SparseSet<struct SkinnedMeshComponent>::const_iterator skinnedgeobegin, skinnedgeoend;
        
        struct TimedSystemEntry{
             std::chrono::duration<double, std::micro> interval;
             std::chrono::time_point<e_clock_t> last_timestamp = e_clock_t::now();
        };
        UnorderedNodeMap<ctti_t, TimedSystemEntry> timedSystemRecords;
        UnorderedNodeMap<ctti_t, pos_t> ecsRangeSizes;
        UnorderedMap<ctti_t, std::pair<tf::Task,tf::Task>> typeToSystem;
        		
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
		World(const std::string& name, bool skip = false) : World(skip){
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
        
        template<typename T>
        class WorldInputBinder{
            T* ptr;
        public:
            WorldInputBinder(decltype(ptr) ptr) : ptr(ptr){}

            size_t get_id() const{
                return reinterpret_cast<size_t>(ptr);
            }
            T* get() const{
                return ptr;
            }
        };
        
        template<typename T>
        static inline WorldInputBinder<T> GetInput(T* ptr){
            return WorldInputBinder<T>(ptr);
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
            std::optional<SparseSet<T>*> ret;
            if (componentMap.find(CTTI<T>()) != componentMap.end()){
                ret.emplace(GetRange<T>());
            }
            return ret;
        }
	};
}
