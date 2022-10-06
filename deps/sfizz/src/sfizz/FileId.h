// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <string>
#include <memory>
#include <iosfwd>

namespace sfz {

/**
 * @brief Sample file identifier within a file pool.
 */
struct FileId {
    /**
     * @brief Construct a null identifier.
     */
    FileId()
    {
    }

    /**
     * @brief Construct a file identifier, optionally reversed.
     *
     * @param filename
     * @param reverse
     */
    explicit FileId(std::string filename, bool reverse = false)
        : filenameBuffer(new std::string(std::move(filename))),
          reverse(reverse)
    {
    }

    /**
     * @brief Make an identifier which is a clone of the callee, except with the
     * reverse flag passed as parameter.
     *
     * @param reverse
     */
    FileId reversed(bool reverse = true)
    {
        FileId id;
        id.filenameBuffer = filenameBuffer;
        id.reverse = reverse;
        return id;
    }

    /**
     * @brief Get the file name of this identifier.
     *
     * @return file name
     */
    const std::string &filename() const noexcept
    {
        return filenameBuffer ? *filenameBuffer : emptyFilename;
    }

    /**
     * @brief Get whether the identified file is reversed.
     *
     * @return bool
     */
    bool isReverse() const noexcept
    {
        return reverse;
    }

    /**
     * @brief Check equality with another identifier.
     *
     * @param other
     */
    bool operator==(const FileId &other) const
    {
        return reverse == other.reverse && filename() == other.filename();
    }

    /**
     * @brief Check inequality with another identifier.
     *
     * @param other
     */
    bool operator!=(const FileId &other) const
    {
        return !operator==(other);
    }

private:
    std::shared_ptr<std::string> filenameBuffer;
    bool reverse = false;
    static const std::string emptyFilename;
};

}

namespace std {
    template <> struct hash<sfz::FileId> {
        size_t operator()(const sfz::FileId &id) const;
    };
}

std::ostream &operator<<(std::ostream &os, const sfz::FileId &fileId);
