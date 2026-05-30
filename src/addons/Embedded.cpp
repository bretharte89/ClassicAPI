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

// Embedded `!!!ClassicAPI` addon fallback. When the user doesn't have
// the addon installed on disk, this module makes the engine think it
// does — without writing anything to the filesystem.
//
// How:
//
//   1. The CMake build embeds every file under `AddOns/!!!ClassicAPI/`
//      into a generated header (`embedded_classicapi.h`) as a byte
//      array per file plus a `{path, data, size}` manifest.
//
//   2. We hook the engine's file-read function at `FUN_FILE_READ`.
//      When the engine asks for a path under
//      `Interface\AddOns\!!!ClassicAPI\`, we allocate a Storm buffer
//      via the same allocator the engine itself uses, copy the
//      embedded content in, and hand it back. The caller's normal
//      `SMemFree` reclaims it on completion — no special lifetime.
//
//   3. We post-hook the engine's `FUN_ADDON_INIT`. After the engine's
//      own `Interface\AddOns\` scan finishes, we call the TOC parser
//      directly with `"!!!ClassicAPI"`. The parser opens its TOC via
//      our hooked read function and registers the addon as a normal
//      entry. The TOC parser is dedup-safe: if the user already has
//      the addon on disk, the engine's scan picked it up, the
//      hash-table lookup hits, and our call is a no-op. So the
//      conditional "only if not on disk" behavior is free.
//
// Same pattern WeirdUtils.dll uses (we reverse-engineered it from
// `0x10015A74` while figuring this out). The addon load pass then
// reads each `.lua` file through the same hooked read path —
// `GlobalStrings.lua`, `Util\Print.lua`, etc. all surface from
// embedded buffers.

#include "embedded_classicapi.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstring>

namespace Addons::Embedded {

namespace {

constexpr const char *kAddonName = "!!!ClassicAPI";

// `Interface\AddOns\!!!ClassicAPI\` — the prefix the engine
// constructs for any of our addon's files. Comparing case-
// insensitively because the TOC parser uses `Interface\AddOns\%s\…`
// where %s comes from the on-disk directory name, and Windows file
// paths are case-insensitive.
constexpr const char *kAddonPathPrefix = "Interface\\AddOns\\!!!ClassicAPI\\";
constexpr size_t kAddonPathPrefixLen = 30; // length of the literal above

char NormalizeChar(char c) {
    if (c == '/') return '\\';
    if (c >= 'A' && c <= 'Z') return static_cast<char>(c + 32);
    return c;
}

bool PathEqualsCI(const char *a, const char *b) {
    while (*a && *b) {
        if (NormalizeChar(*a) != NormalizeChar(*b)) return false;
        ++a; ++b;
    }
    return *a == *b;
}

// Strip the `Interface\AddOns\!!!ClassicAPI\` prefix and return the
// suffix (e.g. `Util\Print.lua`) on a match, NULL otherwise.
const char *StripAddonPrefix(const char *path) {
    if (path == nullptr) return nullptr;
    const char *p = path;
    const char *q = kAddonPathPrefix;
    while (*q) {
        if (*p == '\0') return nullptr;
        if (NormalizeChar(*p) != NormalizeChar(*q)) return nullptr;
        ++p; ++q;
    }
    return p;
}

const ClassicAPIFiles::File *LookupEmbedded(const char *suffix) {
    for (size_t i = 0; i < ClassicAPIFiles::kFileCount; ++i) {
        if (PathEqualsCI(suffix, ClassicAPIFiles::kFiles[i].path))
            return &ClassicAPIFiles::kFiles[i];
    }
    return nullptr;
}

// Storm allocator — same one `FUN_FILE_READ` uses internally. Buffer
// allocated here is freed cleanly by the caller's standard `SMemFree`
// (`FUN_STORM_SMEM_FREE`). `__stdcall` per the function's `RET 0x10`
// (4 args × 4 bytes = 16) epilogue.
//   __stdcall void *SMemAlloc(size_t size, const char *file, int line, int flags)
using SMemAlloc_t = void *(__stdcall *)(size_t size, const char *file, int line, int flags);

// `FUN_FILE_READ` — same shape derived from the Octo decompile.
// Calling convention is `__stdcall` (callee cleans 28 bytes via
// `RET 0x1C`), not `__cdecl` — confirmed via the function epilogue.
// Getting this wrong silently corrupts the caller's stack frame.
//   __stdcall int FileRead(int unused, const char *path, void **outBuf,
//                          size_t *outSize, size_t extraBytes,
//                          int flag1, int flag2)
using FileRead_t = int(__stdcall *)(int unused, const char *path, void **outBuf,
                                    size_t *outSize, size_t extraBytes,
                                    int flag1, int flag2);
FileRead_t FileRead_o = nullptr;

int __stdcall FileRead_h(int unused, const char *path, void **outBuf,
                         size_t *outSize, size_t extraBytes,
                         int flag1, int flag2) {
    const char *suffix = StripAddonPrefix(path);
    if (suffix == nullptr) {
        // Path isn't under `Interface\AddOns\!!!ClassicAPI\` — straight
        // pass-through, we don't care.
        return FileRead_o(unused, path, outBuf, outSize, extraBytes,
                          flag1, flag2);
    }

    // **Disk wins.** Always try the original (disk) read first. If the
    // user has the addon installed on disk, the engine's normal scan
    // picks it up and registers it; if we returned our embedded
    // content here instead, we'd replace the disk content with the
    // embedded version — defeating the "power users edit on disk"
    // promise. Only fall back to embedded if the disk read fails
    // (file doesn't exist), which is exactly the case where the
    // embedded version should kick in.
    const int diskResult = FileRead_o(unused, path, outBuf, outSize,
                                       extraBytes, flag1, flag2);
    if (diskResult != 0) return diskResult;

    const auto *entry = LookupEmbedded(suffix);
    if (entry == nullptr) return 0;

    auto SMemAlloc = reinterpret_cast<SMemAlloc_t>(
        Offsets::FUN_STORM_SMEM_ALLOC);
    const size_t totalSize = entry->size + extraBytes;
    void *buf = SMemAlloc(totalSize, __FILE__, __LINE__, 0);
    if (buf == nullptr) return 0;
    std::memcpy(buf, entry->data, entry->size);
    if (extraBytes > 0) {
        std::memset(static_cast<uint8_t *>(buf) + entry->size, 0, extraBytes);
    }
    if (outBuf != nullptr) *outBuf = buf;
    if (outSize != nullptr) *outSize = entry->size;
    return 1;
}

// `FUN_ADDON_INIT` — `__fastcall(accountName)`. We post-hook it: let
// the engine's normal `Interface\AddOns\` scan run first, then call
// the TOC parser with `"!!!ClassicAPI"`. Dedup-safe — see file header.
using AddonInit_t = void(__fastcall *)(const char *accountName);
AddonInit_t AddonInit_o = nullptr;

using TocParser_t = void(__fastcall *)(const char *name);

void __fastcall AddonInit_h(const char *accountName) {
    AddonInit_o(accountName);
    auto TocParser = reinterpret_cast<TocParser_t>(Offsets::FUN_TOC_PARSER);
    TocParser(kAddonName);
}

const Game::HookAutoRegister _hookFileRead{
    Offsets::FUN_FILE_READ,
    reinterpret_cast<void *>(&FileRead_h),
    reinterpret_cast<void **>(&FileRead_o)};

const Game::HookAutoRegister _hookAddonInit{
    Offsets::FUN_ADDON_INIT,
    reinterpret_cast<void *>(&AddonInit_h),
    reinterpret_cast<void **>(&AddonInit_o)};

} // namespace

} // namespace Addons::Embedded
