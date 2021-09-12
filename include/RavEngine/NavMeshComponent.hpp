#pragma once
#include "Component.hpp"
#include "MeshAsset.hpp"
#include <DetourNavMeshQuery.h>

namespace RavEngine{
    class NavMeshComponent : public Component{
    private:
        class dtNavMesh* navMesh = nullptr;
        dtNavMeshQuery* navMeshQuery = nullptr;
        unsigned char* navData = nullptr;
    public:
        struct NavMeshOptions{
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
        NavMeshComponent(Ref<MeshAsset> mesh, const NavMeshOptions& opt);
                
        virtual ~NavMeshComponent();
    };
}
