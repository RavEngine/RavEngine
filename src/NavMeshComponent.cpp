#include "NavMeshComponent.hpp"
#include "Debug.hpp"
#include <Recast.h>
#include <DetourNavMesh.h>
#include <DetourNavMeshBuilder.h>
#include <DetourCommon.h>
#include <DetourCrowd.h>
#include <DetourNavMeshQuery.h>
#include <DetourDebugDraw.h>
#include "App.hpp"
#include "MeshAsset.hpp"
#include "RenderEngine.hpp"

using namespace std;
using namespace RavEngine;

NavMeshComponent::NavMeshComponent(Ref<MeshAsset> mesh, Options opt){
    UpdateNavMesh(mesh, opt);
}

void NavMeshComponent::UpdateNavMesh(Ref<MeshAsset> mesh, Options opt){
    Debug::Assert(mesh->hasSystemRAMCopy(),"MeshAsset must be created with keepInSystemRAM = true");
    mtx.lock();
    // delete old values
    dtFree(navMesh);
    dtFree(navMeshQuery);
    dtFree(navData);
    
    auto& rawData = mesh->GetSystemCopy();
    bounds = mesh->GetBounds();
    
    const float* bmin = bounds.min;
    const float* bmax = bounds.max;
    
    const auto nverts = rawData.vertices.size();
    vector<float> vertsOnly(rawData.vertices.size() * 3);
    for(uint32_t i = 0; i < rawData.vertices.size(); i++){
        vertsOnly[i*3] = rawData.vertices[i].position[0];
        vertsOnly[i*3+1] = rawData.vertices[i].position[1];
        vertsOnly[i*3+2] = rawData.vertices[i].position[2];
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
    
    const int ntris = Debug::AssertSize<decltype(ntris)>(rawData.indices.size()/3);
    // allocate array to hold triangle area types ( = number of triangles)
    unsigned char* triareas = new unsigned char[ntris];
    std::memset(triareas, 0, ntris * sizeof(triareas[0]));

    const int* idxptr = nullptr;
    std::vector<int> convertedIndices;
    if (rawData.indices.mode == MeshAsset::BitWidth::uint32) {
        idxptr = reinterpret_cast<const int*>(rawData.indices.first_element_ptr());
    }
    else {
        convertedIndices.reserve(rawData.indices.size());
        for (int i = 0; i < rawData.indices.size(); i++) {
            convertedIndices.push_back(rawData.indices[i]);
        }
        idxptr = reinterpret_cast<const int*>(convertedIndices.data());
    }
    
    rcMarkWalkableTriangles(&ctx, cfg.walkableSlopeAngle, vertsOnly.data(), Debug::AssertSize<int>(nverts), idxptr, ntris, triareas);
    if(!rcRasterizeTriangles(&ctx, vertsOnly.data(), Debug::AssertSize<int>(vertsOnly.size()), idxptr, triareas, ntris, *solid)){
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
    // TODO: fix - for now, set all poly flags to 1 so that the filter includes them
    for(int i = 0; i < pmesh->npolys; i++){
        pmesh->flags[i] = 1;
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
        
        navMesh = dtAllocNavMesh();
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
    mtx.unlock();
}

NavMeshComponent::~NavMeshComponent(){
    dtFree(navMesh);
    dtFree(navMeshQuery);
    dtFree(navData);
}

RavEngine::Vector<vector3> NavMeshComponent::CalculatePath(const vector3 &start, const vector3 &end, uint16_t maxPoints){
    mtx.lock();
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
    //filter.setIncludeFlags(0xFFF);
    //filter.setExcludeFlags(0);
    //filter.setAreaCost(0, 1.0f);  // TODO: replace 0 with named region enum
    
    dtStatus status;
    dtPolyRef startPoly;
    dtPolyRef endPoly;
    
    status = navMeshQuery->findNearestPoly(startf, halfexts, &filter, &startPoly, nearestpt);
    if (dtStatusFailed(status) || startPoly == 0){
        Debug::Fatal("Could not locate start poly");
    }
    status = navMeshQuery->findNearestPoly(endf, halfexts, &filter, &endPoly, endpt);
    if (dtStatusFailed(status) || endPoly == 0){
        Debug::Fatal("Could not locate end poly");
    }
    
    std::vector<dtPolyRef> polyPath(maxPoints);
    int nPathCount = 0;
    
    status = navMeshQuery->findPath(startPoly, endPoly, nearestpt, endpt, &filter, polyPath.data(), &nPathCount, maxPoints);
    if (dtStatusFailed(status)){
        Debug::Fatal("Unable to create poly path");
    }
    std::vector<float> straightPath(maxPoints);
    int nVertCount = 0;
    status = navMeshQuery->findStraightPath(nearestpt, endpt, polyPath.data(), nPathCount, straightPath.data(), NULL, NULL, &nVertCount, maxPoints*3);
    if (dtStatusFailed(status)){
        Debug::Fatal("Unable to create path");
    }
    
    // convert path to engine format
    RavEngine::Vector<vector3> path(nVertCount);
    for (size_t i = 0; i < path.size(); i++) {
        path[i] = vector3(straightPath[i * 3],straightPath[i * 3 +1],straightPath[i * 3 +2]);
    }
    mtx.unlock();


    return path;
}

void RavEngine::NavMeshComponent::DebugDraw(RavEngine::DebugDrawer& dbg, const RavEngine::Transform& tr) const {
    mtx.lock();
    duDebugDrawNavMesh(&GetApp()->GetRenderEngine(), *navMesh, 0);
    mtx.unlock();
}
