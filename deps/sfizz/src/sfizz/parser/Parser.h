// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include "../Opcode.h"
#include <ghc/fs_std.hpp>
#include <absl/types/optional.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <string>
#include <memory>

namespace sfz {

class Reader;
class ParserListener;
struct SourceLocation;
struct SourceRange;

/**
 * @brief Context-dependent parser for SFZ files
 */
class Parser {
public:
    Parser();
    ~Parser();

    void clear();

    void addExternalDefinition(absl::string_view id, absl::string_view value);
    void clearExternalDefinitions();

    void parseFile(const fs::path& path);
    void parseString(const fs::path& path, absl::string_view sfzView);
    void parseVirtualFile(const fs::path& path, std::unique_ptr<Reader> reader);

    void setRecursiveIncludeGuardEnabled(bool en) { _recursiveIncludeGuardEnabled = en; }
    void setMaximumIncludeDepth(size_t depth) { _maxIncludeDepth = depth; }

    const fs::path& originalDirectory() const noexcept { return _originalDirectory; }

    typedef absl::flat_hash_set<std::string> IncludeFileSet;
    typedef absl::flat_hash_map<std::string, std::string> DefinitionSet;

    const IncludeFileSet& getIncludedFiles() const noexcept { return _pathsIncluded; }
    const DefinitionSet& getDefines() const noexcept { return _currentDefinitions; }

    size_t getErrorCount() const noexcept { return _errorCount; }
    size_t getWarningCount() const noexcept { return _warningCount; }

    using Listener = ParserListener;

    void setListener(Listener* listener) noexcept { _listener = listener; }

private:
    void includeNewFile(const fs::path& path, std::unique_ptr<Reader> reader, const SourceRange& includeStmtRange);
    void addDefinition(absl::string_view id, absl::string_view value);
    void processTopLevel();
    void processDirective();
    void processHeader();
    void processOpcode();

    // errors and warnings
    void emitError(const SourceRange& range, const std::string& message);
    void emitWarning(const SourceRange& range, const std::string& message);

    // recover after error
    void recover();

    // state handling
    void flushCurrentHeader();

    // helpers
    enum class CommentType {
        None,
        Line,
        Block,
    };

    static CommentType getCommentType(Reader& reader);
    size_t skipComment();
    static void trimRight(std::string& text);
    static size_t extractToEol(Reader& reader, std::string* dst); // ignores comment
    std::string expandDollarVars(const SourceRange& range, absl::string_view src);

    // predicates
    static bool isIdentifierChar(char c);
    static bool isSpaceChar(char c);
    static bool isIdentifier(absl::string_view s);

private:
    Listener* _listener = nullptr;

    fs::path _originalDirectory { fs::current_path() };
    DefinitionSet _externalDefinitions;

    // a current list of files included, last one at the back
    std::vector<std::unique_ptr<Reader>> _included;

    // recursive include guard
    size_t _maxIncludeDepth = 32;
    bool _recursiveIncludeGuardEnabled = false;
    IncludeFileSet _pathsIncluded;
    DefinitionSet _currentDefinitions;

    // parsing state
    absl::optional<std::string> _currentHeader;
    std::vector<Opcode> _currentOpcodes;

    // errors and warnings
    size_t _errorCount = 0;
    size_t _warningCount = 0;
};

/**
 * @brief Source file location for errors and warnings.
 */
struct SourceLocation {
    std::shared_ptr<fs::path> filePath;
    size_t lineNumber = 0;
    size_t columnNumber = 0;
    explicit operator bool() const noexcept { return filePath != nullptr; }
};

/**
 * @brief Range of source file.
 */
struct SourceRange {
    SourceLocation start;
    SourceLocation end;
    explicit operator bool() const noexcept { return bool(start) && bool(end); }
};

} // namespace sfz
