// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "ParserPrivate.h"

namespace sfz {

Reader::Reader(const fs::path& filePath)
{
    _accum.reserve(256);
    _loc.filePath = std::make_shared<fs::path>(filePath);
    _lineNumColumns.reserve(256);
}

int Reader::getChar()
{
    int byte;

    if (_accum.empty())
        byte = getNextStreamByte();
    else {
        byte = static_cast<unsigned char>(_accum.back());
        _accum.pop_back();
    }

    if (byte != kEof)
        updateSourceLocationAdding(byte);

    return byte;
}

int Reader::peekChar()
{
    int byte;

    if (_accum.empty()) {
        byte = getChar();
        putBackChar(byte);
    }
    else
        byte = static_cast<unsigned char>(_accum.back());

    return byte;
}

bool Reader::extractExactChar(char c)
{
    int next = peekChar();

    if (next == kEof || next != static_cast<unsigned char>(c))
        return false;

    getChar();
    return true;
}

void Reader::putBackChar(int c)
{
    if (c == kEof)
        return;

    char c8bit = static_cast<unsigned char>(c);
    putBackChars(absl::string_view(&c8bit, 1));
}

void Reader::putBackChars(absl::string_view characters)
{
    _accum.append(characters.rbegin(), characters.rend());

    for (size_t i = characters.size(); i-- > 0;)
        updateSourceLocationRemoving(static_cast<unsigned char>(characters[i]));
}

size_t Reader::skipChars(absl::string_view chars)
{
    size_t count = 0;

    while (chars.find(static_cast<unsigned char>(peekChar())) != chars.npos) {
        getChar();
        ++count;
    }

    return count;
}

bool Reader::hasEof()
{
    return peekChar() == kEof;
}

bool Reader::hasOneOfChars(absl::string_view chars)
{
    int c = peekChar();
    if (c == kEof)
        return false;

    return chars.find(static_cast<unsigned char>(c)) != chars.npos;
}

void Reader::updateSourceLocationAdding(int byte)
{
    if (byte != '\n')
        _loc.columnNumber += 1;
    else {
        _lineNumColumns.push_back(_loc.columnNumber);
        _loc.lineNumber += 1;
        _loc.columnNumber = 0;
    }
}

void Reader::updateSourceLocationRemoving(int byte)
{
    if (byte != '\n')
        _loc.columnNumber -= 1;
    else {
        _loc.lineNumber -= 1;
        _loc.columnNumber = _lineNumColumns[_loc.lineNumber];
        _lineNumColumns.pop_back();
    }
}

//------------------------------------------------------------------------------

FileReader::FileReader(const fs::path& filePath)
    : Reader(filePath), _fileStream(filePath)
{
}

bool FileReader::hasError()
{
    return !_fileStream.is_open() || _fileStream.bad();
}

int FileReader::getNextStreamByte()
{
    return _fileStream.get();
}

StringViewReader::StringViewReader(const fs::path& filePath, absl::string_view sfzView)
    : Reader(filePath), _sfzView(sfzView)
{
}

int StringViewReader::getNextStreamByte()
{
    if (position < _sfzView.length())
        return _sfzView[position++];

    return kEof;
}

}  // namespace sfz
