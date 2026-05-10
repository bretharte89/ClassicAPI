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

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"
#include "item/Arg.h"
#include "item/Data.h"
#include "item/Location.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace Item::Data {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, int unused);

// Two events fire on item-data arrival; they are **mutually exclusive**
// based on what triggered the cache fill:
//
//   `GET_ITEM_INFO_RECEIVED(itemID, success)` — added in 5.4. Fires for
//       **implicit / transparent** cache fills: `GetItemInfo` warmup,
//       hyperlink hover, chat-link resolution, our `SetItemByID` warmup,
//       etc. This is the event most existing addons listen to.
//
//   `ITEM_DATA_LOAD_RESULT(itemID, success)`  — added in 8.0. Fires for
//       **explicit** request paths only: `C_Item.RequestLoadItemData(ByID)`
//       and friends. The completion event for callers who explicitly
//       asked the modern Item-data API to load a specific item.
//
// Behavior verified empirically against 1.15.x: a single cache fill
// fires exactly one event, never both, depending on what initiated
// the request. We honor the split with two distinct callbacks.
//
// Slots are looked up via `Event::Custom::Register` which caches by
// pointer and retries failed claims; the engine's event table isn't
// populated when our boot hook fires, so all early registrations
// return -1 until `RetryAll` claims them later.
static constexpr const char *kItemDataLoadResult = "ITEM_DATA_LOAD_RESULT";
static constexpr const char *kGetItemInfoReceived = "GET_ITEM_INFO_RECEIVED";

static void FireGetItemInfoReceived(int itemID, int success) {
    const int slot = Event::Custom::Register(kGetItemInfoReceived);
    if (slot >= 0)
        Event::Custom::Fire_DD(slot, itemID, success);
}

static void FireItemDataLoadResult(int itemID, int success) {
    const int slot = Event::Custom::Register(kItemDataLoadResult);
    if (slot >= 0)
        Event::Custom::Fire_DD(slot, itemID, success);
}

// Engine callback for **implicit** cache fills (transparent warmup
// triggered by our `Script_GetItemInfo` hook or `SetItemByID`). Fires
// only the broad `GET_ITEM_INFO_RECEIVED` event — `ITEM_DATA_LOAD_RESULT`
// is reserved for explicit `RequestLoadItemData(ByID)` paths to match
// modern API semantics.
//
// Calling convention is __stdcall, 2 args / 8 bytes — matching the
// callback at 0x004FDC30 (used by 4 sites in the binary) which ends in
// `ret 8`. If the engine called us with a different convention, that
// callback would crash the engine, so stdcall is verified by induction.
static void __stdcall ItemLoadCallback_Implicit(void *userData, int success) {
    const auto itemID = static_cast<int>(reinterpret_cast<uintptr_t>(userData));
    FireGetItemInfoReceived(itemID, success ? 1 : 0);
}

// Engine callback for **explicit** cache fills (player-side
// `C_Item.RequestLoadItemData(ByID)` calls). Fires ITEM_DATA_LOAD_RESULT
// only — the implicit GET_ITEM_INFO_RECEIVED path is for ambient cache
// fills, not for callers who explicitly asked.
static void __stdcall ItemLoadCallback_Explicit(void *userData, int success) {
    const auto itemID = static_cast<int>(reinterpret_cast<uintptr_t>(userData));
    FireItemDataLoadResult(itemID, success ? 1 : 0);
}

using ItemLoadCallback_t = void(__stdcall *)(void *userData, int success);

// Calls DBCache_ItemStats_C_GetRecord. With `callback=nullptr`,
// performs only the hash-table lookup; returns the cached record or
// nullptr. With a non-null callback, also kicks off the
// SMSG_ITEM_QUERY_SINGLE if the item isn't cached — the engine fills
// the cache asynchronously and invokes the supplied callback with the
// itemID smuggled through `userData` (the engine just stores and
// replays it without dereferencing).
//
// Disassembly of `_GetRecord` at 0x55BAAC shows it branches on the
// callback arg (stack slot +0x10), not on the 6th `bool` arg that
// VanillaHelpers documented as `requestIfMissing` — that slot is reused
// as a local immediately after entry, never read. The trigger for the
// request path is `callback != NULL`.
static const uint8_t *CacheFetch(uint32_t itemID, ItemLoadCallback_t callback) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    void *cb = reinterpret_cast<void *>(callback);
    void *userData =
        callback ? reinterpret_cast<void *>(static_cast<uintptr_t>(itemID)) : nullptr;
    return fn(cache, itemID, &zeroGuid, cb, userData, 0);
}

// Resolves an item-location table at stack idx to the itemID by walking
// `Location::Resolve` -> CGItem -> instance block (+0x08) -> itemID (+0x0C).
// Returns 0 if the slot is empty or the table is malformed.
static int ResolveLocationToItemID(void *L, int idx) {
    const uint8_t *item = Item::Location::Resolve(L, idx);
    if (item == nullptr)
        return 0;
    auto *instance = *reinterpret_cast<const uint8_t *const *>(
        item + Offsets::OFF_ITEM_INSTANCE_BLOCK);
    if (instance == nullptr)
        return 0;
    return static_cast<int>(*reinterpret_cast<const uint32_t *>(
        instance + Offsets::OFF_INSTANCE_BLOCK_ITEM_ID));
}

static int __fastcall Script_IsItemDataCachedByID(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    const bool cached =
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), nullptr) != nullptr);
    Game::Lua::PushBoolean(L, cached ? 1 : 0);
    return 1;
}

static int __fastcall Script_IsItemDataCached(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.IsItemDataCached(itemLocation)");
        return 0;
    }
    const int itemID = ResolveLocationToItemID(L, 1);
    const bool cached =
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), nullptr) != nullptr);
    Game::Lua::PushBoolean(L, cached ? 1 : 0);
    return 1;
}

// Explicit-request path: kicks off the cache fill via the explicit
// callback (which fires ITEM_DATA_LOAD_RESULT). If the item is already
// cached, the engine won't invoke our callback — synthesize the event
// so addons get the same notification regardless of cache state,
// matching modern `C_Item.RequestLoadItemData(ByID)` semantics.
static void RequestAndMaybeNotify(uint32_t itemID) {
    const bool wasCached = (CacheFetch(itemID, nullptr) != nullptr);
    CacheFetch(itemID, &ItemLoadCallback_Explicit);
    if (wasCached)
        FireItemDataLoadResult(static_cast<int>(itemID), 1);
}

// Public C++ API — see Item/Data.h. Implicit (transparent) warmup
// path used by the `Script_GetItemInfo` hook and `SetItemByID`. Fires
// the cache request only if the item isn't already cached; uses the
// implicit callback which fires only `GET_ITEM_INFO_RECEIVED` (not
// `ITEM_DATA_LOAD_RESULT`), matching modern semantics for ambient
// cache fills.
void WarmCache(uint32_t itemID) {
    CacheFetch(itemID, &ItemLoadCallback_Implicit);
}

// Hook for `Script_GetItemInfo`. Pre-warms the cache from arg 1, then
// dispatches to the original. Effect: the original still returns nil
// for cache misses (vanilla behavior), but a query is now in flight,
// so subsequent calls will return valid data and
// GET_ITEM_INFO_RECEIVED will fire.
Script_GetItemInfo_t Script_GetItemInfo_o = nullptr;

int __fastcall Script_GetItemInfo_h(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID > 0)
        WarmCache(static_cast<uint32_t>(itemID));
    return Script_GetItemInfo_o(L);
}

static int __fastcall Script_RequestLoadItemDataByID(void *L) {
    const int itemID = Item::Arg::ResolveItemID(L, 1);
    if (itemID <= 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    RequestAndMaybeNotify(static_cast<uint32_t>(itemID));
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

static int __fastcall Script_RequestLoadItemData(void *L) {
    if (Game::Lua::Type(L, 1) != Game::Lua::TYPE_TABLE) {
        Game::Lua::Error(L, "Usage: C_Item.RequestLoadItemData(itemLocation)");
        return 0;
    }
    const int itemID = ResolveLocationToItemID(L, 1);
    if (itemID <= 0) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    RequestAndMaybeNotify(static_cast<uint32_t>(itemID));
    Game::Lua::PushBoolean(L, 1);
    return 1;
}

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_Item", "IsItemDataCachedByID",
                                     &Script_IsItemDataCachedByID);
    Game::Lua::RegisterTableFunction("C_Item", "IsItemDataCached", &Script_IsItemDataCached);
    Game::Lua::RegisterTableFunction("C_Item", "RequestLoadItemDataByID",
                                     &Script_RequestLoadItemDataByID);
    Game::Lua::RegisterTableFunction("C_Item", "RequestLoadItemData", &Script_RequestLoadItemData);

    // Seed both events so `Event::Custom::RetryAll` (driven by our hook
    // on `Frame::RegisterEvent`) knows to claim slots for them. The
    // engine's event table isn't populated yet when this fires, so all
    // these calls return -1; later attempts via `RetryAll` or any
    // direct `Register` call land them. Once claimed, slots are cached
    // and reused.
    Event::Custom::Register(kItemDataLoadResult);
    Event::Custom::Register(kGetItemInfoReceived);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Data
