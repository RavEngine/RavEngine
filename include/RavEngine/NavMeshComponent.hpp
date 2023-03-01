#pragma once
#include "MeshAsset.hpp"
#include "IDebugRenderable.hpp"
#include <DetourNavMeshQuery.h>
#include <DetourDebugDraw.h>
#include "Queryable.hpp"
#include "GetApp.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"

namespace RavEngine{
    class NavMeshComponent : public IDebugRenderable, public Queryable<NavMeshComponent,IDebugRenderable>{
    private:
        class dtNavMesh* navMesh = nullptr;
        dtNavMeshQuery* navMeshQuery = nullptr;
        unsigned char* navData = nullptr;
        MeshAsset::Bounds bounds;
        mutable SpinLock mtx;

    public:
		using Queryable<NavMeshComponent,IDebugRenderable>::GetQueryTypes;
        struct Options{
            float cellSize = 0.3f;
            float cellHeight = 0.2f;
            float maxEdgeLen = 12.f;
            float maxSimplificationError = 1.3f;
            float maxVertsPerPoly = 6.f;
            float detailSampleDist = 6.f;
            float detailSampleMaxError = 1.f;
            
            struct Agent{
                float height = 2.0f;
                float radius = 0.6f;
                float maxClimb = 0.9f;
                float maxSlope = 45.f;
            } agent;
           
            float regionMinDimension = 8.f;
            float regionMergeDimension = 20.f;
            
            enum PartitionMethod{
                Watershed,  // best but slowest
                Monotone,   // worst but fastest
                Layer       // compromise, good for tiled w/ small-medium tiles
            } partitionMethod = Watershed;
            
        };
        
        /**
         Construct a mesh asset 
         */
        NavMeshComponent(Ref<MeshAsset> mesh, Options opt);
        
        void UpdateNavMesh(Ref<MeshAsset> mesh, Options opt);
        
        /**
         Calculate a route between two points
         @param start the start location of the path, in local coordinates to the owning entity
         @param end the end location of the path, in local coordinates to the owning entity
         @return list of coordinates composing the path
         */
        RavEngine::Vector<vector3> CalculatePath(const vector3& start, const vector3& end, uint16_t maxPoints = std::numeric_limits<uint16_t>::max());
        
        void DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const override {
            mtx.lock();
            duDebugDrawNavMesh(&GetApp()->GetRenderEngine(), *navMesh, 0);
            mtx.unlock();
        }
                
        virtual ~NavMeshComponent();
    };
}
