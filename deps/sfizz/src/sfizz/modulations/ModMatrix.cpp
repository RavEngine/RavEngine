// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ModMatrix.h"
#include "ModId.h"
#include "ModKey.h"
#include "ModGenerator.h"
#include "Buffer.h"
#include "Config.h"
#include "SIMDHelpers.h"
#include "utility/Debug.h"
#include <absl/container/flat_hash_map.h>
#include <absl/strings/string_view.h>
#include <vector>
#include <algorithm>

namespace sfz {

struct ModMatrix::Impl {
    double sampleRate_ {};
    uint32_t samplesPerBlock_ {};

    uint32_t numFrames_ {};
    NumericId<Voice> currentVoiceId_ {};
    NumericId<Region> currentRegionId_ {};

    float currentVoiceTriggerValue_ {};

    struct Source {
        ModKey key;
        ModGenerator* gen {};
        bool bufferReady {};
        Buffer<float> buffer;
    };

    struct ConnectionData {
        float sourceDepth_ {};
        ModKey sourceDepthMod_ {};
        TargetId sourceDepthModId_ {};
        float velToDepth_ {};
    };

    struct Target {
        ModKey key;
        uint32_t region {};
        absl::flat_hash_map<uint32_t, ConnectionData> connectedSources;
        bool bufferReady {};
        Buffer<float> buffer;
    };

    absl::flat_hash_map<ModKey, uint32_t> sourceIndex_;
    absl::flat_hash_map<ModKey, uint32_t> targetIndex_;

    std::vector<uint32_t> sourceIndicesForGlobal_;
    std::vector<uint32_t> targetIndicesForGlobal_;

    int maxRegionIdx_ { -1 };
    std::vector<std::vector<uint32_t>> sourceIndicesForRegion_;
    std::vector<std::vector<uint32_t>> targetIndicesForRegion_;

    std::vector<Source> sources_;
    std::vector<Target> targets_;
};

ModMatrix::ModMatrix()
    : impl_(new Impl)
{
    setSampleRate(config::defaultSampleRate);
    setSamplesPerBlock(config::defaultSamplesPerBlock);
}

ModMatrix::~ModMatrix()
{
}

void ModMatrix::clear()
{
    Impl& impl = *impl_;

    impl.sourceIndex_.clear();
    impl.targetIndex_.clear();
    impl.sources_.clear();
    impl.targets_.clear();
    impl.sourceIndicesForGlobal_.clear();
    impl.targetIndicesForGlobal_.clear();
    impl.sourceIndicesForRegion_.clear();
    impl.targetIndicesForRegion_.clear();
    impl.maxRegionIdx_ = -1;
}

void ModMatrix::setSampleRate(double sampleRate)
{
    Impl& impl = *impl_;

    if (impl.sampleRate_ == sampleRate)
        return;

    impl.sampleRate_ = sampleRate;

    for (Impl::Source &source : impl.sources_)
        source.gen->setSampleRate(sampleRate);
}

void ModMatrix::setSamplesPerBlock(unsigned samplesPerBlock)
{
    Impl& impl = *impl_;

    if (impl.samplesPerBlock_ == samplesPerBlock)
        return;

    impl.samplesPerBlock_ = samplesPerBlock;

    for (Impl::Source &source : impl.sources_) {
        source.buffer.resize(samplesPerBlock);
        source.gen->setSamplesPerBlock(samplesPerBlock);
    }
    for (Impl::Target &target : impl.targets_)
        target.buffer.resize(samplesPerBlock);
}

ModMatrix::SourceId ModMatrix::registerSource(const ModKey& key, ModGenerator& gen)
{
    Impl& impl = *impl_;

    auto it = impl.sourceIndex_.find(key);
    if (it != impl.sourceIndex_.end()) {
        ASSERT(&gen == impl.sources_[it->second].gen);
        return SourceId(it->second);
    }

    SourceId id(static_cast<int>(impl.sources_.size()));
    impl.sources_.emplace_back();

    Impl::Source &source = impl.sources_.back();
    source.key = key;
    source.gen = &gen;
    source.bufferReady = false;
    source.buffer.resize(impl.samplesPerBlock_);

    impl.sourceIndex_[key] = id.number();
    if (key.region().number() > impl.maxRegionIdx_)
        impl.maxRegionIdx_ = key.region().number();

    gen.setSampleRate(impl.sampleRate_);
    gen.setSamplesPerBlock(impl.samplesPerBlock_);

    return id;
}

ModMatrix::TargetId ModMatrix::registerTarget(const ModKey& key)
{
    Impl& impl = *impl_;

    auto it = impl.targetIndex_.find(key);
    if (it != impl.targetIndex_.end())
        return TargetId(it->second);

    TargetId id(static_cast<int>(impl.targets_.size()));
    impl.targets_.emplace_back();

    Impl::Target &target = impl.targets_.back();
    target.key = key;
    target.bufferReady = false;
    target.buffer.resize(impl.samplesPerBlock_);

    impl.targetIndex_[key] = id.number();
    if (key.region().number() > impl.maxRegionIdx_)
        impl.maxRegionIdx_ = key.region().number();

    return id;
}

ModMatrix::SourceId ModMatrix::findSource(const ModKey& key) const
{
    Impl& impl = *impl_;

    auto it = impl.sourceIndex_.find(key);
    if (it == impl.sourceIndex_.end())
        return {};

    return SourceId(it->second);
}

ModMatrix::TargetId ModMatrix::findTarget(const ModKey& key) const
{
    Impl& impl = *impl_;

    auto it = impl.targetIndex_.find(key);
    if (it == impl.targetIndex_.end())
        return {};

    return TargetId(it->second);
}

bool ModMatrix::connect(SourceId sourceId, TargetId targetId, float sourceDepth, const ModKey& sourceDepthMod, float velToDepth)
{
    Impl& impl = *impl_;
    unsigned sourceIndex = sourceId.number();
    unsigned targetIndex = targetId.number();

    if (sourceIndex >= impl.sources_.size() || targetIndex >= impl.targets_.size())
        return false;

    Impl::Target& target = impl.targets_[targetIndex];
    Impl::ConnectionData& conn = target.connectedSources[sourceIndex];
    conn.sourceDepth_ = sourceDepth;
    conn.sourceDepthMod_ = sourceDepthMod;

    if (sourceDepthMod)
        conn.sourceDepthModId_ = registerTarget(sourceDepthMod);

    conn.velToDepth_ = velToDepth;

    return true;
}

void ModMatrix::init()
{
    Impl& impl = *impl_;

    if (impl.maxRegionIdx_ >= 0) {
        const size_t numRegions = impl.maxRegionIdx_ + 1;
        impl.sourceIndicesForRegion_.resize(numRegions);
        impl.targetIndicesForRegion_.resize(numRegions);
    }

    for (unsigned i = 0; i < impl.sources_.size(); ++i) {
        Impl::Source& source = impl.sources_[i];
        const int flags = source.key.flags();
        if (flags & kModIsPerCycle) {
            ASSERT(!source.key.region());
            source.gen->init(source.key, {}, 0);
            impl.sourceIndicesForGlobal_.push_back(i);
        }
        else if (flags & kModIsPerVoice) {
            ASSERT(source.key.region());
            impl.sourceIndicesForRegion_[source.key.region().number()].push_back(i);
        }
    }

    for (unsigned i = 0; i < impl.targets_.size(); ++i) {
        Impl::Target& target = impl.targets_[i];
        const int flags = target.key.flags();
        if (flags & kModIsPerCycle) {
            ASSERT(!target.key.region());
            impl.targetIndicesForGlobal_.push_back(i);
        }
        else if (flags & kModIsPerVoice) {
            ASSERT(target.key.region());
            impl.targetIndicesForRegion_[target.key.region().number()].push_back(i);
        }
    }
}

void ModMatrix::initVoice(NumericId<Voice> voiceId, NumericId<Region> regionId, unsigned delay)
{
    Impl& impl = *impl_;
    ASSERT(regionId);
    ASSERT(static_cast<size_t>(regionId.number()) < impl.sourceIndicesForRegion_.size());

    const auto idNumber = static_cast<size_t>(regionId.number());
    for (auto idx: impl.sourceIndicesForRegion_[idNumber]) {
        const Impl::Source& source = impl.sources_[idx];
        source.gen->init(source.key, voiceId, delay);
    }
}

void ModMatrix::releaseVoice(NumericId<Voice> voiceId, NumericId<Region> regionId, unsigned delay)
{
    Impl& impl = *impl_;

    ASSERT(regionId);

    const auto idNumber = static_cast<size_t>(regionId.number());
    for (auto idx: impl.sourceIndicesForRegion_[idNumber]) {
        const Impl::Source& source = impl.sources_[idx];
        source.gen->release(source.key, voiceId, delay);
    }
}

void ModMatrix::cancelRelease(NumericId<Voice> voiceId, NumericId<Region> regionId, unsigned delay)
{
    Impl& impl = *impl_;

    ASSERT(regionId);

    const auto idNumber = static_cast<size_t>(regionId.number());
    for (auto idx: impl.sourceIndicesForRegion_[idNumber]) {
        const Impl::Source& source = impl.sources_[idx];
        source.gen->cancelRelease(source.key, voiceId, delay);
    }
}

void ModMatrix::beginCycle(unsigned numFrames)
{
    Impl& impl = *impl_;

    impl.numFrames_ = numFrames;

    for (auto idx: impl.sourceIndicesForGlobal_) {
        Impl::Source& source = impl.sources_[idx];
        source.bufferReady = false;
    }
    for (auto idx: impl.targetIndicesForGlobal_) {
        Impl::Target& target = impl.targets_[idx];
        target.bufferReady = false;
    }
}

void ModMatrix::endCycle()
{
    Impl& impl = *impl_;
    const uint32_t numFrames = impl.numFrames_;

    for (auto idx: impl.sourceIndicesForGlobal_) {
        Impl::Source& source = impl.sources_[idx];
        if (!source.bufferReady) {
            absl::Span<float> buffer(source.buffer.data(), numFrames);
            source.gen->generateDiscarded(source.key, {}, buffer);
        }
    }

    impl.numFrames_ = 0;
}

void ModMatrix::beginVoice(NumericId<Voice> voiceId, NumericId<Region> regionId, float triggerValue)
{
    Impl& impl = *impl_;

    impl.currentVoiceId_ = voiceId;
    impl.currentRegionId_ = regionId;

    impl.currentVoiceTriggerValue_ = triggerValue;

    ASSERT(regionId);

    const auto idNumber = static_cast<size_t>(regionId.number());
    for (auto idx: impl.sourceIndicesForRegion_[idNumber]) {
        Impl::Source& source = impl.sources_[idx];
        source.bufferReady = false;
    }

    for (auto idx: impl.targetIndicesForRegion_[idNumber]) {
        Impl::Target& target = impl.targets_[idx];
        target.bufferReady = false;
    }
}

void ModMatrix::endVoice()
{
    Impl& impl = *impl_;
    const uint32_t numFrames = impl.numFrames_;
    const NumericId<Voice> voiceId = impl.currentVoiceId_;
    const NumericId<Region> regionId = impl.currentRegionId_;

    ASSERT(regionId);
    ASSERT(static_cast<size_t>(regionId.number()) < impl.sourceIndicesForRegion_.size());

    const auto idNumber = static_cast<size_t>(regionId.number());

    for (auto idx: impl.sourceIndicesForRegion_[idNumber]) {
        const Impl::Source& source = impl.sources_[idx];
        if (!source.bufferReady) {
            absl::Span<float> buffer(source.buffer.data(), numFrames);
            source.gen->generateDiscarded(source.key, voiceId, buffer);
        }
    }

    impl.currentVoiceId_ = {};
    impl.currentRegionId_ = {};

    impl.currentVoiceTriggerValue_ = 0.0f;
}

float* ModMatrix::getModulation(TargetId targetId)
{
    if (!validTarget(targetId))
        return nullptr;

    Impl& impl = *impl_;
    const NumericId<Region> regionId = impl.currentRegionId_;
    const float triggerValue = impl.currentVoiceTriggerValue_;
    const uint32_t targetIndex = targetId.number();
    Impl::Target &target = impl.targets_[targetIndex];
    const int targetFlags = target.key.flags();

    const uint32_t numFrames = impl.numFrames_;
    absl::Span<float> buffer(target.buffer.data(), numFrames);

    // only accept per-voice targets of the same region
    if ((targetFlags & kModIsPerVoice) && regionId != target.key.region())
        return nullptr;

    // check if already processed
    if (target.bufferReady)
        return buffer.data();

    // set the ready flag to prevent a cycle
    // in case there is, be sure to initialize the buffer
    target.bufferReady = true;

    auto sourcesPos = target.connectedSources.begin();
    auto sourcesEnd = target.connectedSources.end();
    bool isFirstSource = true;

    // generate sources in their dedicated buffers
    // then add or multiply, depending on target flags
    while (sourcesPos != sourcesEnd) {
        Impl::Source &source = impl.sources_[sourcesPos->first];
        const int sourceFlags = source.key.flags();

        // only accept per-voice sources of the same region
        bool useThisSource = true;
        if (sourceFlags & kModIsPerVoice)
            useThisSource = (regionId == source.key.region());

        if (useThisSource) {
            absl::Span<float> sourceBuffer(source.buffer.data(), numFrames);

            // unless source is already done, process it
            if (!source.bufferReady) {
                source.gen->generate(source.key, impl.currentVoiceId_, sourceBuffer);
                source.bufferReady = true;
            }

            float sourceDepth = sourcesPos->second.sourceDepth_;
            if (sourceFlags & kModIsPerVoice) {
                const float velToDepth = sourcesPos->second.velToDepth_;
                sourceDepth += triggerValue * velToDepth;
            }

            const float* sourceDepthMod = getModulation(sourcesPos->second.sourceDepthModId_);

            if (isFirstSource) {
                if (sourceDepth == 1 && !sourceDepthMod)
                    copy(absl::Span<const float>(sourceBuffer), buffer);
                else if (!sourceDepthMod) {
                    for (uint32_t i = 0; i < numFrames; ++i)
                        buffer[i] = sourceDepth * sourceBuffer[i];
                }
                else if (targetFlags & kModIsMultiplicative) {
                    for (uint32_t i = 0; i < numFrames; ++i)
                        buffer[i] = (sourceDepth * sourceDepthMod[i]) * sourceBuffer[i];
                }
                else {
                    ASSERT(targetFlags & kModIsAdditive);
                    for (uint32_t i = 0; i < numFrames; ++i)
                        buffer[i] = (sourceDepth + sourceDepthMod[i]) * sourceBuffer[i];
                }
                isFirstSource = false;
            }
            else {
                if (targetFlags & kModIsMultiplicative) {
                    if (!sourceDepthMod)
                        multiplyMul1<float>(sourceDepth, sourceBuffer, buffer);
                    else {
                        for (uint32_t i = 0; i < numFrames; ++i)
                            buffer[i] *= (sourceDepth * sourceDepthMod[i]) * sourceBuffer[i];
                    }
                }
                else {
                    ASSERT(targetFlags & kModIsAdditive);
                    if (!sourceDepthMod)
                        multiplyAdd1<float>(sourceDepth, sourceBuffer, buffer);
                    else {
                        for (uint32_t i = 0; i < numFrames; ++i)
                            buffer[i] += (sourceDepth + sourceDepthMod[i]) * sourceBuffer[i];
                    }
                }
            }
        }

        ++sourcesPos;
    }

    // if there were no source, fill output with the neutral element
    if (isFirstSource) {
        if (targetFlags & kModIsMultiplicative)
            fill(buffer, 1.0f);
        else {
            ASSERT(targetFlags & kModIsAdditive);
            fill(buffer, 0.0f);
        }
    }

    return buffer.data();
}

bool ModMatrix::validTarget(TargetId id) const
{
    return static_cast<unsigned>(id.number()) < impl_->targets_.size();
}

bool ModMatrix::validSource(SourceId id) const
{
    return static_cast<unsigned>(id.number()) < impl_->sources_.size();
}

std::string ModMatrix::toDotGraph() const
{
    const Impl& impl = *impl_;

    struct Edge {
        std::string source;
        std::string target;
    };

    // collect all connections as string pairs
    std::vector<Edge> edges;
    for (const Impl::Target& target : impl.targets_) {
        for (const auto& cs : target.connectedSources) {
            const Impl::Source& source = impl.sources_[cs.first];
            Edge e;
            e.source = source.key.toString();
            e.target = target.key.toString();
            edges.push_back(std::move(e));
        }
    }

    // alphabetic sort, to produce stable output for unit testing
    auto compare = [](const Edge& a, const Edge& b) -> bool {
        std::pair<absl::string_view, absl::string_view> aa{a.source, a.target};
        std::pair<absl::string_view, absl::string_view> bb{b.source, b.target};
        return aa < bb;
    };
    std::sort(edges.begin(), edges.end(), compare);

    // write dot graph
    std::string dot;
    dot.reserve(1024);
    absl::StrAppend(&dot, "digraph {" "\n");
    for (const Edge& e : edges) {
        absl::StrAppend(&dot, "\t" "\"", e.source, "\""
                        " -> " "\"", e.target, "\"" "\n");
    }
    absl::StrAppend(&dot, "}" "\n");
    return dot;
}

bool ModMatrix::visitSources(KeyVisitor& vtor) const
{
    const Impl& impl = *impl_;

    for (const Impl::Source& item : impl.sources_) {
        if (!vtor.visit(item.key))
            return false;
    }

    return true;
}

bool ModMatrix::visitTargets(KeyVisitor& vtor) const
{
    const Impl& impl = *impl_;

    for (const Impl::Target& item : impl.targets_) {
        if (!vtor.visit(item.key))
            return false;
    }

    return true;
}

} // namespace sfz
