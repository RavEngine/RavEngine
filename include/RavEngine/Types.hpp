#pragma once
#include <cstdint>
#include <limits>
#include <chrono>
#include "cluster_defs.h"

using entity_id_t = uint32_t;
constexpr entity_id_t INVALID_ENTITY = std::numeric_limits<decltype(INVALID_ENTITY)>::max();

struct EntityHandle{
    entity_id_t id : 24 = INVALID_ENTITY;
    entity_id_t version : 8 = 0;            // MSVC makes the struct the wrong size if we don't do this
    
    bool operator==(const EntityHandle& other) const{
        return id == other.id && version == other.version;
    }
};

static_assert(sizeof(EntityHandle) == sizeof(entity_id_t), "EntityHandle is the wrong size!");

using entity_t = EntityHandle;
using pos_t = uint32_t;
constexpr pos_t INVALID_INDEX = std::numeric_limits<decltype(INVALID_INDEX)>::max();

static constexpr inline bool EntityIsValid(entity_id_t id){
    return id != INVALID_ENTITY;
}


static constexpr inline bool EntityIsValid(entity_t id){
    return EntityIsValid(id.id);
}

static constexpr inline bool PosIsValid(pos_t id){
    return id != INVALID_INDEX;
}
 
using e_clock_t = std::chrono::steady_clock;

constexpr uint8_t MAX_CASCADES = SH_MAX_CASCADES;

#define MOVE_NO_COPY(T)  T(const T&) = delete; T(T&&) = default; T& operator=(T&&) = default;
