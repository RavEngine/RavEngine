// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Buffer.h"
#include "SIMDHelpers.h"
#include "absl/types/span.h"

namespace sfz
{
/**
 * @brief A naive circular buffer which is supposed to hold power values
 * and return the average of its content.
 *
 * @tparam ValueType
 */
template<class ValueType>
class HistoricalBuffer {
public:
	HistoricalBuffer() = delete;
	HistoricalBuffer(size_t size)
	: size(size)
	{
		resize(size);
	}

    /**
     * @brief Resize the underlying buffer. Newly added "slots" are
     * initialized to 0.0
     *
     * @param size
     */
	void resize(size_t size)
	{
		buffer.resize(size);
		fill(absl::MakeSpan(buffer), ValueType { 0 });
		index = 0;
        validMean = false;
	}

    /**
     * @brief Add a value to the buffer
     *
     * @param value
     */
	void push(ValueType value)
	{
        validMean = false;
		if (size > 0) {
			buffer[index] = value;
			if (++index == size)
				index = 0;
		}
	}

    /**
     * @brief Return the average of all the values in the buffer
     *
     * @return ValueType
     */
	ValueType getAverage() const
	{
        if (!validMean) {
            mean = sfz::mean<ValueType>(buffer);
            validMean = true;
        }

		return mean;
	}
private:
	Buffer<ValueType> buffer;
	size_t size { 0 };
	size_t index { 0 };
    mutable bool validMean { true };
    mutable ValueType mean { 0.0 };
};
}
