#pragma once
#include "Component.hpp"
#include "MeshAsset.hpp"
#include <DetourNavMeshQuery.h>
#include "Queryable.hpp"

namespace RavEngine{
    class NavMeshComponent : public Component, public Queryable<NavMeshComponent>{
    private:
        class dtNavMesh* navMesh = nullptr;
        dtNavMeshQuery* navMeshQuery = nullptr;
        unsigned char* navData = nullptr;
        MeshAsset::Bounds bounds;
    public:
        struct Options{
            float cellSize = 0.3;
            float cellHeight = 0.2;
            float maxEdgeLen = 12;
            float maxSimplificationError = 1.3;
            float maxVertsPerPoly = 6;
            float detailSampleDist = 6;
            float detailSampleMaxError = 1;
            
            struct Agent{
                float height = 2.0;
                float radius = 0.6;
                float maxClimb = 0.9;
                float maxSlope = 45;
            } agent;
           
            float regionMinDimension = 8;
            float regionMergeDimension = 20;
            
            enum PartitionMethod{
                Watershed,  // best but slowest
                Monotone,   // worst but fastest
                Layer       // compromise, good for tiled w/ small-medium tiles
            } partitionMethod = Watershed;
            
        };
        
        /**
         Construct a mesh asset 
         */
        NavMeshComponent(Ref<MeshAsset> mesh, const Options& opt);
        
        /**
         Calculate a route between two points
         @param start the start location of the path, in local coordinates to the owning entity
         @param end the end location of the path, in local coordinates to the owning entity
         @return list of coordinates composing the path
         */
        std::vector<vector3> CalculatePath(const vector3& start, const vector3& end, uint16_t maxPoints = std::numeric_limits<uint16_t>::max());
                
        virtual ~NavMeshComponent();
    };
}
