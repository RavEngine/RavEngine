#include "NavMeshComponent.hpp"
#include "Debug.hpp"
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourCommon.h>
#include <DetourCrowd.h>

using namespace std;
using namespace RavEngine;

NavMeshComponent::NavMeshComponent(Ref<MeshAsset> mesh, const Options& opt){
    Debug::Assert(mesh->hasSystemRAMCopy(),"MeshAsset must be created with keepInSystemRAM = true");
    
    auto& rawData = mesh->GetSystemCopy();
    bounds = mesh->GetBounds();
    
    const float* bmin = bounds.min;
    const float* bmax = bounds.max;
    
    const auto nverts = rawData.vertices.size();
    vector<Vertex> vertsOnly(rawData.vertices.size());
    for(uint32_t i = 0; i < rawData.vertices.size(); i++){
        vertsOnly[i] = rawData.vertices[i]; // uses object-slicing to copy only position data
    }
    
    // step 1: setup configuration
    rcConfig cfg;
    memset(&cfg,0,sizeof(cfg));
    cfg.cs = opt.cellSize;
    cfg.ch = opt.cellHeight;
    cfg.walkableSlopeAngle = opt.agent.maxSlope;
    cfg.walkableHeight = ceilf(opt.agent.height / cfg.ch);
    cfg.walkableClimb = ceilf(opt.agent.maxClimb / cfg.ch);
    cfg.walkableRadius = ceilf(opt.agent.radius / cfg.cs);
    cfg.maxEdgeLen = opt.maxEdgeLen / opt.cellSize;
    cfg.maxSimplificationError = opt.maxSimplificationError;
    cfg.minRegionArea = rcSqr(opt.regionMinDimension);
    cfg.mergeRegionArea = rcSqr(opt.regionMergeDimension);
    cfg.maxVertsPerPoly = opt.maxVertsPerPoly;
    cfg.detailSampleDist = opt.detailSampleDist < 0.9? 0 : opt.cellSize * opt.detailSampleDist;
    cfg.detailSampleMaxError = opt.cellHeight * opt.detailSampleMaxError;
    
    // setup bounds
    rcVcopy(cfg.bmin, bmin);
    rcVcopy(cfg.bmax, bmax);
    rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);
    
    rcContext ctx(0);
    
    // step 2: rasterize input polygon
    auto solid = rcAllocHeightfield();
    if (!solid){
        Debug::Fatal("Build nagivation failed: out of memory");
    }
    if (!rcCreateHeightfield(&ctx, *solid, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch)){
        Debug::Fatal("Height field generation failed");
    }
    
    // allocate array to hold triangle area types ( = number of triangles)
    unsigned char* triareas = new unsigned char[rawData.indices.size()/3];
    std::memset(triareas, 0, (rawData.indices.size()/3) * sizeof(triareas[0]));
    rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertsOnly.data()->position, nverts, reinterpret_cast<const int*>(rawData.indices.data()), rawData.indices.size() / 3, triareas);
    if(!rcRasterizeTriangles(&ctx, vertsOnly.data()->position, triareas, vertsOnly.size(), *solid)){
        Debug::Fatal("Could not rasterize triangles for navigation");
    }
    
    delete[] triareas;  // don't need this anymore
    
    // step 3: filter walkable areas
    rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
    rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
    rcFilterWalkableLowHeightSpans(&ctx,cfg.walkableHeight, *solid);
    
    // step 4: partition walkable surfaces to simple regions
    auto chf = rcAllocCompactHeightfield();
    if (!chf){
        Debug::Fatal("Failed to allocate compact height field");
    }
    if (!rcBuildCompactHeightfield(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid, *chf)){
        Debug::Fatal("Compact height field generation failed");
    }
    rcFreeHeightField(solid);   // don't need this anymore
    solid = nullptr;
    
    // Erode walkable area by agent radius
    if (!rcErodeWalkableArea(&ctx, cfg.walkableRadius, *chf)){
        Debug::Fatal("Walkable radius erode failed");
    }
    
    switch(opt.partitionMethod){
        case Options::Watershed:{
            if (!rcBuildDistanceField(&ctx, *chf)){
                Debug::Fatal("Distance field generation failed");
            }
            if (!rcBuildRegions(&ctx,*chf,0,cfg.minRegionArea,cfg.mergeRegionArea)){
                Debug::Fatal("Region generation failed");
            }
        }
        break;
        case Options::Monotone:{
            if (!rcBuildRegionsMonotone(&ctx,*chf,0,cfg.minRegionArea,cfg.mergeRegionArea)){
                Debug::Fatal("Monotone region generation failed");
            }
        }
        break;
        case Options::Layer:{
            if (!rcBuildLayerRegions(&ctx,*chf,0,cfg.minRegionArea)){
                Debug::Fatal("Layer region generation failed");
            }
        }
        break;
    }
    
    // step 5: trace and simplify region contours
    auto cset = rcAllocContourSet();
    if (!cset){
        Debug::Fatal("Could not allocate contour set");
    }
    if (!rcBuildContours(&ctx, *chf, cfg.maxSimplificationError, cfg.maxEdgeLen, *cset)){
        Debug::Fatal("Contour generation failed");
    }
    
    // step 6: build polygon mesh from contours
    auto pmesh = rcAllocPolyMesh();
    if (!pmesh){
        Debug::Fatal("PolyMesh allocation failed");
    }
    if(!rcBuildPolyMesh(&ctx, *cset, cfg.maxVertsPerPoly, *pmesh)){
        Debug::Fatal("Contour triangulation failed");
    }
    
    // step 7: create detail mesh to approximate height on each polygon
    auto dmesh = rcAllocPolyMeshDetail();
    if (!dmesh){
        Debug::Fatal("Detail mesh allocation failed");
    }
    if (!rcBuildPolyMeshDetail(&ctx, *pmesh, *chf, cfg.detailSampleDist, cfg.detailSampleMaxError, *dmesh)){
        Debug::Fatal("Detail mesh generation failed");
    }
    
    // no longer need these
    rcFreeCompactHeightfield(chf);
    rcFreeContourSet(cset);
    chf = nullptr;
    cset = nullptr;
    
    // step 8: create detour data
    if (cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON){
        int navDataSize = 0;
        
        dtNavMeshCreateParams params;
        memset(&params, 0, sizeof(params));
        params.verts = pmesh->verts;
        params.vertCount = pmesh->nverts;
        params.polys = pmesh->polys;
        params.polyAreas = pmesh->areas;
        params.polyFlags = pmesh->flags;
        params.polyCount = pmesh->npolys;
        params.nvp = pmesh->nvp;
        params.detailMeshes = dmesh->meshes;
        params.detailVerts = dmesh->verts;
        params.detailVertsCount = dmesh->nverts;
        params.detailTris = dmesh->tris;
        params.detailTriCount = dmesh->ntris;
        params.offMeshConVerts = nullptr; // m_geom->getOffMeshConnectionVerts();
        params.offMeshConRad = nullptr; //m_geom->getOffMeshConnectionRads();
        params.offMeshConDir = nullptr; // m_geom->getOffMeshConnectionDirs();
        params.offMeshConAreas = nullptr; // m_geom->getOffMeshConnectionAreas();
        params.offMeshConFlags = nullptr; // m_geom->getOffMeshConnectionFlags();
        params.offMeshConUserID = nullptr; // m_geom->getOffMeshConnectionId();
        params.offMeshConCount = 0; // m_geom->getOffMeshConnectionCount();
        params.walkableHeight = opt.agent.height;
        params.walkableRadius = opt.agent.radius;
        params.walkableClimb = opt.agent.maxClimb;
        rcVcopy(params.bmin, pmesh->bmin);
        rcVcopy(params.bmax, pmesh->bmax);
        params.cs = cfg.cs;
        params.ch = cfg.ch;
        params.buildBvTree = true;
        
        if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
        {
            Debug::Fatal("Detour mesh data creation failed");
        }
        
        auto navMesh = dtAllocNavMesh();
        if (!navMesh)
        {
            dtFree(navData);
            Debug::Fatal("Detour mesh allocaton failed");
        }
        
        dtStatus status;
        
        status = navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
        if (dtStatusFailed(status))
        {
            dtFree(navData);
            Debug::Fatal("Could not init Detour navmesh");
        }
        navMeshQuery = dtAllocNavMeshQuery();
        if (!navMeshQuery){
            Debug::Fatal("Could not allocate navmesh query");
        }
        
        status = navMeshQuery->init(navMesh, 2048);
        if (dtStatusFailed(status))
        {
            Debug::Fatal("Could not init Detour navmesh query");
        }
    }
    else{
        Debug::Warning("Cannot generate Detour data for NavMesh - too many vertices");
    }
}

NavMeshComponent::~NavMeshComponent(){
    dtFree(navMesh);
    dtFree(navMeshQuery);
    dtFree(navData);
}

std::vector<vector3> NavMeshComponent::CalculatePath(const vector3 &start, const vector3 &end, uint16_t maxPoints){
    float startf[3]{static_cast<float>(start.x),static_cast<float>(start.y),static_cast<float>(start.z)};
    float endf[3]{static_cast<float>(end.x),static_cast<float>(end.y),static_cast<float>(end.z)};
    
    dtPolyRef ref;
    
    auto midpoint = [](auto f1, auto f2){
        return (f1+f2)/2;
    };
    
    float center[3] = {midpoint(bounds.min[0],bounds.max[0]),midpoint(bounds.min[1],bounds.max[1]),midpoint(bounds.min[2],bounds.max[2])};
    float halfexts[3] = {std::abs(center[0]-bounds.min[0]),std::abs(center[1]-bounds.min[1]),std::abs(center[2]-bounds.min[2])};
    
    // get polyref
    float nearestpt[3];
    float endpt[3];
    
    dtQueryFilter filter;
    filter.setIncludeFlags(0xFFF);
    filter.setExcludeFlags(0);
    //filter.setAreaCost(0, 1.0f);  // TODO: replace 0 with named region enum
    
    dtStatus status;
    dtPolyRef startPoly;
    dtPolyRef endPoly;
    status = navMeshQuery->findNearestPoly(startf, halfexts, &filter, &startPoly, nearestpt);
    if (status & DT_FAILURE || status & DT_STATUS_DETAIL_MASK){
        Debug::Fatal("Could not locate start poly");
    }
    status = navMeshQuery->findNearestPoly(endf, halfexts, &filter, &endPoly, endpt);
    if (status & DT_FAILURE || status & DT_STATUS_DETAIL_MASK){
        Debug::Fatal("Could not locate end poly");
    }
    
    std::vector<dtPolyRef> polyPath(maxPoints);
    int nPathCount = 0;
    
    status = navMeshQuery->findPath(startPoly, endPoly, startf, endf, &filter, polyPath.data(), &nPathCount, maxPoints);
    if (status & DT_FAILURE || status & DT_STATUS_DETAIL_MASK){
        Debug::Fatal("Unable to create path");
    }
    std::vector<float> straightPath(nPathCount);
    int nVertCount = 0;
    status = navMeshQuery->findStraightPath(nearestpt, endpt, polyPath.data(), nPathCount, straightPath.data(), NULL, NULL, &nVertCount, maxPoints*3);
    
    // convert path to engine format
    std::vector<vector3> path(nVertCount/3);
    for (size_t i = 0; i < nVertCount/3; i++){
        path[i] = vector3(straightPath[i],straightPath[i+1],straightPath[i+2]);
    }
    return path;
}
