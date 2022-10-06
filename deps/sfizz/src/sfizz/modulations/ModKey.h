// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "ModKeyHash.h"
#include "ModId.h"
#include "../utility/NumericId.h"
#include <string>
#include <cstring>

namespace sfz {

struct Region;

enum class ModId : int;

/**
 * @brief Identifier of a single modulation source or target within a SFZ instrument
 */
class ModKey {
public:
    struct Parameters;

    ModKey() = default;
    explicit ModKey(ModId id, NumericId<Region> region = {}, Parameters params = {})
        : id_(id), region_(region), params_(params), flags_(ModIds::flags(id_)) {}

    static ModKey createCC(uint16_t cc, uint8_t curve, uint16_t smooth, float step);
    static ModKey createNXYZ(ModId id, NumericId<Region> region = {}, uint8_t N = 0, uint8_t X = 0, uint8_t Y = 0, uint8_t Z = 0);

    explicit operator bool() const noexcept { return id_ != ModId(); }

    const ModId& id() const noexcept { return id_; }
    NumericId<Region> region() const noexcept { return region_; }
    const Parameters& parameters() const noexcept { return params_; }
    int flags() const noexcept { return flags_; }

    bool isSource() const noexcept;
    bool isTarget() const noexcept;
    std::string toString() const;

    /**
     * @brief Obtain the modulation key of the source depth, in the connection
     * between source and target, if such a key exists.
     */
    static ModKey getSourceDepthKey(ModKey source, ModKey target);

    struct RawParameters {
        union {
            //! Parameters if this key identifies a CC source
            struct { uint16_t cc; uint8_t curve; uint16_t smooth; float step; };
            //! Parameters otherwise, based on the related opcode
            // eg. `N` in `lfoN`, `N, X` in `lfoN_eqX`
            struct { uint8_t N, X, Y, Z; };
            // !!! NOTE: NXYZ is expected to be stored in 0-indexed form
            //           eg. `lfo1_eq2` is N=0, X=1
        };
    };

    struct Parameters : RawParameters {
        Parameters() noexcept;
        Parameters(const Parameters& other) noexcept;
        Parameters& operator=(const Parameters& other) noexcept;

        Parameters(Parameters&&) noexcept;
        Parameters &operator=(Parameters&&) noexcept;

        bool operator==(const Parameters& other) const noexcept
        {
            return std::memcmp(
                static_cast<const RawParameters*>(this),
                static_cast<const RawParameters*>(&other),
                sizeof(RawParameters)) == 0;
        }

        bool operator!=(const Parameters& other) const noexcept
        {
            return !operator==(other);
        }
    };

public:
    bool operator==(const ModKey &other) const noexcept
    {
        return id_ == other.id_ && region_ == other.region_ &&
            parameters() == other.parameters();
    }

    bool operator!=(const ModKey &other) const noexcept
    {
        return !this->operator==(other);
    }


private:
    //! Identifier
    ModId id_ {};
    //! Region identifier, only applicable if the modulation is per-voice
    NumericId<Region> region_;
    //! List of values which identify the key uniquely, along with the hash and region
    Parameters params_ {};
    // Memorize the flag
    int flags_;
};

} // namespace sfz
