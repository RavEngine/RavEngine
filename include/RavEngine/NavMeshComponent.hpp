#pragma once
#include "Component.hpp"
#include "MeshAsset.hpp"

namespace RavEngine{
    class NavMeshComponent : public Component{
    public:
        struct NavMeshOptions{
            float cellSize;
            float cellHeight;
            float maxEdgeLen;
            float maxSimplificationError;
            float maxVertsPerPoly;
            float detailSampleDist;
            float detailSampleMaxError;
            
            struct Agent{
                float height = 1.0;
                float radius = 0.5;
                float maxClimb;
                float maxSlope;
            } agent;
           
            float regionMinDimension;
            float regionMergeDimension;
            float vertsPerPoly;
            
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
    };
}
