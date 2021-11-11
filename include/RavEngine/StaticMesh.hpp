#pragma once
#include "Material.hpp"
#include "MeshAsset.hpp"
#include "BuiltinMaterials.hpp"
#include "mathtypes.hpp"
#include "Entity.hpp"

namespace RavEngine {
    class StaticMesh : public Component, public Queryable<StaticMesh> {
    private:
        std::tuple<Ref<MeshAsset>, Ref<PBRMaterialInstance>> tuple;
        StaticMesh(Ref<MeshAsset> m){
            std::get<0>(tuple) = m;
        }
    public:

        StaticMesh(Ref<MeshAsset> m, Ref<PBRMaterialInstance> mat) : StaticMesh(m) {
			std::get<1>(tuple) = mat;
		}
		virtual ~StaticMesh(){}
		
		inline Ref<MeshAsset> getMesh() const{
			return std::get<0>(tuple);
		}

        /**
        Assign a material to this staticmesh
        @param mat the material instance to assign
        */
		inline void SetMaterial(Ref<PBRMaterialInstance> mat){
			std::get<1>(tuple) = mat;
		}

        /**
        @returns the currently assigned material
        */
        inline Ref<PBRMaterialInstance> GetMaterial() const{
			return std::get<1>(tuple);
        }
		
        constexpr inline const auto& getTuple() const{
			return tuple;
		}
    };

    class InstancedStaticMesh : public StaticMesh, public Queryable<InstancedStaticMesh>{
        UnorderedVector<soatransform> instanceRelativeTransform;
        UnorderedVector<matrix4> instanceOutputTransform;
    public:
        using Queryable<InstancedStaticMesh>::GetQueryTypes;
        InstancedStaticMesh(Ref<MeshAsset> m, Ref<PBRMaterialInstance> mat) : StaticMesh(m,mat){}
        
        /**
         Add entities by resize
         @param numInstances new number of instances to create. New instance count will be numInstances - count()
         */
        inline void Resize(uint32_t numInstances){
            instanceRelativeTransform.resize(numInstances);
            instanceOutputTransform.resize(numInstances);
        }
        
        /**
         Resize internal buffer in preparation for new instances
         @param numInstances new number of instance slots to create. New instance capacity will be numInstances - current capacity
         */
        inline void Reserve(uint32_t numInstances){
            instanceRelativeTransform.reserve(numInstances);
            instanceOutputTransform.reserve(numInstances);
        }
        
        /**
         Add a new instance
         @return reference to the created instance.
         */
        inline soatransform& AddInstance(){
            auto& item = instanceRelativeTransform.emplace();
            instanceOutputTransform.emplace();
            
            return item;
        }
        
        /**
         Delete an instance at an index
         */
        inline void RemoveInstance(size_t index){
            instanceRelativeTransform.erase(instanceRelativeTransform.begin() + index);
            instanceOutputTransform.erase(instanceOutputTransform.begin() + index);
        }
        
        /**
         @return instance by index
         */
        inline soatransform& operator[](uint32_t idx) {
            return instanceRelativeTransform[idx];
        }
        
        /**
         Get number of instances
         */
        inline size_t count() const{
            return instanceRelativeTransform.size();
        }
        
        inline void CalculateMatrices(){
            auto parentTransform = GetOwner().lock()->GetTransform().CalculateWorldMatrix();
            for(size_t i = 0; i < instanceRelativeTransform.size(); i++){
                auto& st = instanceRelativeTransform[i];
                instanceOutputTransform[i] = parentTransform * (glm::translate(matrix4(1), st.translate) * glm::toMat4(st.rotate) * glm::scale(matrix4(1), st.scale));
            }
        }
        
        inline const decltype(instanceOutputTransform)& GetAllTransforms() const{
            return instanceOutputTransform;
        }
    };
}
