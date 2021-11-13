#pragma once
#include <cstdint>
#include <limits>
#include <chrono>

using entity_t = uint32_t;
using pos_t = uint32_t;
constexpr entity_t INVALID_ENTITY = std::numeric_limits<decltype(INVALID_ENTITY)>::max();
constexpr pos_t INVALID_INDEX = std::numeric_limits<decltype(INVALID_INDEX)>::max();

static constexpr inline bool EntityIsValid(entity_t id){
    return id != INVALID_ENTITY;
}

static constexpr inline bool PosIsValid(pos_t id){
    return id != INVALID_INDEX;
}
 
using e_clock_t = std::chrono::steady_clock;
