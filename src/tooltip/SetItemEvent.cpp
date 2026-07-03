// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// ClassicAPI. If not, see <https://www.gnu.org/licenses/>.

// `GameTooltip:HookScript("OnTooltipSetItem", fn)` — backport of the modern
// tooltip script that fires whenever a tooltip's item is set, so addons can
// annotate item tooltips by hooking one script instead of wrapping every
// Set* method (the pattern AtlasLoot resorts to in vanilla, rebuilding the
// tooltip and clobbering its item linkage). Fully native — a real frame
// script, settable via the standard SetScript / GetScript / HookScript.
//
// 1.12 already has the machinery for tooltip scripts, just not this one:
//
//   - The CGGameTooltip vtable resolver FUN_GAMETOOLTIP_SCRIPT_RESOLVER maps a
//     script name to its handler slot on the tooltip (OnTooltipCleared etc.,
//     each an 8-byte {handler, context} pair). SetScript/GetScript/HookScript
//     all go through it. We co-hook it: for "OnTooltipSetItem" (a name the
//     engine doesn't know) we hand back a pointer to a C-side per-tooltip cell
//     — the tooltip object (alloc size 0x460) has no free 8-byte slot, and the
//     resolver's only contract is "return the address the script lives at", so
//     external storage works exactly like an in-object slot.
//
//   - Every item setter (SetHyperlink item:, SetBagItem, SetInventoryItem,
//     SetMerchantItem, SetAuctionItem, …) funnels through the shared item
//     builder FUN_GAMETOOLTIP_BUILD_ITEM. We co-hook it and, after the build,
//     fire the handler via FUN_FRAME_RUN_SCRIPT (the same no-arg runner the
//     clear uses for OnTooltipCleared) with the tooltip as `self`.
//
// Suppressed during our own Tooltip::Compare equipped-item build (see
// Suppressor), and guarded against a handler that re-enters the builder.

#include "SetItemEvent.h"

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Tooltip::SetItemEvent {

namespace {

// Per-tooltip 8-byte handler slot {handler, context}. Tooltips are few and
// permanent (GameTooltip, ItemRefTooltip, ShoppingTooltip1/2, AtlasLootTooltip,
// WorldMapTooltip, plus the odd addon tooltip), so a small fixed table with a
// linear scan is plenty and gives each cell a stable address to hand out.
struct Cell {
    void *tooltip;
    uint32_t slot[2]; // [0] = handler, [1] = exec context (written by SetScript)
};
constexpr int kMaxTooltips = 32;
Cell g_cells[kMaxTooltips];
int g_cellCount = 0;

uint32_t *CellFor(void *tooltip, bool create) {
    for (int i = 0; i < g_cellCount; ++i)
        if (g_cells[i].tooltip == tooltip)
            return g_cells[i].slot;
    if (!create || g_cellCount >= kMaxTooltips)
        return nullptr;
    Cell &c = g_cells[g_cellCount++];
    c.tooltip = tooltip;
    c.slot[0] = 0;
    c.slot[1] = 0;
    return c.slot;
}

bool EqualsIgnoreCase(const char *s, const char *literal) {
    if (s == nullptr)
        return false;
    for (;; ++s, ++literal) {
        unsigned char a = static_cast<unsigned char>(*s);
        const unsigned char b = static_cast<unsigned char>(*literal);
        if (a >= 'A' && a <= 'Z')
            a = static_cast<unsigned char>(a + 32);
        if (a != b)
            return false;
        if (b == 0)
            return true;
    }
}

constexpr char kScriptNameLower[] = "ontooltipsetitem"; // matched case-insensitively

// --- Suppression / recursion state -------------------------------------
int g_suppress = 0; // > 0 while our own compare build runs
int g_firing = 0;   // recursion guard around the handler fire

// --- Resolver co-hook (CGGameTooltip vtable script-name → slot) ---------
using Resolver_t = int(__fastcall *)(void *self, void *edx, const char *name);
Resolver_t g_resolverOriginal = nullptr;

int __fastcall Resolver_h(void *self, void *edx, const char *name) {
    const int engineSlot = g_resolverOriginal(self, edx, name);
    if (engineSlot != 0) // a base-frame or existing tooltip script — leave it
        return engineSlot;
    if (EqualsIgnoreCase(name, kScriptNameLower))
        return reinterpret_cast<int>(CellFor(self, /*create*/ true));
    return 0;
}

// --- Item-builder co-hook (fires the handler after the item is set) -----
using Build_t = void(__fastcall *)(void *self, void *edx, uint32_t itemID, const void *guid,
                                   const void *guid2, int a4, int a5, int a6, int headerFlag,
                                   int a8, int a9);
Build_t g_buildOriginal = nullptr;

using RunScript_t = void(__thiscall *)(void *self, uint32_t *slot);

void __fastcall Build_h(void *self, void *edx, uint32_t itemID, const void *guid,
                        const void *guid2, int a4, int a5, int a6, int headerFlag, int a8,
                        int a9) {
    g_buildOriginal(self, edx, itemID, guid, guid2, a4, a5, a6, headerFlag, a8, a9);
    if (g_suppress > 0 || g_firing > 0)
        return;
    uint32_t *slot = CellFor(self, /*create*/ false);
    if (slot == nullptr || slot[0] == 0)
        return;
    ++g_firing;
    reinterpret_cast<RunScript_t>(Offsets::FUN_FRAME_RUN_SCRIPT)(self, slot);
    --g_firing;
}

static const Game::HookAutoRegister _resolverHook{
    Offsets::FUN_GAMETOOLTIP_SCRIPT_RESOLVER,
    reinterpret_cast<void *>(&Resolver_h),
    reinterpret_cast<void **>(&g_resolverOriginal)};

static const Game::HookAutoRegister _buildHook{
    Offsets::FUN_GAMETOOLTIP_BUILD_ITEM,
    reinterpret_cast<void *>(&Build_h),
    reinterpret_cast<void **>(&g_buildOriginal)};

} // namespace

Suppressor::Suppressor() { ++g_suppress; }
Suppressor::~Suppressor() {
    if (g_suppress > 0)
        --g_suppress;
}

} // namespace Tooltip::SetItemEvent
