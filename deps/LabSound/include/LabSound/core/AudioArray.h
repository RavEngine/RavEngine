// License: BSD 3 Clause
// Copyright (C) 2010, Google Inc. All rights reserved.
// Copyright (C) 2015+, The LabSound Authors. All rights reserved.

#ifndef AudioArray_h
#define AudioArray_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace lab
{

template <typename T>
class AudioArray
{
public:
    AudioArray()
        : m_allocation(0)
        , m_alignedData(0)
        , m_size(0)
    {
    }
    explicit AudioArray(size_t n)
        : m_allocation(0)
        , m_alignedData(0)
        , m_size(0)
    {
        allocate(n);
    }

    ~AudioArray()
    {
        free(m_allocation);
    }

    // It's OK to call allocate() multiple times, but data will *not* be copied from an initial allocation
    // if re-allocated. Allocations are zero-initialized.
    void allocate(size_t n)
    {
        size_t initialSize = sizeof(T) * n;

        const size_t alignment = 16;

        if (m_allocation)
            free(m_allocation);

        bool isAllocationGood = false;

        while (!isAllocationGood)
        {
            // Initially we try to allocate the exact size, but if it's not aligned
            // then we'll have to reallocate and from then on allocate extra.
            static size_t extraAllocationBytes = 0;

            T * allocation = static_cast<T *>(malloc(initialSize + extraAllocationBytes));
            if (!allocation)
            {
                //CRASH(); // dimitri
            }
            T * alignedData = alignedAddress(allocation, alignment);

            if (alignedData == allocation || extraAllocationBytes == alignment)
            {
                m_allocation = allocation;
                m_alignedData = alignedData;
                m_size = n;
                isAllocationGood = true;
                zero();
            }
            else
            {
                extraAllocationBytes = alignment;  // always allocate extra after the first alignment failure.
                free(allocation);
            }
        }
    }

    T * data() { return m_alignedData; }
    const T * data() const { return m_alignedData; }
    size_t size() const { return m_size; }

    T & at(size_t i)
    {
        // Note that although it is a size_t, m_size is now guaranteed to be
        // no greater than max unsigned. This guarantee is enforced in allocate().
        return data()[i];
    }

    T & operator[](size_t i) { return at(i); }

    void zero()
    {
        // This multiplication is made safe by the check in allocate().
        memset(this->data(), 0, sizeof(T) * this->size());
    }

    void zeroRange(unsigned start, unsigned end)
    {
        bool isSafe = (start <= end) && (end <= this->size());
        if (!isSafe)
            return;

        // This expression cannot overflow because end - start cannot be
        // greater than m_size, which is safe due to the check in allocate().
        memset(this->data() + start, 0, sizeof(T) * (end - start));
    }

    void copyToRange(const T * sourceData, unsigned start, unsigned end)
    {
        bool isSafe = (start <= end) && (end <= this->size());
        if (!isSafe)
            return;

        // This expression cannot overflow because end - start cannot be
        // greater than m_size, which is safe due to the check in allocate().
        memcpy(this->data() + start, sourceData, sizeof(T) * (end - start));
    }

private:
    static T * alignedAddress(T * address, intptr_t alignment)
    {
        intptr_t value = reinterpret_cast<intptr_t>(address);
        return reinterpret_cast<T *>((value + alignment - 1) & ~(alignment - 1));
    }

    T * m_allocation;
    T * m_alignedData;
    size_t m_size;
};

typedef AudioArray<float> AudioFloatArray;
typedef AudioArray<double> AudioDoubleArray;

}  // lab

#endif  // AudioArray_h
