#pragma once

#include "Synth.h"
#include "Effects.h"
#include "SisterVoiceRing.h"
#include "TriggerEvent.h"
#include "VoiceManager.h"
#include "Layer.h"
#include "BitArray.h"
#include "modulations/sources/ADSREnvelope.h"
#include "modulations/sources/Controller.h"
#include "modulations/sources/FlexEnvelope.h"
#include "modulations/sources/ChannelAftertouch.h"
#include "modulations/sources/PolyAftertouch.h"
#include "modulations/sources/LFO.h"
#include "parser/Parser.h"
#include "parser/ParserListener.h"

namespace sfz {

struct Synth::Impl final: public Parser::Listener {
    Impl();
    ~Impl();

    /**
     * @brief The parser callback; this is called by the parent object each time
     * a new region, group, master, global, curve or control set of opcodes
     * appears in the parser
     *
     * @param header the header for the set of opcodes
     * @param members the opcode members
     */
    void onParseFullBlock(const std::string& header, const std::vector<Opcode>& members) final;

    /**
     * @brief The parser callback when an error occurs.
     */
    void onParseError(const SourceRange& range, const std::string& message) final;

    /**
     * @brief The parser callback when a warning occurs.
     */
    void onParseWarning(const SourceRange& range, const std::string& message) final;

    /**
     * @brief Reset all CCs; to be used on CC 121
     *
     * @param delay the delay for the controller reset
     *
     */
    void resetAllControllers(int delay) noexcept;

    /**
     * @brief Remove all regions, resets all voices and clears everything
     * to bring back the synth in its original state.
     *
     * The callback mutex should be taken to call this function.
     */
    void clear();

    /**
     * @brief Helper function to dispatch <global> opcodes
     *
     * @param members the opcodes of the <global> block
     */
    void handleGlobalOpcodes(const std::vector<Opcode>& members);
    /**
     * @brief Helper function to dispatch <master> opcodes
     *
     * @param members the opcodes of the <master> block
     */
    void handleMasterOpcodes(const std::vector<Opcode>& members);
    /**
     * @brief Helper function to dispatch <group> opcodes
     *
     * @param members the opcodes of the <group> block
     */
    void handleGroupOpcodes(const std::vector<Opcode>& members, const std::vector<Opcode>& masterMembers);
    /**
     * @brief Helper function to dispatch <control> opcodes
     *
     * @param members the opcodes of the <control> block
     */
    void handleControlOpcodes(const std::vector<Opcode>& members);
    /**
     * @brief Helper function to dispatch <effect> opcodes
     *
     * @param members the opcodes of the <effect> block
     */
    void handleEffectOpcodes(const std::vector<Opcode>& members);
    /**
     * @brief Helper function to dispatch <effect> opcodes
     *
     * @param members the opcodes of the <effect> block
     */
    void handleSampleOpcodes(const std::vector<Opcode>& members);
    /**
     * @brief Helper function to merge all the currently active opcodes
     * as set by the successive callbacks and create a new region to store
     * in the synth.
     *
     * @param regionOpcodes the opcodes that are specific to the region
     */
    void buildRegion(const std::vector<Opcode>& regionOpcodes);
    /**
     * @brief Resets and possibly changes the number of voices (polyphony) in
     * the synth.
     *
     * @param numVoices
     */
    void resetVoices(int numVoices);
    /**
     * @brief Make the stored settings take effect in all the voices
     */
    void applySettingsPerVoice();

    /**
     * @brief Establish all connections of the modulation matrix.
     */
    void setupModMatrix();

    /**
     * @brief Get the modification time of all included sfz files
     *
     * @return absl::optional<fs::file_time_type>
     */
    absl::optional<fs::file_time_type> checkModificationTime() const;

    /**
     * @brief Check all regions and start voices for note on events
     *
     * @param delay
     * @param noteNumber
     * @param velocity
     */
    void noteOnDispatch(int delay, int noteNumber, float velocity) noexcept;

    /**
     * @brief Check all regions and start voices for note off events
     *
     * @param delay
     * @param noteNumber
     * @param velocity
     */
    void noteOffDispatch(int delay, int noteNumber, float velocity) noexcept;

    /**
     * @brief Check all regions and start voices for cc events
     *
     * @param delay
     * @param ccNumber
     * @param value
     */
    void ccDispatch(int delay, int ccNumber, float value) noexcept;

    /**
     * @brief Start a voice for a specific region.
     * This will do the needed polyphony checks and voice stealing.
     *
     * @param layer
     * @param delay
     * @param triggerEvent
     * @param ring
     */
    void startVoice(Layer* layer, int delay, const TriggerEvent& triggerEvent, SisterVoiceRingBuilder& ring) noexcept;

    /**
     * @brief Start all delayed sustain release voices of the region if necessary
     *
     * @param layer
     * @param delay
     * @param ring
     */
    void startDelayedSustainReleases(Layer* layer, int delay, SisterVoiceRingBuilder& ring) noexcept;

    /**
     * @brief Start all delayed sostenuto release voices of the region if necessary
     *
     * @param layer
     * @param delay
     * @param ring
     */
    void startDelayedSostenutoReleases(Layer* layer, int delay, SisterVoiceRingBuilder& ring) noexcept;

    /**
     * @brief Reset the default CCs
     *
     */
    void resetDefaultCCValues() noexcept;

    /**
     * @brief Prepare before loading a new SFZ file. The behavior of this function
     * is changed by the reloading state.
     *
     * @param path
     */
    void prepareSfzLoad(const fs::path& path);

    /**
     * @brief Finalize SFZ loading, following a successful execution of the
     * parsing step. The behavior of this function is changed by the reloading
     * state.
     */
    void finalizeSfzLoad();

    /**
     * @brief Set the current keyswitch, taking into account octave offsets and the like.
     *
     * @param noteValue
     */
    void setCurrentSwitch(uint8_t noteValue);

    template<class T>
    static void collectUsedCCsFromCCMap(BitArray<config::numCCs>& usedCCs, const CCMap<T> map) noexcept
    {
        for (auto& mod : map)
            usedCCs.set(mod.cc);
    }

    static void collectUsedCCsFromRegion(BitArray<config::numCCs>& usedCCs, const Region& region);
    static void collectUsedCCsFromModulations(BitArray<config::numCCs>& usedCCs, const ModMatrix& mm);

    BitArray<config::numCCs> collectAllUsedCCs();

    const std::string* getKeyLabel(int keyNumber) const;
    void setKeyLabel(int keyNumber, std::string name);
    void clearKeyLabels();
    const std::string* getCCLabel(int ccNumber) const;
    void setCCLabel(int ccNumber, std::string name);
    void clearCCLabels();
    const std::string* getKeyswitchLabel(int swNumber) const;
    void setKeyswitchLabel(int swNumber, std::string name);
    void clearKeyswitchLabels();

    /**
     * @brief Perform a CC event
     *
     * @param delay      The delay
     * @param ccNumber   The CC number
     * @param normValue  The normalized value
     * @param asMidi     Whether to process as a MIDI event
     */
    void performHdcc(int delay, int ccNumber, float normValue, bool asMidi) noexcept;

    /**
     * @brief Set the default value for a CC
     *
     * @param ccNumber
     * @param value
     */
    void setDefaultHdcc(int ccNumber, float value);

    /**
     * @brief Check if we have to kill any voice when starting a new one
     *      on the specified region with the specified note/cc number
     *
     * @param region
     * @param delay
     * @param number
     */
    void checkOffGroups(const Region* region, int delay, int number);

    /**
     * @brief Resets the callback duration breakdown to 0
     */
    void resetCallbackBreakdown();

    int numGroups_ { 0 };
    int numMasters_ { 0 };
    int numOutputs_ { 1 };

    // Opcode memory; these are used to build regions, as a new region
    // will integrate opcodes from the group, master and global block
    std::vector<Opcode> globalOpcodes_;
    std::vector<Opcode> masterOpcodes_;
    std::vector<Opcode> groupOpcodes_;

    // Names for the CC and notes as set by label_cc and label_key
    std::vector<CCNamePair> ccLabels_;
    std::map<int, size_t> ccLabelsMap_;
    std::vector<NoteNamePair> keyLabels_;
    std::map<int, size_t> keyLabelsMap_;
    BitArray<128> keySlots_;
    BitArray<128> swLastSlots_;
    BitArray<128> sustainOrSostenuto_;
    std::vector<NoteNamePair> keyswitchLabels_;
    std::map<int, size_t> keyswitchLabelsMap_;

    // Set as sw_default if present in the file
    absl::optional<uint8_t> currentSwitch_;
    std::vector<std::string> unknownOpcodes_;
    using RegionViewVector = std::vector<Region*>;
    using LayerViewVector = std::vector<Layer*>;
    using VoiceViewVector = std::vector<Voice*>;
    using LayerPtr = std::unique_ptr<Layer>;
    using RegionPtr = std::unique_ptr<Region>;
    using RegionSetPtr = std::unique_ptr<RegionSet>;
    std::vector<LayerPtr> layers_;
    VoiceManager voiceManager_;

    // These are more general "groups" than sfz and encapsulates the full hierarchy
    RegionSet* currentSet_ { nullptr };
    std::vector<RegionSetPtr> sets_;

    std::array<LayerViewVector, 128> lastKeyswitchLists_;
    std::array<LayerViewVector, 128> downKeyswitchLists_;
    std::array<LayerViewVector, 128> upKeyswitchLists_;
    LayerViewVector previousKeyswitchLists_;
    std::array<LayerViewVector, 128> noteActivationLists_;
    std::array<LayerViewVector, config::numCCs> ccActivationLists_;

    // Effect factory and buses
    EffectFactory effectFactory_;
    typedef std::unique_ptr<EffectBus> EffectBusPtr;
    std::vector<std::vector<EffectBusPtr>> effectBuses_; // first index is the output, then 0 is "main", 1-N are "fx1"-"fxN"
    const std::vector<EffectBusPtr>& getEffectBusesForOutput(uint16_t numOutput) { return effectBuses_[numOutput]; }
    void initEffectBuses();
    void addEffectBusesIfNecessary(uint16_t output);

    int samplesPerBlock_ { config::defaultSamplesPerBlock };
    float sampleRate_ { config::defaultSampleRate };
    float volume_ { Default::globalVolume };
    int numVoices_ { config::numVoices };

    // Distribution used to generate random value for the *rand opcodes
    std::uniform_real_distribution<float> randNoteDistribution_ { 0, 1 };

    // Singletons passed as references to the voices
    Resources resources_;

    // Root path
    std::string rootPath_;

    // Control opcodes
    std::string defaultPath_ { "" };
    std::string image_ { "" };
    int noteOffset_ { Default::noteOffset };
    int octaveOffset_ { Default::octaveOffset };

    // Modulation source generators
    std::unique_ptr<ControllerSource> genController_;
    std::unique_ptr<LFOSource> genLFO_;
    std::unique_ptr<FlexEnvelopeSource> genFlexEnvelope_;
    std::unique_ptr<ADSREnvelopeSource> genADSREnvelope_;
    std::unique_ptr<ChannelAftertouchSource> genChannelAftertouch_;
    std::unique_ptr<PolyAftertouchSource> genPolyAftertouch_;

    // Settings per voice
    struct {
        size_t maxFilters { 0 };
        size_t maxEQs { 0 };
        size_t maxLFOs { 0 };
        size_t maxFlexEGs { 0 };
        bool havePitchEG { false };
        bool haveFilterEG { false };
        bool haveAmplitudeLFO { false };
        bool havePitchLFO { false };
        bool haveFilterLFO { false };
    } settingsPerVoice_;

    CallbackBreakdown callbackBreakdown_;
    double dispatchDuration_ { 0 };

    std::chrono::time_point<std::chrono::high_resolution_clock> lastGarbageCollection_;

    Parser parser_;
    std::string lastPath_;
    absl::optional<fs::file_time_type> modificationTime_ { };
    bool reloading { false };

    std::array<float, config::numCCs> defaultCCValues_ { };
    BitArray<config::numCCs> currentUsedCCs_;
    BitArray<config::numCCs> changedCCsThisCycle_;
    BitArray<config::numCCs> changedCCsLastCycle_;

    // Messaging
    sfizz_receive_t* broadcastReceiver = nullptr;
    void* broadcastData = nullptr;

    Client getBroadcaster() const
    {
        Client client(broadcastData);
        client.setReceiveCallback(broadcastReceiver);
        return client;
    }

    bool playheadMoved_ { false };
};

} // namespace sfz
