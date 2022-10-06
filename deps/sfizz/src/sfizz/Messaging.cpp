// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "Messaging.h"
#include <absl/strings/string_view.h>
#include <algorithm>
#include <type_traits>
#include <cstring>

// ensure that `sfizz_arg_t` has the same storage characteristics as `int64_t`
// Note(jpc) alignment checks fail on old gcc i386
static_assert(
    sizeof(sfizz_arg_t) == sizeof(int64_t) /* &&
    alignof(sfizz_arg_t) == alignof(int64_t) */,
    "The ABI stability check has failed.");

template <class T>
static T paddingSize(T count, unsigned align) {
    unsigned mask = align - 1;
    return (align - (count & mask)) & mask;
};

///
class OSCWriter {
public:
    void setOutputBuffer(void* buffer, uint32_t capacity);
    uint32_t writeMessage(const char* path, const char* sig, const sfizz_arg_t* args);

private:
    uint32_t appendBytes(const void* src, uint32_t count);
    uint32_t appendZeros(uint32_t count);
    template <class T> uint32_t appendInteger(T integer);
    uint32_t appendFloat(float f);
    uint32_t appendDouble(float d);

private:
    uint8_t* dstBuffer_ = nullptr;
    uint32_t dstCapacity_ = 0;
};

class OSCReader {
public:
    void setInputBuffer(const void* buffer, uint32_t capacity);
    void setAllocationBuffer(void* buffer, uint32_t capacity);
    int32_t extractMessage(const char** outPath, const char** outSig, const sfizz_arg_t** outArgs);

private:
    template <class T> T* allocate(uint32_t count);
    bool extractString(const char*& outStr, uint32_t& outLen);
    template <class T> bool extractInteger(T& outValue);
    bool extractFloat(float& f);
    bool extractDouble(double& d);

private:
    const uint8_t* srcBuffer_ = nullptr;
    uint32_t srcCapacity_ = 0;
    uint8_t* allocBuffer_ = nullptr;
    uint32_t allocCapacity_ = 0;
};

///
extern "C" uint32_t sfizz_prepare_message(
    void* buffer, uint32_t capacity,
    const char* path, const char* sig, const sfizz_arg_t* args)
{
    OSCWriter writer;
    writer.setOutputBuffer(buffer, capacity);
    return writer.writeMessage(path, sig, args);
}

extern "C" int32_t sfizz_extract_message(
    const void* srcBuffer, uint32_t srcCapacity,
    void* argsBuffer, uint32_t argsCapacity,
    const char** outPath, const char** outSig, const sfizz_arg_t** outArgs)
{
    OSCReader reader;
    reader.setInputBuffer(srcBuffer, srcCapacity);
    reader.setAllocationBuffer(argsBuffer, argsCapacity);
    return reader.extractMessage(outPath, outSig, outArgs);
}

///
void OSCWriter::setOutputBuffer(void* buffer, uint32_t capacity)
{
    dstBuffer_ = reinterpret_cast<uint8_t*>(buffer);
    dstCapacity_ = capacity;
}

uint32_t OSCWriter::writeMessage(const char* path, const char* sig, const sfizz_arg_t* args)
{
    uint32_t msglen = 0;

    // write path, null byte, and 4byte padding
    uint32_t pathlen = static_cast<uint32_t>(strlen(path));
    msglen += appendBytes(path, pathlen + 1);
    msglen += appendZeros(paddingSize(pathlen + 1, 4));

    // write comma, signature, null byte, and 4byte padding
    uint32_t siglen = static_cast<uint32_t>(strlen(sig));
    msglen += appendBytes(",", 1);
    msglen += appendBytes(sig, siglen + 1);
    msglen += appendZeros(paddingSize(siglen + 2, 4));

    // write arguments
    for (uint32_t i = 0; i < siglen; ++i) {
        switch (sig[i]) {
        default:
            return 0;
        case 'i':
        case 'c':
        case 'r':
            msglen += appendInteger(args[i].i);
            break;
        case 'm':
            msglen += appendBytes(args[i].m, 4);
            break;
        case 'h':
            msglen += appendInteger(args[i].h);
            break;
        case 'f':
            msglen += appendFloat(args[i].f);
            break;
        case 'd':
            msglen += appendDouble(args[i].d);
            break;
        case 's':
        case 'S':
            {
                size_t len = strlen(args[i].s);
                msglen += appendBytes(args[i].s, len + 1);
                msglen += appendZeros(paddingSize(len + 1, 4));
            }
            break;
        case 'b':
            {
                msglen += appendInteger(args[i].b->size);
                msglen += appendBytes(args[i].b->data, args[i].b->size);
                msglen += appendZeros(paddingSize(args[i].b->size, 4));
            }
            break;
        case 'T':
        case 'F':
        case 'N':
        case 'I':
            break;
        }
    }

    return msglen;
}

uint32_t OSCWriter::appendBytes(const void* src, uint32_t count)
{
    uint32_t written = std::min(dstCapacity_, count);
    memcpy(dstBuffer_, src, written);
    dstBuffer_ += written;
    dstCapacity_ -= written;
    return count;
}

uint32_t OSCWriter::appendZeros(uint32_t count)
{
    uint32_t written = std::min(dstCapacity_, count);
    memset(dstBuffer_, '\0', written);
    dstBuffer_ += written;
    dstCapacity_ -= written;
    return count;
}

template <class T> uint32_t OSCWriter::appendInteger(T integer)
{
    using U = typename std::make_unsigned<T>::type;
    const U uinteger = static_cast<U>(integer);
    uint8_t data[sizeof(U)];
    for (unsigned i = 0; i < sizeof(U); ++i) {
        unsigned sh = 8 * (sizeof(U) - 1 - i);
        data[i] = (uint8_t)((uinteger >> sh) & 0xff);
    }
    return appendBytes(data, sizeof(U));
}

uint32_t OSCWriter::appendFloat(float f)
{
    union { float f; uint32_t i; } u;
    u.f = f;
    return appendInteger(u.i);
}

uint32_t OSCWriter::appendDouble(float d)
{
    union { double d; uint64_t i; } u;
    u.d = d;
    return appendInteger(u.i);
}

///
void OSCReader::setInputBuffer(const void* buffer, uint32_t capacity)
{
    srcBuffer_ = reinterpret_cast<const uint8_t*>(buffer);
    srcCapacity_ = capacity;
}

void OSCReader::setAllocationBuffer(void* buffer, uint32_t capacity)
{
    allocBuffer_ = reinterpret_cast<uint8_t*>(buffer);
    allocCapacity_ = capacity;
}

int32_t OSCReader::extractMessage(const char** outPath, const char** outSig, const sfizz_arg_t** outArgs)
{
    const uint8_t* const srcStart = srcBuffer_;

    // read path, null byte
    const char* path;
    uint32_t pathlen;
    if (!extractString(path, pathlen))
        return 0;
    if (outPath)
        *outPath = path;

    // read signature, null byte
    const char* sig;
    uint32_t siglen;
    if (!extractString(sig, siglen) || sig[0] != ',')
        return 0;
    ++sig;
    --siglen;
    if (outSig)
        *outSig = sig;

    // read arguments
    sfizz_arg_t* args = allocate<sfizz_arg_t>(siglen);
    if (!args)
        return -1;
    if (outArgs)
        *outArgs = args;

    for (uint32_t i = 0, n = siglen; i < n; ++i) {
        switch (sig[i]) {
        default:
            return 0;
        case 'i':
        case 'c':
        case 'r':
            if (!extractInteger(args[i].i))
                return 0;
            break;
        case 'm':
            if (srcCapacity_ < 4)
                return 0;
            memcpy(args[i].m, srcBuffer_, 4);
            srcBuffer_ += 4;
            srcCapacity_ -= 4;
            break;
        case 'h':
            if (!extractInteger(args[i].h))
                return 0;
            break;
        case 'f':
            if (!extractFloat(args[i].f))
                return 0;
            break;
        case 'd':
            if (!extractDouble(args[i].d))
                return 0;
            break;
        case 's':
        case 'S':
            {
                const char* str;
                uint32_t len;
                if (!extractString(str, len))
                    return 0;
                args[i].s = str;
            }
            break;
        case 'b':
            {
                sfizz_blob_t* blob = allocate<sfizz_blob_t>(1);
                if (!blob)
                    return -1;
                args[i].b = blob;
                uint32_t len = blob->size;
                if (!extractInteger(len))
                    return 0;
                uint32_t padlen = len + paddingSize(len, 4);
                if (srcCapacity_ < padlen)
                    return 0;
                blob->data = srcBuffer_;
                blob->size = len;
                srcBuffer_ += padlen;
                srcCapacity_ -= padlen;
            }
            break;
        case 'T':
        case 'F':
        case 'N':
        case 'I':
            break;
        }
    }

    return srcBuffer_ - srcStart;
}

template <class T> T* OSCReader::allocate(uint32_t count)
{
    uintptr_t pad = paddingSize(reinterpret_cast<uintptr_t>(allocBuffer_), alignof(T));
    uint32_t size = count * sizeof(T);
    if (allocCapacity_ < pad + size)
        return nullptr;
    void* ptr = allocBuffer_ + pad;
    allocBuffer_ += pad + size;
    allocCapacity_ -= pad + size;
    return reinterpret_cast<T *>(ptr);
}

bool OSCReader::extractString(const char*& outStr, uint32_t& outLen)
{
    const char* str = reinterpret_cast<const char*>(srcBuffer_);
    uint32_t len = static_cast<uint32_t>(strnlen(str, srcCapacity_));
    if (len == srcCapacity_)
        return false;
    uint32_t padlen = len + 1 + paddingSize(len + 1, 4);
    if (padlen > srcCapacity_)
        return false;
    srcBuffer_ += padlen;
    srcCapacity_ -= padlen;
    outStr = str;
    outLen = len;
    return true;
}

template <class T> bool OSCReader::extractInteger(T& outValue)
{
    if (srcCapacity_ < sizeof(T))
        return false;
    using U = typename std::make_unsigned<T>::type;
    U value = 0;
    const uint8_t* src = reinterpret_cast<const uint8_t*>(srcBuffer_);
    for (unsigned i = 0; i < sizeof(T); ++i)
        value = (value << 8) | src[i];
    srcBuffer_ += sizeof(T);
    srcCapacity_ -= sizeof(T);
    outValue = static_cast<T>(value);
    return true;
};

bool OSCReader::extractFloat(float& f)
{
    union { float f; uint32_t i; } u;
    if (!extractInteger(u.i))
        return false;
    f = u.f;
    return true;
}

bool OSCReader::extractDouble(double& d)
{
    union { double d; uint64_t i; } u;
    if (!extractInteger(u.i))
        return false;
    d = u.d;
    return true;
}
