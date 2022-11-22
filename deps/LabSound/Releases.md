
## v1.1
# 23 October 2022

### Simplified channel management

Simplified channel management by automatically conforming signal processing 
pathways from the leaf nodes. Leaf node channel counts are propagated backwards 
towards the audio context such that intermediate nodes preserve channel counts, 
unless they have an explicit behavior of changing the channel count. 
For example, connecting a stereo source to a gain node will promote a mono gain 
node to stereo. A mono node connected to a stereo panner node will result in a 
stereo output.

### SampledAudioNode rewritten

SampledAudioNode has been rewritten to do it's own compositing when scheduled 
such that many instances play in an overlapping manner. This allows efficiently 
playing dozens of copies of the same sound simultaneously. The new algorithm 
requires less intermediate storage and has a much lower CPU cost. Detuning has 
been improved to allow a greater working range, and higher quality when 
pitching higher. SampledAudioNode no longer has an embedded stereo panning 
node, thus simplifying panning and compositing logic. Scheduling is now sample 
accurate, rather than quantum-accurate, and loop points and loop counts have 
been added.

### WaveShaperNode rewritten

The WaveShaperNode has been rewritten to minimize CPU overhead. Unnecessary
object orient complexity has been deleted.

### Improved threading

Threading and thread safety has been improved. The update service thread is 
nearly empty at this point, as graph modifications are now managed by the 
main update. Communications with other threads now occurs through concurrent 
lockfree queues. Many mutexes are now gone, as well as rare termination races.

### Miscellaneous

- autoconfigure channel counts
- rename BPMDelay.h to BPMDelayNode.h
- rewrite WaveShaper for efficiency
- rewrite SampledAudioNode for efficiency and sample accurate timing
- rewrite resampler for SampledAudioNode for quality and speed
- use concurrent queues to eliminate mutexes in audio callback
- minimize use of maintenance thread
- bulletproof asynchronous connects and disconnects of node wiring
- improved offline context interface
- recorder node can run as a pull node indepdently of device context
- improved miniaudio backend
- numerous Android fixes (by xioxin)



## v1.0.1
# 11 April 2021

- New platforms: Linux, Raspberry Pi, iOS, Android, in addition to macOS and Windows.
- various fixes to regressions in SampledAudioNode
- add accessors to expose node parameters for integration into DAWs
- various platform fixes for iOS and Android
- Github Actions now managing the CI

## v1.0
# 15 Jan 2021

This 1.0 release finalizes the API with functionality that allows full introspection of all nodes and attributes. To go with this 1.0, LabSoundGraphToy is also released at https://github.com/LabSound/LabSoundGraphToy to enable exploration of the library, its nodes, and its functionality.


- Much thread safety introduced
- ADSRNode now has a gate param that can be used to schedule note on and off
- Profiling system added
- Node scheduling reworked
- Audio popping mitigated
- RecorderNode defers mix to down to save
- RecorderNode create bus from recording
- SfxrNode exposes presets as settings
- SampledAudioNode has scheduled voices
- AudioNodes all take an AudioContext& during construction
- Offline rendering works consistently with online rendering, including automatic pull nodes
- Automatic pull nodes work reliably
- Updated use of data types to conform to LabSound conventions
- New GranulationNode
- fixed almost all of the compiler warnings
- removed bang in favor of an onStart callback that allows rescheduling in the processing call


## v0.14.0 Release Candidate
# 15 Mar 2020

- Removed many custom backends in favor of RtAudio and miniaudio
- Consolidated demos into a single header
- clang format applied to sources
- cloning method added to AudioBus
- Added Input and Output configuration structs to allow selection of devices
- names added for params and settings for use in tools
- Bus added as available Setting type
- ADSRNode timing fixed
- AnalyserNode can render output to any desired output buffer size by binning and interpolation
- AudioDevice nodes added in major backend refactoring
- ConvolverNode - new implementation
- DiodeNode is now derived from AudioNode
- OscillatorNode - new implementation
- SuperSaw is now scheduled

### Releases
------------

## v0.13.0 Release Candidate
# 18 Aug 2019

- AudioSettings. AudioParams already exist for rate varying values on Audio Nodes. AudioParams are registered on nodes by name, and can be looked up by name. Non-rate varying parameters such as filter modes, buffer sizes, and so on could not be similarly looked up by name. AudioSettings are introduced in this release for non-rate varying parameters, such as filter modes, buffer sizes and so on. Settings for each AudioNode are documented in its corresponding header file. A vector of param or setting names can be fetched from an AudioNode and used to populate a user interface. Each fetched AudioSetting can be queried as to whether its last set type was a Uint32 or a Float, in order that user interfaces can self configure appropriately.

- Added some nullchecks to AudioChannel memory management as pointed out by harmonicvision

## v0.12.0 Beta
# 4 Jan 2018

- clean up how AudioNodes are connected to one another. Formerly, the connect(...) method was called directly on an AudioNode, taking the graph lock and effecting changes to the AudioContext itself. This is no longer the case. AudioContext is now the arbiter of all graph changes.

## v0.11.0 Alpha
# 4 Jan 2018

- Numerous bug and maintenance fixes.
- A cmake build system.

## v0.10.0 Alpha
# 2 Jan 2016

- Several breaking API changes related to AudioContext creation and PannerNode initialization.

## v0.9.0 Alpha
# 2 Jan 2016

- Maintenance vs2015
