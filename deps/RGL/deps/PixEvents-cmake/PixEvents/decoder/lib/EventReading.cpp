// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#include "pch.h"

#include "EventReading.h"

// We show this string in the PIX UI if we encounter an invalid UTF8 string in a PIX marker
static std::wstring_view InvalidUtf8String = L"<invalid UTF8 string>";

namespace PixEventDecoder
{
    //
    // This helps read through an event buffer in a controlled way. Key things
    // to know about how these buffers are structured that this class enforces:
    //
    // * the buffer is always written to in uint64_t sized chunks
    // * the buffer is always terminated with a PIXEventsBlockEndMarker
    //
    // This is important because:
    // * strings written to a buffer may be truncated (with no null terminator)
    // * truncated strings are terminated with PIXEventsBlockEndMarker
    //
    // So any code that tries to read strings from a buffer needs to be aware of
    // the above points.  This class helps with that.
    //
    class Reader
    {
        uint64_t const* m_begin;
        uint64_t const* m_pos;
        uint64_t const* m_end;

        unsigned m_byteIndex;
    
    public:
        Reader(uint64_t const* begin, uint64_t const* end)
            : m_begin(begin)
            , m_pos(begin)
            , m_end(end)
            , m_byteIndex(0)
        {
            assert(begin != nullptr);
            assert(end != nullptr);
        }

        bool IsAtEnd() const
        {
            if (m_pos >= m_end)
                return true;

            if (*m_pos == PIXEventsBlockEndMarker)
                return true;

            return false;
        }

        uint8_t const* GetPos() const
        {
            return reinterpret_cast<uint8_t const*>(m_pos) + m_byteIndex;
        }

        void SkipBytes(uint64_t bytes)
        {
            m_byteIndex += bytes;
            while (m_byteIndex >= sizeof(uint64_t))
            {
                m_pos++;
                m_byteIndex -= sizeof(uint64_t);
            }
        }

        template<typename T>
        T Read()
        {
            auto value = Peek<T>();
            SkipBytes(sizeof(T));

            return value;
        }

        template<typename T>
        T Peek()
        {
            assert(!IsAtEnd());

            uint8_t const* p = reinterpret_cast<uint8_t const*>(m_pos) + m_byteIndex;
            return *reinterpret_cast<T const*>(p);
        }
        
        uint32_t BytesUsed() const
        {
            return (m_pos - m_begin) * sizeof(uint64_t) + m_byteIndex;
        }
    };

    //this constant serves both as ANSI and UNICODE empty string, least significant byte of UNICODE \0 is ANSI \0
    const wchar_t* EmptyString = L"";

    struct SavedStringInfo
    {
        UINT32 Length; //how many characters are in the source string, does not include terminating zero.
        UINT32 BytesUsed; //how many bytes are used by the source string, includes string info, start and end alignment, and terminating zero.
        bool IsAnsi; //indicates if the string is char or wchar_t
        union //pointer to the actual beginning of the string in the data block, or pointer to EmptyString constant if string was fully truncated.
        {
            const void* RawData;
            const char* AnsiString;
            const wchar_t* UnicodeString;
        };
    };


    template<class T> 
    uint8_t const* FindStringEnd(Reader* r)
    {
        constexpr T NullTerminator = 0;

        while (!r->IsAtEnd() && r->Read<T>() != NullTerminator)
        {
        }

        return r->GetPos();
    }

    SavedStringInfo ReadString(
        _In_reads_to_ptr_(theLimit) const UINT64* theSource,
        _In_ const UINT64* theLimit,
        _In_opt_ const UINT8* pStringMetadata)
    {
        SavedStringInfo readInfo = {};
        readInfo.UnicodeString = const_cast<wchar_t*>(EmptyString); //set string pointer to EmptyString constant in case nothing will be read

        if (theSource == nullptr || theLimit == nullptr || theSource >= theLimit) //nothing to read: invalid function parameters or string is truncated
        {
            return readInfo;
        }

        Reader r(theSource, theLimit);

        uint64_t alignment = 0; //typically stays 0
        uint64_t copyChunkSize = 0;
        bool isShortcut = false; //currently unused
        bool isAnsi = false;
        if (pStringMetadata != nullptr)
        {
            isAnsi = ((*pStringMetadata & PIX_EVENT_METADATA_STRING_IS_ANSI) != 0);
            copyChunkSize = 8;
        }
        else
        {
            uint64_t stringInfo = r.Read<uint64_t>();

            if (stringInfo == 0) //nullptr was passed as a string parameter
            {
                return readInfo;
            }

            if (!PIXDecodeStringInfo(stringInfo, alignment, copyChunkSize, isAnsi, isShortcut) //valid string info when unused bits are 0
                || (copyChunkSize != 8 && copyChunkSize != 16) //string is expected to be written in 8 or 16 byte chunks
                || (alignment >= copyChunkSize)) //alignment must be always less than chunk size
            {
                // As there wasn't a valid string here, we report that we didn't
                // consume any bytes.
                //
                // TODO: this seems suspicious, but this is old behavior - this code
                // might be trying to be robust against bad data, but really it
                // should reject buffers entirely that don't look correct rather
                // than continue trying to parse past errors.
                readInfo.BytesUsed = 0;

                return readInfo;
            }
        }

        if (r.IsAtEnd())
        {
            // there's nothing to read; string was truncated after string info
            return readInfo;
        }

        //alignment is in bytes. set source to the actual beginning of the string
        r.SkipBytes(alignment);

        if (r.IsAtEnd()) //nothing to read: string is truncated
        {
            readInfo.BytesUsed = r.BytesUsed();
            return readInfo;
        }

        uint8_t const* stringBegin = r.GetPos();
        uint8_t const* stringEnd = isAnsi ? FindStringEnd<char>(&r) : FindStringEnd<wchar_t>(&r);

        //in case string ended before the end of the copy chunk, make the size taken by the string to be the next multiple of the chunk size
        auto stringLen = stringEnd - stringBegin;
        if ((stringLen % copyChunkSize) != 0)
        {
            auto toSkip = copyChunkSize - (stringLen % copyChunkSize);
            r.SkipBytes(toSkip);
        }

        size_t characterSize = isAnsi ? sizeof(char) : sizeof(wchar_t);

        // We need to ensure that the string is always null terminated; even
        // though some parts of this code claim otherwise, the formatting part
        // at least has a hard assumption that these strings are terminated.
        if (*stringEnd != 0)
        {
            // If this string isn't null terminated, then stringEnd actually
            // points to valid data that we shouldn't write over. Instead, we
            // null terminate the previous character (thus truncating the string
            // a bit further).
            //
            // (Yes: we're modifying a const buffer.)
            uint8_t const* nullTerminator = stringEnd - characterSize;
            assert(nullTerminator >= stringBegin);

            if (isAnsi)
                *const_cast<char*>(reinterpret_cast<char const*>(nullTerminator)) = '\0';
            else
                *const_cast<wchar_t*>(reinterpret_cast<wchar_t const*>(nullTerminator)) = L'\0';
        }

        // readInfo.Length is the length of the string _not including_ the null terminator
        readInfo.Length = (stringLen - 1) * characterSize;
        readInfo.RawData = stringBegin;
        readInfo.IsAnsi = isAnsi;
        readInfo.BytesUsed = r.BytesUsed();

        assert(readInfo.BytesUsed % sizeof(UINT64) == 0); //has to be an even number of qwords

        return readInfo;
    }

    enum FormatSpecifierDetection
    {
        DoublePercent,
        NotSpecifier,
        NonStringSpecifier,
        StringSpecifier,
        StringSpecifierWithSize,
    };

    template<class T> std::pair<FormatSpecifierDetection, UINT32> IsFormatSpecifier(_In_z_ const T* str)
    {
        for (UINT32 i = 1; ; ++i)
        {
            switch (str[i])
            {
            case 0:
                //i to advance to the terminating zero
                return std::make_pair(FormatSpecifierDetection::NotSpecifier, i);

            case T('%'):
                if (i == 1) //%% case
                {
                    //i + 1 to advance past %%, possibly to a terminating zero
                    return std::make_pair(FormatSpecifierDetection::DoublePercent, i + 1);
                }
                else
                {
                    //i to advance to the found % in not %% case
                    return std::make_pair(FormatSpecifierDetection::NotSpecifier, i);
                }

            case T('A'):
            case T('C'):
            case T('E'):
            case T('F'):
            case T('G'):
            case T('X'):
            case T('a'):
            case T('c'):
            case T('d'):
            case T('e'):
            case T('f'):
            case T('g'):
            case T('i'):
            case T('n'):
            case T('o'):
            case T('p'):
            case T('u'):
            case T('x'):
                //i + 1 to advance past the format specifier, possibly to a terminating zero
                return std::make_pair(FormatSpecifierDetection::NonStringSpecifier, i + 1);

            case T('S'):
            case T('s'):
                //i + 1 to advance past the format specifier, possibly to a terminating zero
                if (str[i - 1] == T('*'))
                {
                    // https://stackoverflow.com/questions/7899119/what-does-s-mean-in-printf
                    return std::make_pair(FormatSpecifierDetection::StringSpecifierWithSize, i + 1);
                }
                else
                {
                    return std::make_pair(FormatSpecifierDetection::StringSpecifier, i + 1);
                }
            }
        }
    }

    template<class T> UINT32 PopulateFormatArguments(
        _Out_writes_(argumentsCount) UINT64* arguments,
        UINT32 argumentsCount,
        _In_z_ T* formatString,
        const UINT64* source,
        const UINT64* limit)
    {
        UINT32 bytesUsed = 0;
        UINT32 argumentIndex = 0;

        T* symbol = formatString;
        while(*symbol && (argumentIndex < argumentsCount))
        {
            if (*symbol != T('%'))
            {
                ++symbol;
            }
            else //if (*symbol == T('%'))
            {
                std::pair<FormatSpecifierDetection, UINT32> formatSpecifier = IsFormatSpecifier(symbol);
                symbol += formatSpecifier.second;
                switch (formatSpecifier.first)
                {
                case FormatSpecifierDetection::NonStringSpecifier:
                    {
                        if (source < limit)
                        {
                            arguments[argumentIndex++] = *source++;
                            bytesUsed += sizeof(UINT64);
                        }
                        else
                        {
                            //for case when an event at the end of the block was truncated
                            //and there was no space to write a format argument, fill it with zero
                            arguments[argumentIndex++] = 0ull;
                        }
                        break;
                    }
                case FormatSpecifierDetection::StringSpecifier:
                case FormatSpecifierDetection::StringSpecifierWithSize:
                    {
                        if (formatSpecifier.first == FormatSpecifierDetection::StringSpecifierWithSize)
                        {
                            if (source < limit)
                            {
                                arguments[argumentIndex++] = *source++;
                                bytesUsed += sizeof(UINT64);
                            }
                            else
                            {
                                arguments[argumentIndex++] = 0ull;
                            }
                        }

                        SavedStringInfo argumentStringInfo = ReadString(source, limit, nullptr);
                        source += argumentStringInfo.BytesUsed / sizeof(UINT64);
                        bytesUsed += argumentStringInfo.BytesUsed;
                        arguments[argumentIndex++] = reinterpret_cast<UINT64>(argumentStringInfo.RawData);
                        break;
                    }
                }
            }
        }

        return bytesUsed;
    }

    //starts reading event at user metadata
    _Use_decl_annotations_
        EventData ReadEndContextEvent(
            const UINT64* source)
    {
        EventData eventData = {};

        // copy out context
        eventData.Context = *source++;
        eventData.TotalBytesUsed += sizeof(UINT64);
        
        return eventData;
    }

    //starts reading event at user metadata
    _Use_decl_annotations_
    EventData ReadEventWithNoFormatParameters(
        UINT64 eventInfo,
        const UINT64* source,
        const UINT64* limit,
        wchar_t* buffer,
        UINT32 bufferLength)
    {
        EventData eventData;

        if (source >= limit)
        {
            return eventData;
        }

        UINT64 timestamp = 0;
        PixOp pixOp = PixOp_Invalid;
        UINT8 eventSize = 0;
        UINT8 eventMetadata = 0;
        PixOp legacyPixOp = PixOp_Invalid;
        PIXDecodeEventInfo(eventInfo, &timestamp, &pixOp, &eventSize, &eventMetadata, &legacyPixOp);

        if ((eventMetadata & PIX_EVENT_METADATA_HAS_COLOR) == PIX_EVENT_METADATA_HAS_COLOR)
        {
            eventData.Metadata = *source++;
            eventData.TotalBytesUsed += sizeof(UINT64);
        }
        else
        {
            if (eventSize > 0)
            {
                eventData.Metadata = PIXDecodeIndexColor(eventMetadata);
            }
        }

        const UINT8* pStringMetadata = (eventSize > 0) ? &eventMetadata : nullptr;
        const bool bIsContextEvent = (eventMetadata & PIX_EVENT_METADATA_ON_CONTEXT) != 0;

        if (bIsContextEvent)
        {
            // copy out context
            eventData.Context = *source++;
            eventData.TotalBytesUsed += sizeof(UINT64);
        }

        SavedStringInfo savedStringInfo = ReadString(source, limit, pStringMetadata);
        UINT32 eventLength = std::min(savedStringInfo.Length, bufferLength);

        if (savedStringInfo.IsAnsi)
        {
            static auto utf8Locale = _create_locale(LC_ALL, ".UTF8"); // We treat all ANSI strings as UTF8

            size_t convertedCharacters = 0;
            errno_t result = _mbstowcs_s_l(&convertedCharacters, buffer, bufferLength, savedStringInfo.AnsiString, savedStringInfo.Length, utf8Locale);
            if (result != 0)
            {
                StringCchPrintfW(buffer, bufferLength, InvalidUtf8String.data());
                eventLength = InvalidUtf8String.size();
            }
            else
            {
                assert(buffer[convertedCharacters - 1] == L'\0');
            }
        }
        else
        {
            errno_t result = wcscpy_s(buffer, bufferLength, savedStringInfo.UnicodeString);
            if (result != 0)
            {
                assert(result == 0);
                return eventData;
            }
        }
        eventData.Length = eventLength;
        eventData.FormatStringBytesUsed = savedStringInfo.BytesUsed;
        eventData.TotalBytesUsed += savedStringInfo.BytesUsed;

        return eventData;
    }

    //starts reading event at user metadata
    _Use_decl_annotations_
    EventData ReadEventWithFormatParameters(
        UINT64 eventInfo,
        const UINT64* source,
        const UINT64* limit,
        wchar_t* unicodeBuffer,
        char* ansiBuffer,
        UINT32 bufferLength,
        UINT64* arguments,
        UINT32* pArgumentsCount)
    {
        static auto utf8Locale = _create_locale(LC_ALL, ".UTF8"); // We treat all ANSI strings as UTF8

        EventData eventData;

        if (pArgumentsCount != nullptr)
        {
            *pArgumentsCount = 0;
        }

        if (source >= limit)
        {
            return eventData;
        }

        UINT64 timestamp = 0;
        PixOp pixOp = PixOp_Invalid;
        UINT8 eventSize = 0;
        UINT8 eventMetadata = 0;
        PixOp legacyOpcode = PixOp_Invalid;
        PIXDecodeEventInfo(eventInfo, &timestamp, &pixOp, &eventSize, &eventMetadata, &legacyOpcode);

        if ((eventMetadata & PIX_EVENT_METADATA_HAS_COLOR) == PIX_EVENT_METADATA_HAS_COLOR)
        {
            eventData.Metadata = *source++;
            eventData.TotalBytesUsed += sizeof(UINT64);
        }
        else
        {
            if (eventSize > 0)
            {
                eventData.Metadata = PIXDecodeIndexColor(eventMetadata);
            }
        }

        const UINT8* pStringMetadata = (eventSize > 0) ? &eventMetadata : nullptr;
        const bool bIsContextEvent = (eventMetadata & PIX_EVENT_METADATA_ON_CONTEXT) != 0;

        if (bIsContextEvent)
        {
            eventData.Context = *source++;
            eventData.TotalBytesUsed += sizeof(UINT64);
        }

        UINT64 localArguments[PIX_MAX_ARGUMENTS];
        UINT64* pArguments = (arguments == nullptr) ? localArguments : arguments;
        memset(pArguments, 0, sizeof(UINT64) * PIX_MAX_ARGUMENTS);

        SavedStringInfo formatStringInfo = ReadString(source, limit, pStringMetadata);
        source += formatStringInfo.BytesUsed / sizeof(UINT64);
        eventData.FormatStringBytesUsed = formatStringInfo.BytesUsed;
        eventData.TotalBytesUsed += formatStringInfo.BytesUsed;

        const UINT64 eventSizeUsed = 1 + eventData.TotalBytesUsed / sizeof(UINT64);

        // For V2 we know the size of the event so we can update the limit
        if (eventSize > 0 && eventSize < PIXEventsSizeMax)
        {
            assert(eventSize >= eventSizeUsed);
            limit = source + (eventSize - eventSizeUsed);
        }

        // For V2 events, we're using var args path if there's more room in the event header
        // For V1 events, this was already established
        const bool noVarArgsEvent = (legacyOpcode == PixOp_Invalid) ?
            (eventSizeUsed == eventSize) : false;

        if (noVarArgsEvent)
        {
            if (formatStringInfo.IsAnsi)
            {
                size_t convertedCharacters = 0;
                errno_t result = _mbstowcs_s_l(&convertedCharacters, unicodeBuffer, bufferLength, formatStringInfo.AnsiString, formatStringInfo.Length, utf8Locale);
                if (result != 0)
                {
                    StringCchPrintfW(unicodeBuffer, bufferLength, InvalidUtf8String.data());
                    eventData.Length = InvalidUtf8String.size();
                    return eventData;
                }
                else
                {
                    assert(unicodeBuffer[convertedCharacters - 1] == L'\0');
                }
            }
            else
            {
                errno_t result = wcscpy_s(unicodeBuffer, bufferLength, formatStringInfo.UnicodeString);
                if (result != 0)
                {
                    assert(result == 0);
                    return eventData;
                }
            }
        }
        else
        {
            if (formatStringInfo.IsAnsi)
            {
                const UINT32 totalBytesUsed = PopulateFormatArguments(pArguments, PIX_MAX_ARGUMENTS, formatStringInfo.AnsiString, source, limit);
                if (pArgumentsCount != nullptr)
                {
                    *pArgumentsCount = totalBytesUsed / sizeof(UINT64);
                }
                eventData.TotalBytesUsed += totalBytesUsed;

                HRESULT hr = StringCchPrintfA(ansiBuffer, bufferLength, formatStringInfo.AnsiString,
                    pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7],
                    pArguments[8], pArguments[9], pArguments[10], pArguments[11], pArguments[12], pArguments[13], pArguments[14], pArguments[15]);
                if(FAILED(hr))
                {
                    // return without string
                    assert(SUCCEEDED(hr));
                    return eventData;
                }

                size_t convertedCharacters = 0;
                errno_t result = _mbstowcs_s_l(&convertedCharacters, unicodeBuffer, bufferLength, ansiBuffer, strnlen_s(ansiBuffer, bufferLength), utf8Locale);
                if (result != 0)
                {
                    StringCchPrintfW(unicodeBuffer, bufferLength, InvalidUtf8String.data());
                    eventData.Length = InvalidUtf8String.size();
                }
                else
                {
                    assert(unicodeBuffer[convertedCharacters - 1] == L'\0');
                    eventData.Length = static_cast<UINT32>(wcsnlen_s(unicodeBuffer, bufferLength));
                }
            }
            else
            {
                const UINT32 totalBytesUsed = PopulateFormatArguments(pArguments, PIX_MAX_ARGUMENTS, formatStringInfo.UnicodeString, source, limit);
                if (pArgumentsCount != nullptr)
                {
                    *pArgumentsCount = totalBytesUsed / sizeof(UINT64);
                }
                eventData.TotalBytesUsed += totalBytesUsed;

                HRESULT hr = StringCchPrintfW(unicodeBuffer, bufferLength, formatStringInfo.UnicodeString,
                    pArguments[0], pArguments[1], pArguments[2], pArguments[3], pArguments[4], pArguments[5], pArguments[6], pArguments[7],
                    pArguments[8], pArguments[9], pArguments[10], pArguments[11], pArguments[12], pArguments[13], pArguments[14], pArguments[15]);
                if (FAILED(hr))
                {
                    // return without string
                    assert(SUCCEEDED(hr));
                    return eventData;
                }
                eventData.Length = static_cast<UINT32>(wcsnlen_s(unicodeBuffer, bufferLength));
            }
        }

        return eventData;
    }
}
