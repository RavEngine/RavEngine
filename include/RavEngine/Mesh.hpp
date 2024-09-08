#pragma once
#include <vector>
#include "Vector.hpp"
#include <glm/mat4x4.hpp>
#include <vector>
#include "Common3D.hpp"

namespace RavEngine{


struct SerializedMeshDataHeader{
    const std::array<char, 4> header = {'r','v','e','m'};
    uint32_t numVertices = 0;
    uint32_t numIndicies = 0;
    uint8_t attributes = 0;     // info about the file

    constexpr static uint8_t SkinnedMeshBit = 1 << 0;
};

typedef VertexNormalUV vertex_t;

template<template<typename...> class T>
struct MeshPartBase{
    T<uint32_t> indices;
    T<vertex_t> vertices;
};
struct MeshPart : public MeshPartBase<Vector>{};

template<typename T>
struct basic_immutable_span : public std::span<const T,std::dynamic_extent>{
    basic_immutable_span(){}
    basic_immutable_span(const T* ptr, size_t count) : std::span<const T,std::dynamic_extent>(ptr,count){}
};

struct MeshPartView : public MeshPartBase<basic_immutable_span>{
    MeshPartView(){}
    MeshPartView(const MeshPart& other){
        vertices = { other.vertices.data(),other.vertices.size() };
        indices = { other.indices.data(), other.indices.size() };
    }
};


}
