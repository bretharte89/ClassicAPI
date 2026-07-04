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

// `PLAYER_EQUIPMENT_CHANGED(equipmentSlot, hasCurrent)` — the WotLK-era
// per-slot paperdoll event. Vanilla only fires UNIT_INVENTORY_CHANGED for
// the player, which doesn't say WHICH slot changed; gear trackers and stat
// sheets built against 3.3+ gate their refresh on this event.
//
// Fully event-driven via the engine's own descriptor-field observer system
// (see the FUN_DESC_OBSERVER_REGISTER block in Offsets.h). The engine's
// inventory setup (FUN_INV_OBSERVER_SETUP, once per enter-world) registers
// change observers for the bag/backpack/bank/keyring GUID fields of the
// player descriptor — but never for the 19 equipment slots (0x4A8..0x538).
// We co-hook that setup and register our own observer per equipment field,
// mirroring the engine's registration shape exactly (bank 4, size 8). The
// dispatcher invokes us only when a slot's item GUID actually changed
// (per-node mirror memcmp), so there's no polling and no diff bookkeeping
// on our side, and the nodes share the engine observers' lifetime (they
// die with the player object; the setup re-runs each enter-world).
//
// `equipmentSlot` is the 1-based Lua inventory slot (1 = head … 19 =
// tabard), matching modern. `hasCurrent` is 1 when the slot now holds an
// item and nil when it's empty (the engine dispatcher has no `%b`; 1/nil
// keeps the idiomatic `if hasCurrent then` working — see FireIdSuccess).

#include "Game.h"
#include "Offsets.h"
#include "event/Custom.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Player::Equipment {

namespace {

constexpr const char *kEvtEquipmentChanged = "PLAYER_EQUIPMENT_CHANGED";
const Event::Custom::AutoReserve _r{kEvtEquipmentChanged};

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// Whether the slot currently holds an item — read from the player's
// inventory-manager GUID array, the same source the engine's own bag
// observer (FUN_004F8DB0) treats as the post-change truth when its field
// observer fires.
bool SlotHasItem(int slot0Based) {
    auto resolve =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<const uint8_t *>(resolve("player"));
    if (player == nullptr)
        return false;
    const uint8_t *invMgr = player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;
    const uint32_t count = *reinterpret_cast<const uint32_t *>(
        invMgr + Offsets::OFF_INVMGR_SLOT_COUNT);
    if (static_cast<uint32_t>(slot0Based) >= count)
        return false;
    const uint64_t *guids = *reinterpret_cast<const uint64_t *const *>(
        invMgr + Offsets::OFF_INVMGR_GUID_ARRAY);
    return guids != nullptr && guids[slot0Based] != 0;
}

// Descriptor-field observer callback — ABI per the Offsets.h note:
// __fastcall(fieldOffset /*ecx*/, size /*edx*/, guidLo, guidHi, oldValue*,
// userArg), return 1, callee cleans 0x10. Invoked by the dispatcher only
// when the watched equipment GUID field actually changed.
int __fastcall EquipSlotChanged_cb(uint32_t fieldOffset, uint32_t /*size*/,
                                   uint32_t /*guidLo*/, uint32_t /*guidHi*/,
                                   const uint32_t * /*oldValue*/,
                                   void * /*userArg*/) {
    const int slot0 =
        static_cast<int>(fieldOffset - Offsets::OFF_DESC_PLAYER_EQUIP_FIRST) >> 3;
    if (slot0 >= 0 && slot0 < Offsets::DESC_PLAYER_EQUIP_SLOTS) {
        Event::Custom::FireIdSuccess(Event::Custom::Lookup(kEvtEquipmentChanged),
                                     slot0 + 1, SlotHasItem(slot0));
    }
    return 1;
}

// Co-hook on the engine's inventory observer setup: after it registers the
// bag-range observers, register ours for the equipment range it skips.
// Same registrar, same shape — only the field offsets differ.
using ObserverSetup_t = void(__fastcall *)();
ObserverSetup_t g_origSetup = nullptr;

using RegisterObserver_t = void(__fastcall *)(
    int bank, uint32_t fieldOffset, uint32_t guidLo, uint32_t guidHi, int size,
    const void *callback, void *userArg1, void *userArg2);

void __fastcall ObserverSetup_h() {
    g_origSetup();
    const uint64_t guid = Unit::Identity::PlayerGuid();
    if (guid == 0)
        return;
    auto reg = reinterpret_cast<RegisterObserver_t>(
        Offsets::FUN_DESC_OBSERVER_REGISTER);
    for (int slot = 0; slot < Offsets::DESC_PLAYER_EQUIP_SLOTS; ++slot) {
        reg(Offsets::DESC_OBSERVER_BANK_PLAYER,
            Offsets::OFF_DESC_PLAYER_EQUIP_FIRST + slot * 8,
            static_cast<uint32_t>(guid), static_cast<uint32_t>(guid >> 32),
            /*size*/ 8, reinterpret_cast<const void *>(&EquipSlotChanged_cb),
            nullptr, nullptr);
    }
}

static const Game::HookAutoRegister _setupHook{
    Offsets::FUN_INV_OBSERVER_SETUP,
    reinterpret_cast<void *>(&ObserverSetup_h),
    reinterpret_cast<void **>(&g_origSetup)};

} // namespace

} // namespace Player::Equipment
