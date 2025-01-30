#pragma once
#include "Vector.hpp"
#include <glm/mat4x4.hpp>
#include "Common3D.hpp"
#include <span>

namespace RavEngine{

    using VertexAttrib_t = uint8_t;

struct SerializedMeshDataHeader{
    const std::array<char, 4> header = {'r','v','e','m'};
    uint32_t numVertices = 0;
    uint32_t numIndicies = 0;
    VertexAttrib_t attributes = 0;     // info about the file

    constexpr static VertexAttrib_t 
        SkinnedMeshBit = 1 << 0,
        hasPositionsBit = 1 << 1,
        hasNormalsBit = 1 << 2,
        hasTangentsBit = 1 << 3,
        hasBitangentsBit = 1 << 4,
        hasUV0Bit = 1 << 5,
        hasLightmapUVBit = 1 << 6
        ;
};

typedef VertexNormalUV vertex_t;

using VertexPosition_t = decltype(VertexNormalUV::position);
using VertexNormal_t = decltype(VertexNormalUV::normal);
using VertexTangent_t = decltype(VertexNormalUV::tangent);
using VertexBitangent_t = decltype(VertexNormalUV::bitangent);
using VertexUV_t = decltype(VertexNormalUV::uv);

template<template<typename...> class T>
struct MeshPartBase{
    T<uint32_t> indices;
    T<VertexPosition_t> positions;
    T<VertexNormal_t> normals;
    T<VertexTangent_t> tangents;
    T<VertexBitangent_t> bitangents;
    T<VertexUV_t> uv0;
    T<VertexUV_t> lightmapUVs;

    uint32_t NumVerts() const {
        return positions.size();
    }
};
struct MeshPart : public MeshPartBase<Vector>{
    void ReserveVerts(uint32_t size) {
        positions.reserve(size);
        normals.reserve(size);
        tangents.reserve(size);
        bitangents.reserve(size);
        uv0.reserve(size);
        lightmapUVs.reserve(size);
    }
};

template<typename T>
struct basic_immutable_span : public std::span<const T,std::dynamic_extent>{
    basic_immutable_span(){}
    basic_immutable_span(const T* ptr, size_t count) : std::span<const T,std::dynamic_extent>(ptr,count){}
};

struct MeshPartView : public MeshPartBase<basic_immutable_span>{
    MeshPartView(){}
    MeshPartView(const MeshPart& other){
        positions = { other.positions.data(),other.positions.size() };
        normals = { other.normals.data(),other.normals.size() };
        tangents = { other.tangents.data(),other.tangents.size() };
        bitangents = { other.bitangents.data(),other.bitangents.size() };
        uv0 = { other.uv0.data(),other.uv0.size() };
        lightmapUVs = { other.lightmapUVs.data(),other.lightmapUVs.size() };
        indices = { other.indices.data(), other.indices.size() };
    }
};


}
