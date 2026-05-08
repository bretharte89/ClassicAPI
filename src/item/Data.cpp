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
#include "item/Location.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace Item::Data {

using GetItemRecord_t = const uint8_t *(__thiscall *)(void *cache, uint32_t itemID,
                                                       const uint64_t *guid, void *callback,
                                                       void *userData, int unused);

// `ITEM_DATA_LOAD_RESULT` event-table slot. We never store this directly;
// instead we look it up via `Event::Custom::Register` which caches by
// pointer and retries failed claims. The engine's table isn't populated
// when our boot hook fires, so all early registrations return -1 — the
// retry path inside `Custom::Register` (also driven by our hook on
// `Frame::RegisterEvent`) eventually claims the slot.
static constexpr const char *kItemDataLoadResult = "ITEM_DATA_LOAD_RESULT";

// Callback the engine invokes from its SMSG response handler after the
// data lands in the cache. We get the itemID back via `userData` (passed
// as `(void *)(uintptr_t)itemID` at request time — the engine just stores
// and replays it without dereferencing) and `success` as the 2nd arg.
//
// Calling convention is __stdcall, 2 args / 8 bytes — matching the
// callback at 0x004FDC30 (used by 4 sites in the binary) which ends in
// `ret 8`. If the engine called us with a different convention, that
// callback would crash the engine, so stdcall is verified by induction.
static void __stdcall ItemLoadCallback(void *userData, int success) {
    const int slot = Event::Custom::Register(kItemDataLoadResult);
    if (slot < 0)
        return;
    const auto itemID =
        static_cast<int>(reinterpret_cast<uintptr_t>(userData));
    Event::Custom::Fire_DD(slot, itemID, success ? 1 : 0);
}

// Calls DBCache_ItemStats_C_GetRecord. With `fireRequest=false`, performs
// only the hash-table lookup; returns the cached record or nullptr.
// With `fireRequest=true`, also kicks off the SMSG_ITEM_QUERY_SINGLE if
// the item isn't cached — the engine fills the cache asynchronously and
// invokes our callback with `userData` (we pass the itemID through it so
// the callback knows which item completed).
//
// Disassembly of `_GetRecord` at 0x55BAAC shows it branches on the
// callback arg (stack slot +0x10), not on the 6th `bool` arg that
// VanillaHelpers documented as `requestIfMissing` — that slot is reused
// as a local immediately after entry, never read. The trigger for the
// request path is `callback != NULL`.
static const uint8_t *CacheFetch(uint32_t itemID, bool fireRequest) {
    auto fn = reinterpret_cast<GetItemRecord_t>(Offsets::FUN_DBCACHE_ITEMSTATS_GET_RECORD);
    auto *cache = reinterpret_cast<void *>(Offsets::VAR_ITEMDB_CACHE);
    const uint64_t zeroGuid = 0;
    void *callback = fireRequest ? reinterpret_cast<void *>(&ItemLoadCallback) : nullptr;
    void *userData =
        fireRequest ? reinterpret_cast<void *>(static_cast<uintptr_t>(itemID)) : nullptr;
    return fn(cache, itemID, &zeroGuid, callback, userData, 0);
}

// Same input shape as `Item::Info::ResolveItemID` — accepts a number or a
// string containing "item:NNN" (matches both the bare shorthand and full
// chat links). Returns 0 on bad input or unparseable strings.
static int ParseItemArg(void *L, int idx) {
    if (Game::Lua::IsNumber(L, idx))
        return static_cast<int>(Game::Lua::ToNumber(L, idx));
    if (Game::Lua::Type(L, idx) != Game::Lua::TYPE_STRING)
        return 0;
    const char *s = Game::Lua::ToString(L, idx);
    if (s == nullptr)
        return 0;
    if (const char *m = std::strstr(s, "item:"))
        return std::atoi(m + 5);
    return std::atoi(s);
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
    const int itemID = ParseItemArg(L, 1);
    const bool cached =
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), false) != nullptr);
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
        (itemID > 0) && (CacheFetch(static_cast<uint32_t>(itemID), false) != nullptr);
    Game::Lua::PushBoolean(L, cached ? 1 : 0);
    return 1;
}

// If the item is already cached, the engine won't invoke our callback —
// no SMSG round-trip happens. Fire ITEM_DATA_LOAD_RESULT ourselves so
// addons get the same notification regardless of cache state, matching
// modern API semantics.
static void RequestAndMaybeNotify(uint32_t itemID) {
    const bool wasCached = (CacheFetch(itemID, false) != nullptr);
    CacheFetch(itemID, true);
    if (wasCached) {
        const int slot = Event::Custom::Register(kItemDataLoadResult);
        if (slot >= 0)
            Event::Custom::Fire_DD(slot, static_cast<int>(itemID), 1);
    }
}

static int __fastcall Script_RequestLoadItemDataByID(void *L) {
    const int itemID = ParseItemArg(L, 1);
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

    // Seed the cache so `Event::Custom::RetryAll` (driven by our hook on
    // `Frame::RegisterEvent`) knows to claim a slot for this name. The
    // event table isn't populated yet when this fires, so the call returns
    // -1; later attempts via `RetryAll` or any direct `Register` call get
    // it. Once claimed, the slot is cached and reused.
    Event::Custom::Register(kItemDataLoadResult);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::Data
