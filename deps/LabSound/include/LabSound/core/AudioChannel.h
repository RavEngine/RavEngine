// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioChannel_h
#define AudioChannel_h

#include "LabSound/core/AudioArray.h"

#include <memory>

namespace lab
{

// An AudioChannel represents a buffer of non-interleaved floating-point audio samples.
// The PCM samples are normally assumed to be in a nominal range -1.0 -> +1.0
class AudioChannel
{
    AudioChannel(const AudioChannel &);  // noncopyable

public:
    // Memory can be externally referenced, or can be internally allocated with an AudioFloatArray.

    // Reference an external buffer.
    AudioChannel(float * storage, int length)
        : m_length(length)
        , m_rawPointer(storage)
        , m_silent(false)
    {
    }

    // Manage storage for us.
    explicit AudioChannel(int length)
        : m_length(length)
        , m_silent(true)
    {
        m_memBuffer.reset(new AudioFloatArray(length));
    }

    // An empty audio channel -- must call set() before it's useful...
    AudioChannel()
        : m_length(0)
        , m_silent(true)
    {
    }

    // Redefine the memory for this channel.
    // storage represents external memory not managed by this object.
    void set(float * storage, int length)
    {
        m_memBuffer.reset();  // clean up managed storage
        m_rawPointer = storage;
        m_length = length;
        m_silent = false;
    }

    // How many sample-frames do we contain?
    int length() const { return m_length; }

    // resizeSmaller() can only be called with a new length <= the current length.
    // The data stored in the bus will remain undisturbed.
    void resizeSmaller(int newLength);

    // Direct access to PCM sample data. Non-const accessor clears silent flag.
    float * mutableData()
    {
        clearSilentFlag();
        return const_cast<float *>(data());
    }

    const float * data() const
    {
        if (m_rawPointer)
            return m_rawPointer;
        if (m_memBuffer)
            return m_memBuffer->data();
        return nullptr;
    }

    // Zeroes out all sample values in buffer.
    void zero()
    {
        if (m_silent) return;

        m_silent = true;

        if (m_memBuffer)
            m_memBuffer->zero();
        else if (m_rawPointer)
            memset(m_rawPointer, 0, sizeof(float) * m_length);
    }

    // Clears the silent flag.
    void clearSilentFlag() { m_silent = false; }

    bool isSilent() const { return m_silent; }

    // Scales all samples by the same amount.
    void scale(float scale);

    // A simple memcpy() from the source channel
    void copyFrom(const AudioChannel * sourceChannel);

    // Copies the given range from the source channel.
    void copyFromRange(const AudioChannel * sourceChannel, int startFrame, int endFrame);

    // Sums (with unity gain) from the source channel.
    void sumFrom(const AudioChannel * sourceChannel);

    // Returns maximum absolute value (useful for normalization).
    float maxAbsValue() const;

private:
    int m_length = 0;
    float * m_rawPointer = nullptr;
    std::unique_ptr<AudioFloatArray> m_memBuffer;
    bool m_silent = true;
};

}  // lab

#endif  // AudioChannel_h
