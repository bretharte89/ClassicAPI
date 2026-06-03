// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

// Inject ClassicAPI-owned bindings into the engine's TARGETING group.
//
// Why a file-read hook rather than an addon-side Bindings.xml: vanilla
// stores every binding in a single linearly-indexed table, and the
// keybind UI walks that table in index order with `HEADER_*` pseudo-
// entries splitting groups. Addon Bindings.xml files get parsed
// AFTER FrameXML, so even with `header="TARGETING"` an addon binding
// receives a display index higher than every engine binding — it
// renders orphaned at the very bottom of the keybind list rather than
// under TARGETING.
//
// To land inside TARGETING, the binding's display index has to fall
// between HEADER_TARGETING and HEADER_INTERFACE. The cleanest way to
// arrange that is to splice the bindings into the FrameXML file the
// engine itself parses, so they take their indices from the same
// running counter every native binding does.
//
// Splice site: just before `<Binding name="TOGGLECHARACTER0"` (the
// first INTERFACE-section entry). Our injected bindings inherit
// TARGETING from the preceding `header="TARGETING"` declaration on
// `TARGETNEARESTENEMY` (the engine never re-asserts a header on each
// binding — group membership is positional). They appear at the tail
// of TARGETING, just after PETATTACK / ATTACKTARGET.
//
// The engine's binding parser is `FUN_004b6f70`. It calls
// `FUN_FILE_READ` (`0x00648620`) to load the file, then walks `<Binding>`
// elements assigning each the next display index from a running
// counter. By the time our injected bindings are processed, the
// engine has already emitted HEADER_TARGETING and a sequence of
// TARGETING entries; ours append to that sequence and HEADER_INTERFACE
// follows.

#include "Inject.h"
#include "Offsets.h"

#include "embedded_bindings.h"

#include <cstring>

namespace Bindings::Inject {

namespace {

constexpr const char *kFrameXMLBindingsPath = "Interface\\FrameXML\\Bindings.xml";

// First INTERFACE-section binding. Splicing immediately before this
// puts our entries at the tail of the TARGETING block.
constexpr const char *kSpliceMarker = "<Binding name=\"TOGGLECHARACTER0\"";

// Extracts the inner content between the outer `<Bindings>` /
// `</Bindings>` tags of our embedded XML fragment. The wrapper exists
// so the source file (`src/bindings/Bindings.xml`) is a well-formed
// XML document on its own; at splice time only the inner elements
// belong in the engine's binding list. Resolved once and cached.
//
// Returns {nullptr, 0} if the embedded buffer doesn't look like a
// `<Bindings>...</Bindings>` document — defensive, this shouldn't
// happen with a build-time-validated source.
struct InnerXml {
    const char *data;
    size_t size;
};

// Advances `p` past any `<!-- ... -->` comment block starting at the
// current position. Returns the new position (past the closing `-->`)
// if a comment was skipped, or `p` unchanged if no comment was at
// the current position. Caps at `end` if the comment is unterminated.
const char *SkipCommentIfAny(const char *p, const char *end) {
    if (p + 4 > end) return p;
    if (std::memcmp(p, "<!--", 4) != 0) return p;
    p += 4;
    while (p + 3 <= end) {
        if (std::memcmp(p, "-->", 3) == 0)
            return p + 3;
        ++p;
    }
    return end;
}

// Linear scan for `marker` in `[start, end)`, skipping any `<!--...-->`
// comment blocks (where unrelated occurrences of XML tags might live).
const char *FindOutsideComment(const char *start, const char *end, const char *marker) {
    const size_t markerLen = std::strlen(marker);
    const char *p = start;
    while (p + markerLen <= end) {
        const char *afterComment = SkipCommentIfAny(p, end);
        if (afterComment != p) {
            p = afterComment;
            continue;
        }
        if (std::memcmp(p, marker, markerLen) == 0)
            return p;
        ++p;
    }
    return nullptr;
}

InnerXml ResolveInnerXml() {
    const char *content = reinterpret_cast<const char *>(kEmbeddedBindingsXml);
    const char *end = content + kEmbeddedBindingsXmlSize;

    // Locate `<Bindings>` opening tag (skipping any comment blocks
    // that mention the tag literally for documentation purposes).
    const char *open = FindOutsideComment(content, end, "<Bindings");
    if (open == nullptr) return {nullptr, 0};

    // Skip to the closing `>` of the opening tag.
    const char *innerStart = open;
    while (innerStart < end && *innerStart != '>') ++innerStart;
    if (innerStart == end) return {nullptr, 0};
    ++innerStart;

    const char *close = FindOutsideComment(innerStart, end, "</Bindings>");
    if (close == nullptr || close < innerStart) return {nullptr, 0};

    return {innerStart, static_cast<size_t>(close - innerStart)};
}

char NormalizeChar(char c) {
    if (c == '/') return '\\';
    if (c >= 'A' && c <= 'Z') return static_cast<char>(c + 32);
    return c;
}

bool PathEqualsCI(const char *a, const char *b) {
    while (*a != '\0' && *b != '\0') {
        if (NormalizeChar(*a) != NormalizeChar(*b)) return false;
        ++a; ++b;
    }
    return *a == *b;
}

using SMemAlloc_t = void *(__stdcall *)(size_t size, const char *file, int line, int flags);
using SMemFree_t = void(__stdcall *)(void *buf, const char *file, int line, int flags);

const char *FindMarker(const char *content, size_t size, const char *marker) {
    const size_t markerLen = std::strlen(marker);
    if (size < markerLen) return nullptr;
    for (size_t i = 0; i + markerLen <= size; ++i) {
        if (std::memcmp(content + i, marker, markerLen) == 0)
            return content + i;
    }
    return nullptr;
}

} // namespace

bool TryHandle(int unused, const char *path, void **outBuf, size_t *outSize,
               size_t extraBytes, int flag1, int flag2, FileReadFn original) {
    if (path == nullptr || !PathEqualsCI(path, kFrameXMLBindingsPath))
        return false;

    // Pull the real file through the original reader with the caller's
    // requested `extraBytes` honored — the original allocates a Storm
    // buffer of `fileSize + extraBytes` and zero-fills the tail.
    void *origBuf = nullptr;
    size_t origSize = 0;
    const int readOk = original(unused, path, &origBuf, &origSize,
                                 extraBytes, flag1, flag2);
    if (readOk == 0 || origBuf == nullptr)
        return false;

    const char *content = static_cast<const char *>(origBuf);
    const char *splice = FindMarker(content, origSize, kSpliceMarker);
    if (splice == nullptr) {
        // Marker missing — likely a modified or non-vanilla FrameXML
        // (Turtle patch level, custom build). Pass the original
        // through unchanged rather than corrupting it.
        if (outBuf != nullptr) *outBuf = origBuf;
        if (outSize != nullptr) *outSize = origSize;
        return true;
    }

    static const InnerXml inner = ResolveInnerXml();
    if (inner.data == nullptr || inner.size == 0) {
        if (outBuf != nullptr) *outBuf = origBuf;
        if (outSize != nullptr) *outSize = origSize;
        return true;
    }

    const size_t splicePos = static_cast<size_t>(splice - content);
    const size_t newSize = origSize + inner.size;

    auto SMemAlloc = reinterpret_cast<SMemAlloc_t>(Offsets::FUN_STORM_SMEM_ALLOC);
    auto SMemFree = reinterpret_cast<SMemFree_t>(Offsets::FUN_STORM_SMEM_FREE);

    void *newBuf = SMemAlloc(newSize + extraBytes, __FILE__, __LINE__, 0);
    if (newBuf == nullptr) {
        // Allocation failed — fall back to the original buffer so the
        // engine still loads its own bindings.
        if (outBuf != nullptr) *outBuf = origBuf;
        if (outSize != nullptr) *outSize = origSize;
        return true;
    }

    auto *dest = static_cast<char *>(newBuf);
    std::memcpy(dest, content, splicePos);
    std::memcpy(dest + splicePos, inner.data, inner.size);
    std::memcpy(dest + splicePos + inner.size,
                content + splicePos, origSize - splicePos);
    if (extraBytes > 0)
        std::memset(dest + newSize, 0, extraBytes);

    SMemFree(origBuf, __FILE__, __LINE__, 0);

    if (outBuf != nullptr) *outBuf = newBuf;
    if (outSize != nullptr) *outSize = newSize;
    return true;
}

} // namespace Bindings::Inject
