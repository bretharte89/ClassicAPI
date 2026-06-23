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

// `ExportInterfaceFiles art|code` — backport of the 4.3.4 developer-
// console command of the same name. Registered as a real console
// command (the `~` console available when the client is launched with
// `-console`), exactly like retail. Dumps Blizzard's UI files out of
// the mounted MPQ archives onto disk:
//
//   code → .lua / .xml / .toc / .xsd  →  BlizzardInterfaceCode\...
//   art  → .blp / .tga                →  BlizzardInterfaceArt\...
//
// `ExportDBCFiles` — ClassicAPI-original companion that dumps every
// `.dbc` table the client loads to `DBFilesClient\`. It unions the MPQ
// `(listfile)` with a scan of `.text` for the DBC path-getter pattern
// (`mov eax, &"DBFilesClient\X.dbc"; ret`) — the latter is the
// authoritative "what does this build load" list, since the master DBC
// init is straight-line (no name table), and it catches the ~18 DBCs the
// listfile doesn't index. See `ScanPathGetters`.
//
// Files land relative to the client's working directory, mirroring the
// retail command's `BlizzardInterface{Code,Art}\<relative-path>`
// layout. On completion it writes a "wrote N files" line back to the
// console.
//
// **How it works (vanilla 1.12 / Octo):**
//
// Like 4.3.4, we enumerate the mounted archives' `(listfile)` — the
// flat, full-path file index — rather than walking directories. The
// vanilla primitive is `FUN_MPQ_ENUM_FILES` (`0x00401470`, the same
// MPQ walk the macro-icon loader uses): it loops the archive handles
// and dispatches a callback per listfile entry under a path prefix
// (here `"Interface\"`). We collect every matching full path (deduped
// across archives), then read each via `FUN_FILE_READ` and write it to
// disk with Win32.
//
// Don't confuse this with `FUN_0042AD10`, the disk-only
// `FindFirstFileW` walker — pointing the export at that yields zero
// MPQ files. See the `FUN_MPQ_ENUM_FILES` note in Offsets.h.
//
// We export the full `Interface\` tree, matching 4.3.4 (extension
// filter only, no path exclusion). That INCLUDES `Interface\AddOns\` —
// the listfile only indexes MPQ-baked files, so those AddOns entries
// are Blizzard's own UI addons (Blizzard_AuctionUI, Blizzard_TalentUI,
// …), which are part of the stock UI source. The player's loose,
// on-disk addons live in no archive's listfile, so they're never
// enumerated and never re-exported.

#include "Game.h"
#include "Offsets.h"

#include <windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>

namespace Interface::Export {

namespace {

// Engine's file-content reader (see Offsets::FUN_FILE_READ). __stdcall
// (callee cleans 28 bytes via RET 0x1C — NOT __cdecl; getting this
// wrong corrupts the caller's stack). Allocates a Storm buffer; free
// with SMemFree.
using FileRead_t = int(__stdcall *)(int unused, const char *path, void **outBuf,
                                    unsigned int *outSize, unsigned int extraBytes,
                                    int flag1, int flag2);
using SMemFree_t = void(__stdcall *)(void *ptr, const char *file, int line, int flags);

// MPQ enumerator (see Offsets::FUN_MPQ_ENUM_FILES). The callback
// receives each listfile entry's full path in ecx; returning 0 STOPS
// enumeration, so we return 1 to keep going.
using MpqEnumCb_t = int(__fastcall *)(const char *fullPath);
using MpqEnumFiles_t = void(__fastcall *)(int archiveSelector, const char *pathPrefix,
                                          MpqEnumCb_t callback, void *userParam);

// The archive-set selector the engine's own macro-icon loader passes to
// FUN_00401470 — covers the primary mounted archives plus the selected
// extra. We mirror it.
constexpr int kArchiveSelector = 6;

// Case-insensitive ASCII equality.
bool EqualsCI(const char *a, const char *b) {
    for (;; ++a, ++b) {
        char ca = *a, cb = *b;
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<char>(ca + ('a' - 'A'));
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<char>(cb + ('a' - 'A'));
        if (ca != cb)
            return false;
        if (ca == '\0')
            return true;
    }
}

bool HasExtension(const char *name, const char *const *exts, int count) {
    const char *dot = nullptr;
    for (const char *p = name; *p; ++p) {
        if (*p == '.')
            dot = p;
    }
    if (dot == nullptr)
        return false;
    for (int i = 0; i < count; ++i) {
        if (EqualsCI(dot + 1, exts[i]))
            return true;
    }
    return false;
}

// Create every parent directory of `relPath` (a backslash-separated
// path, e.g. "BlizzardInterfaceCode\FrameXML\Foo.lua"). Each component
// is created relative to the working directory; existing dirs are a
// no-op.
void EnsureParentDirs(const std::string &relPath) {
    std::string accum;
    for (size_t i = 0; i < relPath.size(); ++i) {
        const char c = relPath[i];
        if (c == '\\' || c == '/') {
            if (!accum.empty())
                CreateDirectoryA(accum.c_str(), nullptr);
            accum.push_back('\\');
        } else {
            accum.push_back(c);
        }
    }
}

bool WriteFileToDisk(const std::string &relPath, const void *data, unsigned int size) {
    EnsureParentDirs(relPath);
    HANDLE h = CreateFileA(relPath.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    bool ok = true;
    if (size != 0) {
        DWORD written = 0;
        ok = WriteFile(h, data, size, &written, nullptr) != 0 && written == size;
    }
    CloseHandle(h);
    return ok;
}

const char *const kCodeExts[] = {"lua", "xml", "toc", "xsd"};
const char *const kArtExts[] = {"blp", "tga"};
const char *const kDbcExts[] = {"dbc"};

enum Mode { MODE_CODE, MODE_ART };

std::string LowerCopy(const char *s) {
    std::string out;
    for (const char *p = s; *p; ++p) {
        char c = *p;
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c + ('a' - 'A'));
        out.push_back(c);
    }
    return out;
}

// Collector for the MPQ enumeration callback (and the path-getter scan).
// File-static because FUN_00401470 hands the callback only the path
// string; enumeration is synchronous and single-threaded, so a
// file-static sink is safe.
struct Collector {
    const char *const *exts = nullptr;
    int extCount = 0;
    std::vector<std::string> files;
    std::unordered_set<std::string> seen; // lowercased keys — dedup across sources

    // Case-insensitive dedup so the listfile and the path-getter scan
    // (which may differ in casing) never write the same DBC twice.
    void Add(const char *path) {
        if (seen.insert(LowerCopy(path)).second)
            files.emplace_back(path);
    }
};
Collector *g_collector = nullptr;

// MPQ enumeration callback — one call per listfile entry under the
// active prefix. Keep going (return 1) regardless.
int __fastcall CollectCb(const char *fullPath) {
    Collector *c = g_collector;
    if (c != nullptr && fullPath != nullptr &&
        HasExtension(fullPath, c->exts, c->extCount))
        c->Add(fullPath);
    return 1; // 0 would stop enumeration
}

// --- DBC path-getter scan ---------------------------------------------
//
// The DBC loaders aren't driven by any iterable name table — the master
// init (`FUN_0053f8b0`) is straight-line, calling ~150 per-DBC loaders in
// sequence, each of which calls its own one-instruction path-getter:
//
//     B8 <&"DBFilesClient\Foo.dbc">   mov eax, <strVA>
//     C3                              ret
//
// So the authoritative, dynamic list of "what DBCs does this build load"
// is the set of those getters. We recover it the same way docs/DBCs.md
// did — scan `.text` for the `B8 imm32 C3` pattern and keep matches whose
// immediate points at a `.dbc` string. This catches every loaded table,
// including the ~18 the MPQ `(listfile)` doesn't index.

// Build-specific bounds (this 1.12.1 client). `.text` per CLAUDE.md; the
// image range bounds candidate string pointers so the suffix check never
// dereferences unmapped memory. Getter strings live in `.data` (e.g.
// Spell.dbc at 0x00859E30), comfortably inside the image.
constexpr uintptr_t kTextStart = 0x00401000;
constexpr uintptr_t kTextEnd = 0x007FEDAC;
constexpr uintptr_t kImageStart = 0x00400000;
constexpr uintptr_t kImageEnd = 0x00D26000;

// True if `s` (bounded, never reading past the image) names a ".dbc" file.
bool LooksLikeDbcString(const uint8_t *s) {
    const uint8_t *limit = reinterpret_cast<const uint8_t *>(kImageEnd);
    size_t n = 0;
    while (n < 260 && s + n < limit && s[n] != '\0')
        ++n;
    if (s + n >= limit || s[n] != '\0' || n < 4) // not terminated, or too short
        return false;
    return EqualsCI(reinterpret_cast<const char *>(s + n - 4), ".dbc");
}

// Scan .text for the DBC path-getter pattern and add each referenced
// ".dbc" path to the collector. Synchronous, reads only our own image.
void ScanPathGetters(Collector &c) {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(kTextStart);
    const uint8_t *end = reinterpret_cast<const uint8_t *>(kTextEnd) - 6;
    for (; p <= end; ++p) {
        if (p[0] != 0xB8 || p[5] != 0xC3) // mov eax, imm32 ; ret
            continue;
        uint32_t imm;
        std::memcpy(&imm, p + 1, sizeof(imm));
        if (imm < kImageStart || imm >= kImageEnd)
            continue;
        const uint8_t *s = reinterpret_cast<const uint8_t *>(imm);
        if (LooksLikeDbcString(s))
            c.Add(reinterpret_cast<const char *>(s));
    }
}

// Read each collected source path from the MPQs and write it under
// `dstRoot`, re-rooted by stripping the leading `prefixLen` chars (the
// "Interface\" / "DBFilesClient\" prefix). Returns the number written.
int WriteFiles(const std::vector<std::string> &files, size_t prefixLen,
               const char *dstRoot) {
    auto FileRead = reinterpret_cast<FileRead_t>(Offsets::FUN_FILE_READ);
    auto SMemFree = reinterpret_cast<SMemFree_t>(Offsets::FUN_STORM_SMEM_FREE);

    int written = 0;
    for (const std::string &src : files) {
        void *buf = nullptr;
        unsigned int size = 0;
        if (FileRead(0, src.c_str(), &buf, &size, 0, 1, 0) == 0 || buf == nullptr)
            continue;

        // Re-root: "<prefix>Sub\Foo.ext" -> "<dstRoot>\Sub\Foo.ext".
        std::string dst = dstRoot;
        dst.push_back('\\');
        dst.append(src, prefixLen, std::string::npos);

        if (WriteFileToDisk(dst, buf, size))
            ++written;

        SMemFree(buf, __FILE__, __LINE__, 0);
    }
    return written;
}

// Enumerate every MPQ file under `pathPrefix` whose extension matches
// one of `exts`, read each, and write it under `dstRoot`. Returns the
// number of files successfully written.
int ExportTree(const char *pathPrefix, const char *const *exts, int extCount,
               const char *dstRoot) {
    auto MpqEnumFiles = reinterpret_cast<MpqEnumFiles_t>(Offsets::FUN_MPQ_ENUM_FILES);

    Collector collector;
    collector.exts = exts;
    collector.extCount = extCount;

    g_collector = &collector;
    MpqEnumFiles(kArchiveSelector, pathPrefix, &CollectCb, nullptr);
    g_collector = nullptr;

    return WriteFiles(collector.files, std::strlen(pathPrefix), dstRoot);
}

// Export the DBC tables. Unions two sources, deduped case-insensitively:
//   1. the MPQ (listfile) under DBFilesClient\ (catches files that ship
//      in the archives without a loader, e.g. wowerror_strings.dbc), and
//   2. the .text path-getter scan (the authoritative set the client
//      actually loads — catches the ~18 DBCs absent from the listfile).
// Every DBC path is "DBFilesClient\Name.dbc", so re-rooting strips that
// 14-char prefix and writes under `dstRoot`.
int ExportDBC(const char *dstRoot) {
    auto MpqEnumFiles = reinterpret_cast<MpqEnumFiles_t>(Offsets::FUN_MPQ_ENUM_FILES);

    Collector collector;
    collector.exts = kDbcExts;
    collector.extCount = 1;

    g_collector = &collector;
    MpqEnumFiles(kArchiveSelector, "DBFilesClient\\", &CollectCb, nullptr);
    g_collector = nullptr;

    ScanPathGetters(collector);

    return WriteFiles(collector.files, std::strlen("DBFilesClient\\"), dstRoot);
}

// Console-command handler ABI: edx = the argument text after the
// command name ("" when bare). Mirrors 4.3.4's `art`/`code` dispatch.
bool FirstTokenEquals(const char *s, const char *token) {
    size_t i = 0;
    for (; token[i] != '\0'; ++i) {
        char a = s[i];
        if (a >= 'A' && a <= 'Z') a = static_cast<char>(a + ('a' - 'A'));
        if (a != token[i])
            return false;
    }
    const char end = s[i];
    return end == '\0' || end == ' ' || end == '\t';
}

int __fastcall Console_ExportInterfaceFiles(void * /*unused*/, const char *args) {
    const char *p = (args != nullptr) ? args : "";
    while (*p == ' ' || *p == '\t')
        ++p;

    Mode mode;
    if (FirstTokenEquals(p, "art"))
        mode = MODE_ART;
    else if (FirstTokenEquals(p, "code"))
        mode = MODE_CODE;
    else {
        Game::Console::Write("Usage: ExportInterfaceFiles art|code");
        return 1;
    }

    const int written = ExportTree(
        "Interface\\",
        (mode == MODE_ART) ? kArtExts : kCodeExts,
        (mode == MODE_ART) ? 2 : 4,
        (mode == MODE_ART) ? "BlizzardInterfaceArt" : "BlizzardInterfaceCode");

    char msg[160];
    snprintf(msg, sizeof(msg), "ExportInterfaceFiles: wrote %d %s file(s) to %s\\",
             written, (mode == MODE_ART) ? "art" : "code",
             (mode == MODE_ART) ? "BlizzardInterfaceArt" : "BlizzardInterfaceCode");
    Game::Console::Write(msg);
    return 1;
}

// `ExportDBCFiles` — ClassicAPI-original companion to ExportInterfaceFiles.
// Dumps every `.dbc` table the client loads to `DBFilesClient\` under the
// working directory. No subcommand — there's only one thing to export.
// Unions the MPQ (listfile) with a `.text` path-getter scan so it gets
// the complete set the client loads, including DBCs the listfile omits.
int __fastcall Console_ExportDBCFiles(void * /*unused*/, const char * /*args*/) {
    const int written = ExportDBC("DBFilesClient");

    char msg[96];
    snprintf(msg, sizeof(msg),
             "ExportDBCFiles: wrote %d .dbc file(s) to DBFilesClient\\", written);
    Game::Console::Write(msg);
    return 1;
}

// Register the console command. A console command is process-global
// (not Lua-state-bound) and persists for the whole session, so the
// single registration at glue boot — the earliest hook, firing at
// launch before any console can be opened — covers both the login
// console and in-world. The glue hook re-fires on each world→glue
// return (logout), but the engine's registrar dedups by name (and we
// pass the same string literal, so its pointer-equality fast path
// catches the repeat), making a second call a no-op. The command name
// and help string are stored by pointer, so they must be static —
// string literals satisfy that for the DLL's lifetime.
void EnsureRegistered() {
    Game::Console::RegisterCommand(
        "ExportInterfaceFiles", &Console_ExportInterfaceFiles,
        Game::Console::CATEGORY_DEBUG,
        "Extracts Blizzard's UI files from the MPQs to disk. "
        "Usage: ExportInterfaceFiles art|code");
    Game::Console::RegisterCommand(
        "ExportDBCFiles", &Console_ExportDBCFiles,
        Game::Console::CATEGORY_DEBUG,
        "Extracts the client's .dbc tables from the MPQs to disk. "
        "Usage: ExportDBCFiles");
}

const Game::GlueModuleAutoRegister _autoreg{&EnsureRegistered};

} // namespace

} // namespace Interface::Export
