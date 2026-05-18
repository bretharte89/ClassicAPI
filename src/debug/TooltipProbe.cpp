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
// Temporary probe used to hunt for metadata slots on GameTooltip frames —
// specifically, looking for whatever the aura-tooltip path
// (SetUnitBuff/SetUnitDebuff/SetPlayerBuff) stashes that would let us
// recover the spellID without hooking the inner builder at
// `FUN_0052F880`. Intended to be deleted once findings are baked in.
//
// Bound globals:
//
//   _classicapi_DumpTooltipBytes(tooltip, [label])
//      Hex-dumps `[tooltip+0x300..+0x500]` (a 512-byte window covering
//      every state slot Clear at FUN_00530050 touches plus surrounding
//      slack) to `debug.log` via Debug::Log::HexDump. The offset column
//      shows real VAs so values that look like pointers can be
//      eyeballed in Ghidra.
//
//      `label` defaults to "tooltip dump"; passing one labels the
//      block so you can diff across multiple hovers.
//
//      Usage from /script (with the GameTooltip frame currently
//      showing an aura):
//        /script _classicapi_DumpTooltipBytes(GameTooltip, "buff Mark of the Wild")
//        /script _classicapi_DumpTooltipBytes(GameTooltip, "buff Power Word: Fortitude")
//        /script _classicapi_DumpTooltipBytes(GameTooltip, "debuff Curse of Agony")
//        /script _classicapi_DumpTooltipBytes(GameTooltip, "item Hearthstone")
//      then read debug.log and diff the snapshots. Slots that
//      change with the spell are candidates for spellID storage;
//      slots that stay stable across spells but differ between
//      aura/non-aura states are candidate flags.

#include "Game.h"
#include "Log.h"

#include <cstdint>
#include <cstdio>

namespace Debug::TooltipProbe {

namespace {

int __fastcall Script_DumpTooltipBytes(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L,
            "Usage: _classicapi_DumpTooltipBytes(tooltip, [label])");
        return 0;
    }
    void *tooltipObj = Game::Lua::ResolveObject(L, 1);
    if (tooltipObj == nullptr) {
        Game::Lua::Error(L, "_classicapi_DumpTooltipBytes: not a frame object");
        return 0;
    }

    const char *label = "tooltip dump";
    if (Game::Lua::IsString(L, 2))
        label = Game::Lua::ToString(L, 2);

    // 0x300..+0x500 = 0x200 bytes; covers everything Clear touches
    // (+0x368..+0x39C state slots, +0x32C/+0x344/+0x364 line arrays,
    // +0x44C cleanup callback) plus surrounding slack so an
    // unexpected slot still falls in the window.
    auto *base = static_cast<const uint8_t *>(tooltipObj) + 0x300;
    Debug::Log::HexDump(label, base, 0x200,
                        reinterpret_cast<uintptr_t>(base));
    return 0;
}

// `_classicapi_DumpTooltipDeref(tooltip, [label])` — follows the
// candidate pointer at `tooltip+0x314` (the slot that varies per
// spell/item content) and hex-dumps the first 0x40 bytes pointed
// to. The first u32 there is what we hope is the spellID for aura
// tooltips. HexDump is SEH-wrapped so a faulting read won't crash
// the host process.
int __fastcall Script_DumpTooltipDeref(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L,
            "Usage: _classicapi_DumpTooltipDeref(tooltip, [label])");
        return 0;
    }
    void *tooltipObj = Game::Lua::ResolveObject(L, 1);
    if (tooltipObj == nullptr) {
        Game::Lua::Error(L, "_classicapi_DumpTooltipDeref: not a frame object");
        return 0;
    }

    const char *label = "tooltip deref";
    if (Game::Lua::IsString(L, 2))
        label = Game::Lua::ToString(L, 2);

    const uint32_t tag = *reinterpret_cast<const uint32_t *>(
        static_cast<const uint8_t *>(tooltipObj) + 0x310);
    const uintptr_t ptr = *reinterpret_cast<const uintptr_t *>(
        static_cast<const uint8_t *>(tooltipObj) + 0x314);

    char header[128];
    std::snprintf(header, sizeof(header),
                  "%s [+0x310 tag=0x%08X, +0x314 ptr=0x%08X]",
                  label, tag, static_cast<unsigned>(ptr));

    if (ptr == 0) {
        Debug::Log::Append(header, "(null pointer)");
        return 0;
    }
    Debug::Log::HexDump(header, reinterpret_cast<const void *>(ptr), 0x40,
                        ptr);
    return 0;
}

// `_classicapi_DumpTooltipChain(tooltip, [label])` — one-shot deep
// probe. Follows `tooltip+0x314` to the content object, dumps 64
// bytes there, then for each of the three sub-pointers we suspect
// might lead to the spellID (+0xC, +0x1C, +0x34 of the content
// object), dumps 64 more bytes if the sub-pointer is non-null.
// All into a single labeled block so a single hover-and-run is
// enough to capture an entire chain.
int __fastcall Script_DumpTooltipChain(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L,
            "Usage: _classicapi_DumpTooltipChain(tooltip, [label])");
        return 0;
    }
    void *tooltipObj = Game::Lua::ResolveObject(L, 1);
    if (tooltipObj == nullptr) {
        Game::Lua::Error(L, "_classicapi_DumpTooltipChain: not a frame object");
        return 0;
    }

    const char *label = "tooltip chain";
    if (Game::Lua::IsString(L, 2))
        label = Game::Lua::ToString(L, 2);

    const uintptr_t contentPtr = *reinterpret_cast<const uintptr_t *>(
        static_cast<const uint8_t *>(tooltipObj) + 0x314);

    char header[128];
    std::snprintf(header, sizeof(header),
                  "%s [content +0x314 = 0x%08X]",
                  label, static_cast<unsigned>(contentPtr));

    if (contentPtr == 0) {
        Debug::Log::Append(header, "(null content pointer)");
        return 0;
    }

    Debug::Log::HexDump(header, reinterpret_cast<const void *>(contentPtr),
                        0x40, contentPtr);

    // Sub-pointer candidates within the content object. +0xC is
    // non-null for buffs, +0x1C for debuffs, +0x34 for both.
    static const int kSubOffsets[] = {0xC, 0x1C, 0x34};
    for (int subOff : kSubOffsets) {
        const uintptr_t sub = *reinterpret_cast<const uintptr_t *>(
            contentPtr + subOff);
        char subLabel[128];
        std::snprintf(subLabel, sizeof(subLabel),
                      "  ↳ content+0x%X = 0x%08X",
                      subOff, static_cast<unsigned>(sub));
        if (sub == 0) {
            Debug::Log::Append(subLabel, "(null)");
            continue;
        }
        Debug::Log::HexDump(subLabel, reinterpret_cast<const void *>(sub),
                            0x40, sub);
    }
    return 0;
}

// `_classicapi_PeekBytes(addr, [len], [label])` — generic memory peek.
// Hex-dumps `len` bytes starting at `addr` (numeric VA). Defaults
// to 0x40 bytes and label "peek". SEH-wrapped via HexDump so a
// faulting read won't take down the host process. Useful for
// following pointer chains revealed by the more specific probes
// above — e.g. dumping what's at the +0xC or +0x34 fields of the
// tooltip's content object.
int __fastcall Script_PeekBytes(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: _classicapi_PeekBytes(addr, [len], [label])");
        return 0;
    }
    const uintptr_t addr = static_cast<uintptr_t>(Game::Lua::ToNumber(L, 1));
    int len = 0x40;
    if (Game::Lua::IsNumber(L, 2))
        len = static_cast<int>(Game::Lua::ToNumber(L, 2));
    if (len <= 0 || len > 0x1000)
        len = 0x40;

    const char *label = "peek";
    if (Game::Lua::IsString(L, 3))
        label = Game::Lua::ToString(L, 3);

    if (addr == 0) {
        Debug::Log::Append(label, "(null address)");
        return 0;
    }
    Debug::Log::HexDump(label, reinterpret_cast<const void *>(addr), len, addr);
    return 0;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_DumpTooltipBytes",
                                      &Script_DumpTooltipBytes);
    Game::Lua::RegisterGlobalFunction("_classicapi_DumpTooltipDeref",
                                      &Script_DumpTooltipDeref);
    Game::Lua::RegisterGlobalFunction("_classicapi_DumpTooltipChain",
                                      &Script_DumpTooltipChain);
    Game::Lua::RegisterGlobalFunction("_classicapi_PeekBytes",
                                      &Script_PeekBytes);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Debug::TooltipProbe
