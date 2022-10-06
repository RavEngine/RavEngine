// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Oversampler.h"
#include "OversamplerHelpers.h"
#include "Buffer.h"
#include "AudioSpan.h"
#include "AudioReader.h"
#include "SIMDConfig.h"
#include <jsl/allocator>

template <class T, std::size_t A = sfz::config::defaultAlignment>
using aligned_vector = std::vector<T, jsl::aligned_allocator<T, A>>;

sfz::Oversampler::Oversampler(sfz::Oversampling factor, size_t chunkSize)
: factor(factor), chunkSize(chunkSize)
{

}

void sfz::Oversampler::stream(AudioSpan<float> input, AudioSpan<float> output, std::atomic<size_t>* framesReady)
{
    ASSERT(output.getNumFrames() >= input.getNumFrames() * static_cast<int>(factor));
    ASSERT(output.getNumChannels() == input.getNumChannels());

    const auto numFrames = input.getNumFrames();
    const auto numChannels = input.getNumChannels();

    aligned_vector<Upsampler> upsampler(numChannels);

    // Intermediate buffer
    sfz::Buffer<float> temp { std::max<size_t>(128, Upsampler::recommendedBuffer(16, chunkSize)) };

    size_t inputFrameCounter { 0 };
    size_t outputFrameCounter { 0 };
    while(inputFrameCounter < numFrames)
    {
        // std::cout << "Input frames: " << inputFrameCounter << "/" << numFrames << '\n';
        const auto thisChunkSize = std::min(chunkSize, numFrames - inputFrameCounter);
        const auto outputChunkSize = thisChunkSize * static_cast<int>(factor);
        for (size_t chanIdx = 0; chanIdx < numChannels; chanIdx++) {
            const auto inputChunk = input.getSpan(chanIdx).subspan(inputFrameCounter, thisChunkSize);
            const auto outputChunk = output.getSpan(chanIdx).subspan(outputFrameCounter, outputChunkSize);
            upsampler[chanIdx].process(
                static_cast<int>(factor),
                inputChunk.data(), outputChunk.data(), static_cast<int>(inputChunk.size()),
                temp.data(), static_cast<int>(temp.size()));
        }
        inputFrameCounter += thisChunkSize;
        outputFrameCounter += outputChunkSize;

        if (framesReady != nullptr)
            framesReady->fetch_add(outputChunkSize);
    }
}

void sfz::Oversampler::stream(AudioReader& input, AudioSpan<float> output, std::atomic<size_t>* framesReady)
{
    ASSERT(output.getNumFrames() >= static_cast<size_t>(input.frames() * static_cast<int>(factor)));
    ASSERT(output.getNumChannels() == input.channels());

    const auto numFrames = static_cast<size_t>(input.frames());
    const auto numChannels = input.channels();

    aligned_vector<Upsampler> upsampler(numChannels);

    // Intermediate buffers
    sfz::Buffer<float> fileBlock { chunkSize * numChannels };
    sfz::Buffer<float> channelBlock { chunkSize };
    sfz::Buffer<float> temp { std::max<size_t>(128, Upsampler::recommendedBuffer(16, chunkSize)) };

    auto deinterleave = [numChannels](
        float* output, const float* input, size_t numFrames, unsigned chanIdx)
    {
        for (size_t i = 0; i < numFrames; ++i)
            output[i] = input[i * numChannels + chanIdx];
    };

    size_t inputFrameCounter { 0 };
    size_t outputFrameCounter { 0 };
    bool inputEof = false;
    while (!inputEof && inputFrameCounter < numFrames)
    {
        // std::cout << "Input frames: " << inputFrameCounter << "/" << numFrames << '\n';
        auto thisChunkSize = std::min(chunkSize, numFrames - inputFrameCounter);
        const auto numFramesRead = static_cast<size_t>(
            input.readNextBlock(fileBlock.data(), thisChunkSize));
        if (numFramesRead == 0)
            break;
        if (numFramesRead < thisChunkSize) {
            inputEof = true;
            thisChunkSize = numFramesRead;
        }
        const auto outputChunkSize = thisChunkSize * static_cast<int>(factor);

        for (size_t chanIdx = 0; chanIdx < numChannels; chanIdx++) {
            const auto outputChunk = output.getSpan(chanIdx).subspan(outputFrameCounter, outputChunkSize);

            if (factor == Oversampling::x1)
                deinterleave(outputChunk.data(), fileBlock.data(), thisChunkSize, chanIdx);
            else {
                deinterleave(channelBlock.data(), fileBlock.data(), thisChunkSize, chanIdx);
                upsampler[chanIdx].process(
                    static_cast<int>(factor),
                    channelBlock.data(), outputChunk.data(), static_cast<int>(thisChunkSize),
                    temp.data(), static_cast<int>(temp.size()));
            }
        }
        inputFrameCounter += thisChunkSize;
        outputFrameCounter += outputChunkSize;

        if (framesReady != nullptr)
            framesReady->fetch_add(outputChunkSize);
    }
}
