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

// `C_NewItems` — the modern "new item glow" bookkeeping system. Retail
// flags items freshly added to your bags as "new" (the sparkle/flash on
// the bag button) and clears the flag once you've acknowledged them.
// Vanilla 1.12 never had the feature — no DBC, no server data, no engine
// bit; it's pure client-side bookkeeping over bag contents. So we add it,
// entirely in C++ the same way retail's client owns it — no addon Lua.
//
// Public surface (matches retail exactly):
//   C_NewItems.IsNewItem(bagID, slotIndex)   -> bool
//   C_NewItems.RemoveNewItem(bagID, slotIndex)
//   C_NewItems.ClearAll()
//
// Newness is keyed on each item's **instance GUID** (read off the CGItem),
// not on (bagID, slotIndex). That's what makes a flag survive the player
// rearranging a bag — exactly like retail, and impossible for a pure-Lua
// addon (vanilla exposes no per-instance GUID to Lua).
//
// The feed is event-driven, not polled: it hangs off `Bag::UpdateDelayed`,
// which co-hooks the engine's three `BAG_UPDATE` fire sites and signals once
// per frame in which a bag actually changed. On that signal we scan bags
// 0..4, diff the resident item GUIDs against the previous scan, flag anything
// newly acquired, and prune flags for items that left. `BAG_NEW_ITEMS_UPDATED`
// (no payload — matching retail) fires whenever the new-item set changes. So
// nothing runs on quiet frames.
//
// The one exception is establishing the login baseline: a lightweight
// WorldTick handler waits ~1.5s after the player is first resolvable (long
// enough for the initial bag population to finish), takes a single scan as the
// baseline, and then goes idle — items already owned at login are therefore
// never flagged. It re-arms on player-GUID change (relog / character switch).
// Bank and equipment are out of scope, matching retail.
//
// Bag enumeration is pure C++: it replicates what `PackBagSlot` does
// internally (see Offsets.h). The backpack is indexed straight off the
// player inventory manager; each equipped bag 1..4 is resolved to its
// container object and inventory via the same GUID getter + vtable call the
// engine uses, then walked with `GetItemBySlot`. None of the Lua-stack-bound
// helpers are touched, so the scan is safe from the frame-loop tick.

#include "Game.h"
#include "Offsets.h"
#include "bag/UpdateDelayed.h"
#include "event/Custom.h"
#include "item/Location.h"
#include "tick/WorldTick.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Item::NewItems {

namespace {

constexpr const char *kEvtNewItems = "BAG_NEW_ITEMS_UPDATED";
const Event::Custom::AutoReserve _r{kEvtNewItems};

// Bags cap at backpack (16) + four equipped bags; even the largest Turtle
// bags stay well under this. Overflow silently drops — a flagged-new item
// past the cap just won't glow, never a crash.
constexpr int kMaxItems = 256;

// Delay after the player is first resolvable before the baseline is taken,
// giving the initial bag population time to finish. Wall-clock (via the
// engine ms counter) so it's independent of frame rate.
constexpr uint32_t kSettleMs = 1500;

uint64_t g_seen[kMaxItems]; // all bag-resident GUIDs at the last scan
int g_seenCount = 0;
uint64_t g_new[kMaxItems]; // subset currently flagged "new"
int g_newCount = 0;
bool g_seeded = false;     // true once the login baseline has been taken
uint64_t g_playerGuid = 0; // re-arm the baseline when this changes (relog)
uint32_t g_loginMs = 0;    // engine ms at which the current player resolved

uint32_t NowMs() {
    using TickCount_t = uint32_t(__fastcall *)();
    return reinterpret_cast<TickCount_t>(Offsets::FUN_OS_TICKCOUNT_MS)();
}

bool Contains(const uint64_t *arr, int count, uint64_t guid) {
    for (int i = 0; i < count; ++i)
        if (arr[i] == guid)
            return true;
    return false;
}

bool AddUnique(uint64_t *arr, int *count, uint64_t guid) {
    if (Contains(arr, *count, guid) || *count >= kMaxItems)
        return false;
    arr[(*count)++] = guid;
    return true;
}

bool RemoveFrom(uint64_t *arr, int *count, uint64_t guid) {
    for (int i = 0; i < *count; ++i) {
        if (arr[i] == guid) {
            arr[i] = arr[--(*count)];
            return true;
        }
    }
    return false;
}

uint64_t ItemGUID(const uint8_t *item) {
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return *reinterpret_cast<const uint64_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_GUID);
}

// --- pure-C++ bag enumeration (no Lua stack) -----------------------------

using GetItemBySlot_t = void *(__thiscall *)(void *inventory, int slot0Based);

// Collects the instance GUIDs of every item in bags 0..4 into `out`.
// Returns the count. Pure C++ — safe to call off the frame-loop tick.
int ScanBags(const uint8_t *player, uint64_t *out) {
    auto getItemBySlot =
        reinterpret_cast<GetItemBySlot_t>(Offsets::FUN_ITEMMGR_GET_ITEM_BY_SLOT);
    int count = 0;

    // Backpack (bagID 0): indexed directly off the player inventory manager
    // at linear slot (S-1) + 23. GetItemBySlot only reads the manager, so the
    // const_cast to satisfy its `this` parameter is safe.
    auto *invMgr = const_cast<uint8_t *>(player) + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
    const int backpackSlots = Item::Location::GetBagSlotCount(0);
    for (int slot = 1; slot <= backpackSlots; ++slot) {
        auto *item = static_cast<const uint8_t *>(
            getItemBySlot(invMgr, Offsets::BACKPACK_LINEAR_BASE + slot - 1));
        const uint64_t guid = ItemGUID(item);
        if (guid != 0)
            AddUnique(out, &count, guid);
    }

    // Equipped bags 1..4: resolve each bag's inventory object, then walk it.
    for (int bagID = 1; bagID <= 4; ++bagID) {
        const int slots = Item::Location::GetBagSlotCount(bagID);
        if (slots <= 0)
            continue;
        void *bagInv = Item::Location::EquippedBagInventory(bagID);
        if (bagInv == nullptr)
            continue;
        for (int slot = 1; slot <= slots; ++slot) {
            auto *item = static_cast<const uint8_t *>(getItemBySlot(bagInv, slot - 1));
            const uint64_t guid = ItemGUID(item);
            if (guid != 0)
                AddUnique(out, &count, guid);
        }
    }
    return count;
}

void FireChanged() {
    Event::Custom::Fire(Event::Custom::Lookup(kEvtNewItems), "");
}

void CopyToSeen(const uint64_t *current, int count) {
    for (int i = 0; i < count; ++i)
        g_seen[i] = current[i];
    g_seenCount = count;
}

// Login-baseline handler. Runs every frame but does no work once the
// baseline is taken — it only watches for (re)login and, after a short
// settle, takes the single authoritative baseline scan. Steady state is just
// a GUID read + `if (g_seeded) return`.
void OnTick() {
    const uint64_t guid = Unit::Identity::PlayerGuid();
    if (guid == 0)
        return; // not in world yet

    // Fresh login / character switch: drop all state and re-arm the settle.
    if (guid != g_playerGuid) {
        g_playerGuid = guid;
        g_newCount = 0;
        g_seenCount = 0;
        g_seeded = false;
        g_loginMs = NowMs();
    }

    if (g_seeded)
        return; // baseline established; the bag-change signal drives the rest
    if (NowMs() - g_loginMs < kSettleMs)
        return; // let the initial bag population finish

    const uint8_t *player = Unit::Identity::PlayerObject();
    if (player == nullptr)
        return; // not resolvable yet — retry next tick

    // Everything currently in the bags becomes the baseline; nothing owned at
    // login is "new". Any change up to this point is intentionally absorbed.
    g_seenCount = ScanBags(player, g_seen);
    g_newCount = 0;
    g_seeded = true;
}

// Authoritative bag-change handler — fires only on frames where the engine
// actually changed a bag (via Bag::UpdateDelayed). Diffs the current bag set
// against the baseline, flags newly-acquired items, prunes departed ones.
void OnBagChanged() {
    if (!g_seeded)
        return; // pre-baseline changes are folded into the login baseline
    const uint64_t guid = Unit::Identity::PlayerGuid();
    if (guid == 0 || guid != g_playerGuid)
        return; // mid-relog; OnTick re-arms the baseline
    const uint8_t *player = Unit::Identity::PlayerObject();
    if (player == nullptr)
        return;

    uint64_t current[kMaxItems];
    const int currentCount = ScanBags(player, current);

    bool changed = false;
    // Drop flags for items no longer in the bags (used, sold, banked).
    for (int i = 0; i < g_newCount;) {
        if (!Contains(current, currentCount, g_new[i])) {
            g_new[i] = g_new[--g_newCount];
            changed = true;
        } else {
            ++i;
        }
    }
    // Flag anything not present at the previous scan.
    for (int i = 0; i < currentCount; ++i) {
        if (!Contains(g_seen, g_seenCount, current[i]))
            changed |= AddUnique(g_new, &g_newCount, current[i]);
    }
    CopyToSeen(current, currentCount);

    if (changed)
        FireChanged();
}

const Tick::WorldTick::AutoSubscribe _tick{&OnTick};
const Bag::UpdateDelayed::AutoSubscribe _bagChange{&OnBagChanged};

// --- C_NewItems Lua surface ----------------------------------------------

// Resolves a `(bagID, slotIndex)` Lua-arg pair to the item's instance GUID,
// or 0 if the args are missing/non-numeric or the slot is empty. Uses the
// Lua-context bag resolver — fine here, these run as Lua C functions.
uint64_t SlotGUID(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2))
        return 0;
    const int bagID = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int slotIndex = static_cast<int>(Game::Lua::ToNumber(L, 2));
    return ItemGUID(Item::Location::ResolveBag(L, bagID, slotIndex));
}

int __fastcall Script_IsNewItem(void *L) {
    const uint64_t guid = SlotGUID(L);
    Game::Lua::PushBool(L, guid != 0 && Contains(g_new, g_newCount, guid));
    return 1;
}

int __fastcall Script_RemoveNewItem(void *L) {
    const uint64_t guid = SlotGUID(L);
    if (guid != 0 && RemoveFrom(g_new, &g_newCount, guid))
        FireChanged();
    return 0;
}

int __fastcall Script_ClearAll(void *L) {
    (void)L;
    if (g_newCount > 0) {
        g_newCount = 0;
        FireChanged();
    }
    return 0;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_NewItems", "IsNewItem", &Script_IsNewItem);
    Game::Lua::RegisterTableFunction("C_NewItems", "RemoveNewItem", &Script_RemoveNewItem);
    Game::Lua::RegisterTableFunction("C_NewItems", "ClearAll", &Script_ClearAll);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Item::NewItems
