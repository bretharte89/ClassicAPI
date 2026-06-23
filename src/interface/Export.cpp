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

// Length of the "Interface\" prefix we strip from MPQ paths before
// re-rooting under the destination folder.
constexpr size_t kRootPrefixLen = 10; // strlen("Interface\\")

const char *const kCodeExts[] = {"lua", "xml", "toc", "xsd"};
const char *const kArtExts[] = {"blp", "tga"};

enum Mode { MODE_CODE, MODE_ART };

// Collector for the MPQ enumeration callback. File-static because
// FUN_00401470 hands the callback only the path string; enumeration is
// synchronous and single-threaded, so a file-static sink is safe.
struct Collector {
    const char *const *exts = nullptr;
    int extCount = 0;
    std::vector<std::string> files;
    std::unordered_set<std::string> seen; // dedup across archives
};
Collector *g_collector = nullptr;

// MPQ enumeration callback — one call per listfile entry under the
// "Interface\" prefix. Keep going (return 1) regardless.
int __fastcall CollectCb(const char *fullPath) {
    Collector *c = g_collector;
    if (c != nullptr && fullPath != nullptr &&
        HasExtension(fullPath, c->exts, c->extCount)) {
        if (c->seen.insert(fullPath).second)
            c->files.emplace_back(fullPath);
    }
    return 1; // 0 would stop enumeration
}

// Enumerate every MPQ file under Interface\ whose extension matches
// `mode`, read each, and write it under the destination root. Returns
// the number of files successfully written.
int ExportTree(Mode mode) {
    auto MpqEnumFiles = reinterpret_cast<MpqEnumFiles_t>(Offsets::FUN_MPQ_ENUM_FILES);
    auto FileRead = reinterpret_cast<FileRead_t>(Offsets::FUN_FILE_READ);
    auto SMemFree = reinterpret_cast<SMemFree_t>(Offsets::FUN_STORM_SMEM_FREE);

    const char *dstRoot =
        (mode == MODE_ART) ? "BlizzardInterfaceArt" : "BlizzardInterfaceCode";

    Collector collector;
    collector.exts = (mode == MODE_ART) ? kArtExts : kCodeExts;
    collector.extCount = (mode == MODE_ART) ? 2 : 4;

    g_collector = &collector;
    MpqEnumFiles(kArchiveSelector, "Interface\\", &CollectCb, nullptr);
    g_collector = nullptr;

    const std::vector<std::string> &files = collector.files;
    int written = 0;
    for (const std::string &src : files) {
        void *buf = nullptr;
        unsigned int size = 0;
        if (FileRead(0, src.c_str(), &buf, &size, 0, 1, 0) == 0 || buf == nullptr)
            continue;

        // Re-root: "Interface\FrameXML\Foo.lua" -> "<dstRoot>\FrameXML\Foo.lua".
        std::string dst = dstRoot;
        dst.push_back('\\');
        dst.append(src, kRootPrefixLen, std::string::npos);

        if (WriteFileToDisk(dst, buf, size))
            ++written;

        SMemFree(buf, __FILE__, __LINE__, 0);
    }
    return written;
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

    const int written = ExportTree(mode);

    char msg[160];
    snprintf(msg, sizeof(msg), "ExportInterfaceFiles: wrote %d %s file(s) to %s\\",
             written, (mode == MODE_ART) ? "art" : "code",
             (mode == MODE_ART) ? "BlizzardInterfaceArt" : "BlizzardInterfaceCode");
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
}

const Game::GlueModuleAutoRegister _autoreg{&EnsureRegistered};

} // namespace

} // namespace Interface::Export
