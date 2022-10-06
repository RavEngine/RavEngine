// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "SynthPrivate.h"
#include "FilePool.h"
#include "Curve.h"
#include "MidiState.h"
#include "SynthConfig.h"
#include "utility/StringViewHelpers.h"
#include <absl/strings/ascii.h>
#include <cstring>

// TODO: `ccModDepth` and `ccModParameters` are O(N), need better implementation

namespace sfz {
static constexpr unsigned maxIndices = 8;

static bool extractMessage(const char* pattern, const char* path, unsigned* indices);
static uint64_t hashMessagePath(const char* path, const char* sig);

void sfz::Synth::dispatchMessage(Client& client, int delay, const char* path, const char* sig, const sfizz_arg_t* args)
{
    UNUSED(args);
    Impl& impl = *impl_;
    unsigned indices[maxIndices];

    switch (hashMessagePath(path, sig)) {
        #define MATCH(p, s) case hash(p "," s): \
            if (extractMessage(p, path, indices) && !strcmp(sig, s))

        #define GET_REGION_OR_BREAK(idx)            \
            if (idx >= impl.layers_.size())         \
                break;                              \
            Layer& layer = *impl.layers_[idx];      \
            const Region& region = layer.getRegion();

        #define GET_FILTER_OR_BREAK(idx)                \
            if (idx >= region.filters.size())           \
                break;                                  \
            const auto& filter = region.filters[idx];

        #define GET_EQ_OR_BREAK(idx)                    \
            if (idx >= region.equalizers.size())        \
                break;                                  \
            const auto& eq = region.equalizers[idx];

        #define GET_LFO_OR_BREAK(idx)             \
            if (idx >= region.lfos.size())        \
                break;                            \
            const auto& lfo = region.lfos[idx];

        #define GET_EG_OR_BREAK(idx)              \
            if (idx >= region.flexEGs.size())     \
                break;                            \
            auto& eg = region.flexEGs[idx];

        #define GET_EG_POINT_OR_BREAK(idx)        \
            if (idx >= eg.points.size())          \
                break;                            \
            auto& point = eg.points[idx];

        MATCH("/hello", "") {
            client.receive(delay, "/hello", "", nullptr);
        } break;

        //----------------------------------------------------------------------

        MATCH("/num_regions", "") {
            client.receive<'i'>(delay, path, int(impl.layers_.size()));
        } break;

        MATCH("/num_groups", "") {
            client.receive<'i'>(delay, path, impl.numGroups_);
        } break;

        MATCH("/num_masters", "") {
            client.receive<'i'>(delay, path, impl.numMasters_);
        } break;

        MATCH("/num_curves", "") {
            client.receive<'i'>(delay, path, int(impl.resources_.getCurves().getNumCurves()));
        } break;

        MATCH("/num_samples", "") {
            client.receive<'i'>(delay, path, int(impl.resources_.getFilePool().getNumPreloadedSamples()));
        } break;

        MATCH("/octave_offset", "") {
            client.receive<'i'>(delay, path, impl.octaveOffset_);
        } break;

        MATCH("/note_offset", "") {
            client.receive<'i'>(delay, path, impl.noteOffset_);
        } break;

        MATCH("/num_outputs", "") {
            client.receive<'i'>(delay, path, impl.numOutputs_);
        } break;

        //----------------------------------------------------------------------

        MATCH("/key/slots", "") {
            const BitArray<128>& keys = impl.keySlots_;
            sfizz_blob_t blob { keys.data(), static_cast<uint32_t>(keys.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/key&/label", "") {
            if (indices[0] >= 128)
                break;
            const std::string* label = impl.getKeyLabel(indices[0]);
            client.receive<'s'>(delay, path, label ? label->c_str() : "");
        } break;

        //----------------------------------------------------------------------

        MATCH("/root_path", "") {
            client.receive<'s'>(delay, path, impl.rootPath_.c_str());
        } break;

        MATCH("/image", "") {
            client.receive<'s'>(delay, path, impl.image_.c_str());
        } break;

        //----------------------------------------------------------------------

        MATCH("/sw/last/slots", "") {
            const BitArray<128>& switches = impl.swLastSlots_;
            sfizz_blob_t blob { switches.data(), static_cast<uint32_t>(switches.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/sw/last/current", "") {
            if (impl.currentSwitch_)
                client.receive<'i'>(delay, path, *impl.currentSwitch_);
            else
                client.receive<'N'>(delay, path, {});
        } break;

        MATCH("/sw/last/&/label", "") {
            if (indices[0] >= 128)
                break;
            const std::string* label = impl.getKeyswitchLabel(indices[0]);
            client.receive<'s'>(delay, path, label ? label->c_str() : "");
        } break;

        //----------------------------------------------------------------------

        MATCH("/cc/slots", "") {
            const BitArray<config::numCCs>& ccs = impl.currentUsedCCs_;
            sfizz_blob_t blob { ccs.data(), static_cast<uint32_t>(ccs.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/cc&/default", "") {
            if (indices[0] >= config::numCCs)
                break;
            client.receive<'f'>(delay, path, impl.defaultCCValues_[indices[0]]);
        } break;

        MATCH("/cc&/value", "") {
            if (indices[0] >= config::numCCs)
                break;
            // Note: result value is not frame-exact
            client.receive<'f'>(delay, path, impl.resources_.getMidiState().getCCValue(indices[0]));
        } break;

        MATCH("/cc&/value", "f") {
            if (indices[0] >= config::numCCs)
                break;
            impl.resources_.getMidiState().ccEvent(delay, indices[0], args[0].f);
        } break;

        MATCH("/cc&/label", "") {
            if (indices[0] >= config::numCCs)
                break;
            const std::string* label = impl.getCCLabel(indices[0]);
            client.receive<'s'>(delay, path, label ? label->c_str() : "");
        } break;

        MATCH("/cc/changed", "") {
            const BitArray<config::numCCs>& changedCCs = impl.changedCCsThisCycle_;
            sfizz_blob_t blob { changedCCs.data(), static_cast<uint32_t>(changedCCs.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/cc/changed~", "") {
            const BitArray<config::numCCs>& changedCCs = impl.changedCCsLastCycle_;
            sfizz_blob_t blob { changedCCs.data(), static_cast<uint32_t>(changedCCs.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/sustain_or_sostenuto/slots", "") {
            const BitArray<128>& sustainOrSostenuto = impl.sustainOrSostenuto_;
            sfizz_blob_t blob { sustainOrSostenuto.data(),
                static_cast<uint32_t>(sustainOrSostenuto.byte_size()) };
            client.receive<'b'>(delay, path, &blob);
        } break;

        MATCH("/aftertouch", "") {
            client.receive<'f'>(delay, path, impl.resources_.getMidiState().getChannelAftertouch());
        } break;

        MATCH("/poly_aftertouch/&", "") {
            if (indices[0] > 127)
                break;
            // Note: result value is not frame-exact
            client.receive<'f'>(delay, path, impl.resources_.getMidiState().getPolyAftertouch(indices[0]));
        } break;

        MATCH("/pitch_bend", "") {
            // Note: result value is not frame-exact
            client.receive<'f'>(delay, path, impl.resources_.getMidiState().getPitchBend());
        } break;

        //----------------------------------------------------------------------

        MATCH("/mem/buffers", "") {
            uint64_t total = BufferCounter::counter().getTotalBytes();
            client.receive<'h'>(delay, path, total);
        } break;

        //----------------------------------------------------------------------

        MATCH("/region&/delay", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.delay);
        } break;

        MATCH("/region&/sample", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'s'>(delay, path, region.sampleId->filename().c_str());
        } break;

        MATCH("/region&/direction", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.sampleId->isReverse())
                client.receive<'s'>(delay, path, "reverse");
            else
                client.receive<'s'>(delay, path, "forward");
        } break;

        MATCH("/region&/delay_random", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.delayRandom);
        } break;

        MATCH("/region&/delay_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.delayCC.getWithDefault(indices[1]));
        } break;

        MATCH("/region&/offset", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.offset);
        } break;

        MATCH("/region&/offset_random", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.offsetRandom);
        } break;

        MATCH("/region&/offset_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.offsetCC.getWithDefault(indices[1]));
        } break;

        MATCH("/region&/end", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.sampleEnd);
        } break;

        MATCH("/region&/end_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.endCC.getWithDefault(indices[1]));
        } break;

        MATCH("/region&/enabled", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.disabled()) {
                client.receive<'F'>(delay, path, {});
            } else {
                client.receive<'T'>(delay, path, {});
            }
        } break;

        MATCH("/region&/trigger_on_note", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.triggerOnNote) {
                client.receive<'T'>(delay, path, {});
            } else {
                client.receive<'F'>(delay, path, {});
            }
        } break;

        MATCH("/region&/trigger_on_cc", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.triggerOnCC) {
                client.receive<'T'>(delay, path, {});
            } else {
                client.receive<'F'>(delay, path, {});
            }
        } break;

        MATCH("/region&/count", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.sampleCount)
                client.receive<'h'>(delay, path, *region.sampleCount);
            else
                client.receive<'N'>(delay, path, {});
        } break;

        MATCH("/region&/loop_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].h = region.loopRange.getStart();
            args[1].h = region.loopRange.getEnd();
            client.receive(delay, path, "hh", args);
        } break;

        MATCH("/region&/loop_start_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.loopStartCC.getWithDefault(indices[1]));
        } break;

        MATCH("/region&/loop_end_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.loopEndCC.getWithDefault(indices[1]));
        } break;

        MATCH("/region&/loop_mode", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (!region.loopMode) {
                client.receive<'s'>(delay, path, "no_loop");
                break;
            }

            switch (*region.loopMode) {
            case LoopMode::no_loop:
                client.receive<'s'>(delay, path, "no_loop");
                break;
            case LoopMode::loop_continuous:
                client.receive<'s'>(delay, path, "loop_continuous");
                break;
            case LoopMode::loop_sustain:
                client.receive<'s'>(delay, path, "loop_sustain");
                break;
            case LoopMode::one_shot:
                client.receive<'s'>(delay, path, "one_shot");
                break;
            }
        } break;

        MATCH("/region&/loop_crossfade", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.loopCrossfade);
        } break;

        MATCH("/region&/loop_count", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.loopCount)
                client.receive<'h'>(delay, path, *region.loopCount);
            else
                client.receive<'N'>(delay, path, {});
        } break;

        MATCH("/region&/output", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.output);
        } break;

        MATCH("/region&/group", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.group);
        } break;

        MATCH("/region&/off_by", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (!region.offBy) {
                client.receive<'N'>(delay, path, {});
            } else {
                client.receive<'h'>(delay, path, *region.offBy);
            }
        } break;

        MATCH("/region&/off_mode", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.offMode) {
            case OffMode::time:
                client.receive<'s'>(delay, path, "time");
                break;
            case OffMode::normal:
                client.receive<'s'>(delay, path, "normal");
                break;
            case OffMode::fast:
                client.receive<'s'>(delay, path, "fast");
                break;
            }
        } break;

        MATCH("/region&/key_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].i = region.keyRange.getStart();
            args[1].i = region.keyRange.getEnd();
            client.receive(delay, path, "ii", args);
        } break;

        MATCH("/region&/off_time", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.offTime);
        } break;

        MATCH("/region&/pitch_keycenter", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.pitchKeycenter);
        } break;

        MATCH("/region&/vel_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.velocityRange.getStart();
            args[1].f = region.velocityRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/bend_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.bendRange.getStart();
            args[1].f = region.bendRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/program_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].i = region.programRange.getStart();
            args[1].i = region.programRange.getEnd();
            client.receive(delay, path, "ii", args);
        } break;

        MATCH("/region&/cc_range&", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            const auto& conditions = region.ccConditions.getWithDefault(indices[1]);
            args[0].f = conditions.getStart();
            args[1].f = conditions.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/sw_last", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.lastKeyswitch) {
                client.receive<'i'>(delay, path, *region.lastKeyswitch);
            } else if (region.lastKeyswitchRange) {
                sfizz_arg_t args[2];
                args[0].i = region.lastKeyswitchRange->getStart();
                args[1].i = region.lastKeyswitchRange->getEnd();
                client.receive(delay, path, "ii", args);
            } else {
                client.receive<'N'>(delay, path, {});
            }

        } break;

        MATCH("/region&/sw_label", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.keyswitchLabel) {
                client.receive<'s'>(delay, path, region.keyswitchLabel->c_str());
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/sw_up", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.upKeyswitch) {
                client.receive<'i'>(delay, path, *region.upKeyswitch);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/sw_down", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.downKeyswitch) {
                client.receive<'i'>(delay, path, *region.downKeyswitch);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/sw_previous", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.previousKeyswitch) {
                client.receive<'i'>(delay, path, *region.previousKeyswitch);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/sw_vel", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.velocityOverride) {
            case VelocityOverride::current:
                client.receive<'s'>(delay, path, "current");
                break;
            case VelocityOverride::previous:
                client.receive<'s'>(delay, path, "previous");
                break;
            }
        } break;

        MATCH("/region&/chanaft_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.aftertouchRange.getStart();
            args[1].f = region.aftertouchRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/polyaft_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.polyAftertouchRange.getStart();
            args[1].f = region.polyAftertouchRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/bpm_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.bpmRange.getStart();
            args[1].f = region.bpmRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/rand_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.randRange.getStart();
            args[1].f = region.randRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/seq_length", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.sequenceLength);
        } break;

        MATCH("/region&/seq_position", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'h'>(delay, path, region.sequencePosition);
        } break;

        MATCH("/region&/trigger", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.trigger) {
            case Trigger::attack:
                client.receive<'s'>(delay, path, "attack");
                break;
            case Trigger::first:
                client.receive<'s'>(delay, path, "first");
                break;
            case Trigger::release:
                client.receive<'s'>(delay, path, "release");
                break;
            case Trigger::release_key:
                client.receive<'s'>(delay, path, "release_key");
                break;
            case Trigger::legato:
                client.receive<'s'>(delay, path, "legato");
                break;
            }
        } break;

        MATCH("/region&/start_cc_range&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto trigger = region.ccTriggers.get(indices[1]);
            if (trigger) {
                sfizz_arg_t args[2];
                args[0].f = trigger->getStart();
                args[1].f = trigger->getEnd();
                client.receive(delay, path, "ff", args);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/volume", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.volume);
        } break;

        MATCH("/region&/volume_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Volume);
            if (value) {
                client.receive<'f'>(delay, path, *value);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/volume_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Volume);
            if (params) {
                client.receive<'f'>(delay, path, params->step);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/volume_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Volume);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/volume_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Volume);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pan", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.pan * 100.0f);
        } break;

        MATCH("/region&/pan_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Pan);
            if (value) {
                client.receive<'f'>(delay, path, *value * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pan_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pan);
            if (params) {
                client.receive<'f'>(delay, path, params->step * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pan_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pan);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pan_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pan);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/width", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.width * 100.0f);
        } break;

        MATCH("/region&/width_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Width);
            if (value) {
                client.receive<'f'>(delay, path, *value * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/width_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Width);
            if (params) {
                client.receive<'f'>(delay, path, params->step * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/width_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Width);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/width_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Width);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/position", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.position * 100.0f);
        } break;

        MATCH("/region&/position_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Position);
            if (value) {
                client.receive<'f'>(delay, path, *value * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/position_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Position);
            if (params) {
                client.receive<'f'>(delay, path, params->step * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/position_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Position);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/position_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Position);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amplitude", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitude * 100.0f);
        } break;

        MATCH("/region&/amplitude_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Amplitude);
            if (value) {
                client.receive<'f'>(delay, path, *value * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amplitude_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Amplitude);
            if (params) {
                client.receive<'f'>(delay, path, params->step * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amplitude_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Amplitude);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amplitude_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Amplitude);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amp_keycenter", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.ampKeycenter);
        } break;

        MATCH("/region&/amp_keytrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.ampKeytrack);
        } break;

        MATCH("/region&/amp_veltrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.ampVeltrack * 100.0f);
        } break;

        MATCH("/region&/amp_veltrack_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.ampVeltrackCC.contains(indices[1])) {
                const auto& cc = region.ampVeltrackCC.getWithDefault(indices[1]);
                client.receive<'f'>(delay, path, cc.modifier * 100.0f);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amp_veltrack_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.ampVeltrackCC.contains(indices[1])) {
                const auto& cc = region.ampVeltrackCC.getWithDefault(indices[1]);
                client.receive<'i'>(delay, path, cc.curve );
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/amp_random", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.ampRandom);
        } break;

        MATCH("/region&/xfin_key_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].i = region.crossfadeKeyInRange.getStart();
            args[1].i = region.crossfadeKeyInRange.getEnd();
            client.receive(delay, path, "ii", args);
        } break;

        MATCH("/region&/xfout_key_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].i = region.crossfadeKeyOutRange.getStart();
            args[1].i = region.crossfadeKeyOutRange.getEnd();
            client.receive(delay, path, "ii", args);
        } break;

        MATCH("/region&/xfin_vel_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.crossfadeVelInRange.getStart();
            args[1].f = region.crossfadeVelInRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/xfout_vel_range", "") {
            GET_REGION_OR_BREAK(indices[0])
            sfizz_arg_t args[2];
            args[0].f = region.crossfadeVelOutRange.getStart();
            args[1].f = region.crossfadeVelOutRange.getEnd();
            client.receive(delay, path, "ff", args);
        } break;

        MATCH("/region&/xfin_cc_range&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto range = region.crossfadeCCInRange.get(indices[1]);
            if (range) {
                sfizz_arg_t args[2];
                args[0].f = range->getStart();
                args[1].f = range->getEnd();
                client.receive(delay, path, "ff", args);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/xfout_cc_range&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto range = region.crossfadeCCOutRange.get(indices[1]);
            if (range) {
                sfizz_arg_t args[2];
                args[0].f = range->getStart();
                args[1].f = range->getEnd();
                client.receive(delay, path, "ff", args);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/xf_keycurve", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.crossfadeKeyCurve) {
            case CrossfadeCurve::gain:
                client.receive<'s'>(delay, path, "gain");
                break;
            case CrossfadeCurve::power:
                client.receive<'s'>(delay, path, "power");
                break;
            }
        } break;

        MATCH("/region&/xf_velcurve", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.crossfadeVelCurve) {
            case CrossfadeCurve::gain:
                client.receive<'s'>(delay, path, "gain");
                break;
            case CrossfadeCurve::power:
                client.receive<'s'>(delay, path, "power");
                break;
            }
        } break;

        MATCH("/region&/xf_cccurve", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch (region.crossfadeCCCurve) {
            case CrossfadeCurve::gain:
                client.receive<'s'>(delay, path, "gain");
                break;
            case CrossfadeCurve::power:
                client.receive<'s'>(delay, path, "power");
                break;
            }
        } break;

        MATCH("/region&/global_volume", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.globalVolume);
        } break;

        MATCH("/region&/master_volume", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.masterVolume);
        } break;

        MATCH("/region&/group_volume", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.groupVolume);
        } break;

        MATCH("/region&/global_amplitude", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.globalAmplitude * 100.0f);
        } break;

        MATCH("/region&/master_amplitude", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.masterAmplitude * 100.0f);
        } break;

        MATCH("/region&/group_amplitude", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.groupAmplitude * 100.0f);
        } break;

        MATCH("/region&/pitch_keytrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.pitchKeytrack);
        } break;

        MATCH("/region&/pitch_veltrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.pitchVeltrack);
        } break;

        MATCH("/region&/pitch_veltrack_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.pitchVeltrackCC.contains(indices[1])) {
                const auto& cc = region.pitchVeltrackCC.getWithDefault(indices[1]);
                client.receive<'f'>(delay, path, cc.modifier);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitch_veltrack_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.pitchVeltrackCC.contains(indices[1])) {
                const auto& cc = region.pitchVeltrackCC.getWithDefault(indices[1]);
                client.receive<'i'>(delay, path, cc.curve );
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitch_random", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.pitchRandom);
        } break;

        MATCH("/region&/transpose", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.transpose);
        } break;

        MATCH("/region&/pitch", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.pitch);
        } break;

        MATCH("/region&/pitch_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto value = region.ccModDepth(indices[1], ModId::Pitch);
            if (value) {
                client.receive<'f'>(delay, path, *value);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitch_stepcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pitch);
            if (params) {
                client.receive<'f'>(delay, path, params->step);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitch_smoothcc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pitch);
            if (params) {
                client.receive<'i'>(delay, path, params->smooth);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitch_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto params = region.ccModParameters(indices[1], ModId::Pitch);
            if (params) {
                client.receive<'i'>(delay, path, params->curve);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/bend_up", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.bendUp);
        } break;

        MATCH("/region&/bend_down", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.bendDown);
        } break;

        MATCH("/region&/bend_step", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.bendStep);
        } break;

        MATCH("/region&/bend_smooth", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.bendSmooth);
        } break;

        MATCH("/region&/ampeg_attack", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.attack);
        } break;

        MATCH("/region&/ampeg_delay", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.delay);
        } break;

        MATCH("/region&/ampeg_decay", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.decay);
        } break;

        MATCH("/region&/ampeg_hold", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.hold);
        } break;

        MATCH("/region&/ampeg_release", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.release);
        } break;

        MATCH("/region&/ampeg_start", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.start * 100.0f);
        } break;

        MATCH("/region&/ampeg_sustain", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.sustain * 100.0f);
        } break;

        MATCH("/region&/ampeg_depth", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.amplitudeEG.depth);
        } break;

        MATCH("/region&/ampeg_vel&attack", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2attack);
        } break;

        MATCH("/region&/ampeg_vel&delay", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2delay);
        } break;

        MATCH("/region&/ampeg_vel&decay", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2decay);
        } break;

        MATCH("/region&/ampeg_vel&hold", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2hold);
        } break;

        MATCH("/region&/ampeg_vel&release", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2release);
        } break;

        MATCH("/region&/ampeg_vel&sustain", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2sustain * 100.0f);
        } break;

        MATCH("/region&/ampeg_vel&depth", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (indices[1] != 2)
                break;
            client.receive<'f'>(delay, path, region.amplitudeEG.vel2depth);
        } break;

        MATCH("/region&/ampeg_dynamic", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.amplitudeEG.dynamic) {
                client.receive<'T'>(delay, path, {});
            } else {
                client.receive<'F'>(delay, path, {});
            }
        } break;

        MATCH("/region&/fileg_dynamic", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.filterEG && region.filterEG->dynamic) {
                client.receive<'T'>(delay, path, {});
            } else {
                client.receive<'F'>(delay, path, {});
            }
        } break;

        MATCH("/region&/pitcheg_dynamic", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.pitchEG && region.pitchEG->dynamic) {
                client.receive<'T'>(delay, path, {});
            } else {
                client.receive<'F'>(delay, path, {});
            }
        } break;

        MATCH("/region&/note_polyphony", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.notePolyphony) {
                client.receive<'i'>(delay, path, *region.notePolyphony);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/note_selfmask", "") {
            GET_REGION_OR_BREAK(indices[0])
            switch(region.selfMask) {
            case SelfMask::mask:
                client.receive(delay, path, "T", nullptr);
                break;
            case SelfMask::dontMask:
                client.receive(delay, path, "F", nullptr);
                break;
            }
        } break;

        MATCH("/region&/rt_dead", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.rtDead) {
                client.receive(delay, path, "T", nullptr);
            } else {
                client.receive(delay, path, "F", nullptr);
            }
        } break;

        MATCH("/region&/sustain_sw", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.checkSustain) {
                client.receive(delay, path, "T", nullptr);
            } else {
                client.receive(delay, path, "F", nullptr);
            }
        } break;

        MATCH("/region&/sostenuto_sw", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.checkSostenuto) {
                client.receive(delay, path, "T", nullptr);
            } else {
                client.receive(delay, path, "F", nullptr);
            }
        } break;

        MATCH("/region&/sustain_cc", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.sustainCC);
        } break;

        MATCH("/region&/sostenuto_cc", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.sostenutoCC);
        } break;

        MATCH("/region&/sustain_lo", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.sustainThreshold);
        } break;

        MATCH("/region&/sostenuto_lo", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.sostenutoThreshold);
        } break;

        MATCH("/region&/oscillator_phase", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.oscillatorPhase);
        } break;

        MATCH("/region&/oscillator_quality", "") {
            GET_REGION_OR_BREAK(indices[0])
            if (region.oscillatorQuality) {
                client.receive<'i'>(delay, path, *region.oscillatorQuality);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/oscillator_mode", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.oscillatorMode);
        } break;

        MATCH("/region&/oscillator_multi", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, region.oscillatorMulti);
        } break;

        MATCH("/region&/oscillator_detune", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.oscillatorDetune);
        } break;

        MATCH("/region&/oscillator_mod_depth", "") {
            GET_REGION_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, region.oscillatorModDepth * 100.0f);
        } break;

        // TODO: detune cc, mod depth cc

        MATCH("/region&/effect&", "") {
            GET_REGION_OR_BREAK(indices[0])
            auto effectIdx = indices[1];
            if (indices[1] == 0)
                break;

            if (effectIdx < region.gainToEffect.size())
                client.receive<'f'>(delay, path, region.gainToEffect[effectIdx] * 100.0f);
        } break;

        MATCH("/region&/ampeg_attack_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccAttack.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value);
        } break;

        MATCH("/region&/ampeg_decay_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccDecay.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value);
        } break;

        MATCH("/region&/ampeg_delay_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccDelay.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value);
        } break;

        MATCH("/region&/ampeg_hold_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccHold.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value);
        } break;

        MATCH("/region&/ampeg_release_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccRelease.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value);
        } break;

        MATCH("/region&/ampeg_start_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccStart.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value * 100.0f);
        } break;

        MATCH("/region&/ampeg_sustain_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            float value = region.amplitudeEG.ccSustain.getWithDefault(indices[1]);
            client.receive<'f'>(delay, path, value * 100.0f);
        } break;

        MATCH("/region&/filter&/cutoff", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, filter.cutoff);
        } break;

        MATCH("/region&/filter&/resonance", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, filter.resonance);
        } break;

        MATCH("/region&/filter&/gain", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, filter.gain);
        } break;

        MATCH("/region&/filter&/keycenter", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'i'>(delay, path, filter.keycenter);
        } break;

        MATCH("/region&/filter&/keytrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'i'>(delay, path, filter.keytrack);
        } break;

        MATCH("/region&/filter&/veltrack", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            client.receive<'i'>(delay, path, filter.veltrack);
        } break;

        MATCH("/region&/filter&/veltrack_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            if (filter.veltrackCC.contains(indices[2])) {
                const auto& cc = filter.veltrackCC.getWithDefault(indices[2]);
                client.receive<'f'>(delay, path, cc.modifier);
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/filter&/veltrack_curvecc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            if (filter.veltrackCC.contains(indices[2])) {
                const auto& cc = filter.veltrackCC.getWithDefault(indices[2]);
                client.receive<'i'>(delay, path, cc.curve );
            } else {
                client.receive<'N'>(delay, path, {});
            }
        } break;

        MATCH("/region&/filter&/type", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            switch (filter.type) {
            case FilterType::kFilterLpf1p: client.receive<'s'>(delay, path, "lpf_1p"); break;
            case FilterType::kFilterHpf1p: client.receive<'s'>(delay, path, "hpf_1p"); break;
            case FilterType::kFilterLpf2p: client.receive<'s'>(delay, path, "lpf_2p"); break;
            case FilterType::kFilterHpf2p: client.receive<'s'>(delay, path, "hpf_2p"); break;
            case FilterType::kFilterBpf2p: client.receive<'s'>(delay, path, "bpf_2p"); break;
            case FilterType::kFilterBrf2p: client.receive<'s'>(delay, path, "brf_2p"); break;
            case FilterType::kFilterBpf1p: client.receive<'s'>(delay, path, "bpf_1p"); break;
            case FilterType::kFilterBrf1p: client.receive<'s'>(delay, path, "brf_1p"); break;
            case FilterType::kFilterApf1p: client.receive<'s'>(delay, path, "apf_1p"); break;
            case FilterType::kFilterLpf2pSv: client.receive<'s'>(delay, path, "lpf_2p_sv"); break;
            case FilterType::kFilterHpf2pSv: client.receive<'s'>(delay, path, "hpf_2p_sv"); break;
            case FilterType::kFilterBpf2pSv: client.receive<'s'>(delay, path, "bpf_2p_sv"); break;
            case FilterType::kFilterBrf2pSv: client.receive<'s'>(delay, path, "brf_2p_sv"); break;
            case FilterType::kFilterLpf4p: client.receive<'s'>(delay, path, "lpf_4p"); break;
            case FilterType::kFilterHpf4p: client.receive<'s'>(delay, path, "hpf_4p"); break;
            case FilterType::kFilterLpf6p: client.receive<'s'>(delay, path, "lpf_6p"); break;
            case FilterType::kFilterHpf6p: client.receive<'s'>(delay, path, "hpf_6p"); break;
            case FilterType::kFilterPink: client.receive<'s'>(delay, path, "pink"); break;
            case FilterType::kFilterLsh: client.receive<'s'>(delay, path, "lsh"); break;
            case FilterType::kFilterHsh: client.receive<'s'>(delay, path, "hsh"); break;
            case FilterType::kFilterPeq: client.receive<'s'>(delay, path, "peq"); break;
            case FilterType::kFilterBpf4p: client.receive<'s'>(delay, path, "bpf_4p"); break;
            case FilterType::kFilterBpf6p: client.receive<'s'>(delay, path, "bpf_6p"); break;
            case FilterType::kFilterNone: client.receive<'s'>(delay, path, "none"); break;
            }
        } break;

        MATCH("/region&/eq&/gain", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, eq.gain);
        } break;

        MATCH("/region&/eq&/bandwidth", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, eq.bandwidth);
        } break;

        MATCH("/region&/eq&/frequency", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            client.receive<'f'>(delay, path, eq.frequency);
        } break;

        MATCH("/region&/eq&/vel&freq", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            if (indices[2] != 2)
                break;
            client.receive<'f'>(delay, path, eq.vel2frequency);
        } break;

        MATCH("/region&/eq&/vel&gain", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            if (indices[2] != 2)
                break;
            client.receive<'f'>(delay, path, eq.vel2gain);
        } break;

        MATCH("/region&/eq&/type", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EQ_OR_BREAK(indices[1])
            switch (eq.type) {
            case EqType::kEqNone: client.receive<'s'>(delay, path, "none"); break;
            case EqType::kEqPeak: client.receive<'s'>(delay, path, "peak"); break;
            case EqType::kEqLshelf: client.receive<'s'>(delay, path, "lshelf"); break;
            case EqType::kEqHshelf: client.receive<'s'>(delay, path, "hshelf"); break;
            }
        } break;

        MATCH("/region&/lfo&/wave", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_LFO_OR_BREAK(indices[1])
            if (lfo.sub.size() == 0)
                break;

            client.receive<'i'>(delay, path, static_cast<int32_t>(lfo.sub[0].wave));
        } break;

        MATCH("/region&/eg&/point&/time", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EG_OR_BREAK(indices[1])
            GET_EG_POINT_OR_BREAK(indices[2] + 1)

            client.receive<'f'>(delay, path, point.time);
        } break;

        MATCH("/region&/eg&/point&/time_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EG_OR_BREAK(indices[1])
            GET_EG_POINT_OR_BREAK(indices[2] + 1)

            client.receive<'f'>(delay, path, point.ccTime.getWithDefault(indices[3]));
        } break;

        MATCH("/region&/eg&/point&/level", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EG_OR_BREAK(indices[1])
            GET_EG_POINT_OR_BREAK(indices[2] + 1)

            client.receive<'f'>(delay, path, point.level);
        } break;

        MATCH("/region&/eg&/point&/level_cc&", "") {
            GET_REGION_OR_BREAK(indices[0])
            GET_EG_OR_BREAK(indices[1])
            GET_EG_POINT_OR_BREAK(indices[2] + 1)

            client.receive<'f'>(delay, path, point.ccLevel.getWithDefault(indices[3]));
        } break;

        #undef GET_REGION_OR_BREAK
        #undef GET_FILTER_OR_BREAK
        #undef GET_EQ_OR_BREAK
        #undef GET_LFO_OR_BREAK
        #undef GET_EG_OR_BREAK
        #undef GET_EG_POINT_OR_BREAK

        //----------------------------------------------------------------------
        // Setting values
        // Note: all these must be rt-safe within the parseOpcode method in region

        MATCH("/sample_quality", "i") {
            impl.resources_.getSynthConfig().liveSampleQuality =
                Opcode::transform(Default::sampleQuality, static_cast<int>(args[0].i));
        } break;

        MATCH("/oscillator_quality", "i") {
            impl.resources_.getSynthConfig().liveOscillatorQuality =
                Opcode::transform(Default::oscillatorQuality, static_cast<int>(args[0].i));
        } break;

        MATCH("/freewheeling_sample_quality", "i") {
            impl.resources_.getSynthConfig().freeWheelingSampleQuality =
                Opcode::transform(Default::freewheelingSampleQuality, static_cast<int>(args[0].i));
        } break;

        MATCH("/freewheeling_oscillator_quality", "i") {
            impl.resources_.getSynthConfig().freeWheelingOscillatorQuality =
                Opcode::transform(Default::freewheelingOscillatorQuality, static_cast<int>(args[0].i));
        } break;

        MATCH("/sustain_cancels_release", "T") {
            impl.resources_.getSynthConfig().sustainCancelsRelease = true;
        } break;

        MATCH("/sustain_cancels_release", "F") {
            impl.resources_.getSynthConfig().sustainCancelsRelease = false;
        } break;

        #define GET_REGION_OR_BREAK(idx)            \
            if (idx >= impl.layers_.size())         \
                break;                              \
            Layer& layer = *impl.layers_[idx];      \
            Region& region = layer.getRegion();

        #define GET_FILTER_OR_BREAK(idx)          \
            if (idx >= region.filters.size())     \
                break;                            \
            auto& filter = region.filters[idx];

        #define GET_LFO_OR_BREAK(idx)             \
            if (idx >= region.lfos.size())        \
                break;                            \
            auto& lfo = region.lfos[idx];

        #define GET_LFO_SUB_OR_BREAK(idx)         \
            if (idx >= lfo.sub.size())            \
                break;                            \
            auto& sub = lfo.sub[idx];

        MATCH("/region&/pitch_keycenter", "i") {
            GET_REGION_OR_BREAK(indices[0])
            region.pitchKeycenter = Opcode::transform(Default::key, args[0].i);
        } break;

        MATCH("/region&/loop_mode", "s") {
            GET_REGION_OR_BREAK(indices[0])
            region.loopMode = Opcode::readOptional(Default::loopMode, args[0].s);
        } break;

        MATCH("/region&/filter&/type", "s") {
            GET_REGION_OR_BREAK(indices[0])
            GET_FILTER_OR_BREAK(indices[1])
            filter.type = Opcode::read(Default::filter, args[0].s);
        } break;

        MATCH("/region&/lfo&/wave", "i") {
            indices[2] = 0;
            goto set_lfoN_wave;
        } break;

        MATCH("/region&/lfo&/wave&", "i") {
        set_lfoN_wave:
            GET_REGION_OR_BREAK(indices[0])
            GET_LFO_OR_BREAK(indices[1])
            GET_LFO_SUB_OR_BREAK(indices[2])
            sub.wave = Opcode::transform(Default::lfoWave, args[0].i);
        } break;

        #undef GET_REGION_OR_BREAK
        #undef GET_FILTER_OR_BREAK
        #undef GET_LFO_OR_BREAK
        #undef GET_LFO_SUB_OR_BREAK

        //----------------------------------------------------------------------
        // Voices

        MATCH("/num_active_voices", "") {
            client.receive<'i'>(delay, path, impl.voiceManager_.getNumActiveVoices());
        } break;

        #define GET_VOICE_OR_BREAK(idx)                     \
            if (static_cast<int>(idx) >= impl.numVoices_)   \
                break;                                      \
            const auto& voice = impl.voiceManager_[idx];    \
            if (voice.isFree())                             \
                break;

        MATCH("/voice&/trigger_value", "") {
            GET_VOICE_OR_BREAK(indices[0])
            client.receive<'f'>(delay, path, voice.getTriggerEvent().value);
        } break;

        MATCH("/voice&/trigger_number", "") {
            GET_VOICE_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, voice.getTriggerEvent().number);
        } break;

        MATCH("/voice&/trigger_type", "") {
            GET_VOICE_OR_BREAK(indices[0])
            const auto& event = voice.getTriggerEvent();
            switch (event.type) {
            case TriggerEventType::CC:
                client.receive<'s'>(delay, path, "cc");
                break;
            case TriggerEventType::NoteOn:
                client.receive<'s'>(delay, path, "note_on");
                break;
            case TriggerEventType::NoteOff:
                client.receive<'s'>(delay, path, "note_on");
                break;
            }

        } break;

        MATCH("/voice&/remaining_delay", "") {
            GET_VOICE_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, voice.getRemainingDelay());
        } break;

        MATCH("/voice&/source_position", "") {
            GET_VOICE_OR_BREAK(indices[0])
            client.receive<'i'>(delay, path, voice.getSourcePosition());
        } break;

        #undef MATCH
        // TODO...
    }
}

static bool extractMessage(const char* pattern, const char* path, unsigned* indices)
{
    unsigned nthIndex = 0;

    while (const char *endp = strchr(pattern, '&')) {
        if (nthIndex == maxIndices)
            return false;

        size_t length = endp - pattern;
        if (strncmp(pattern, path, length))
            return false;
        pattern += length;
        path += length;

        length = 0;
        while (absl::ascii_isdigit(path[length]))
            ++length;

        if (!absl::SimpleAtoi(absl::string_view(path, length), &indices[nthIndex++]))
            return false;

        pattern += 1;
        path += length;
    }

    return !strcmp(path, pattern);
}

static uint64_t hashMessagePath(const char* path, const char* sig)
{
    uint64_t h = Fnv1aBasis;
    while (unsigned char c = *path++) {
        if (!absl::ascii_isdigit(c))
            h = hashByte(c, h);
        else {
            h = hashByte('&', h);
            while (absl::ascii_isdigit(*path))
                ++path;
        }
    }
    h = hashByte(',', h);
    while (unsigned char c = *sig++)
        h = hashByte(c, h);
    return h;
}

} // namespace sfz
