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

// Player state queries that modern WoW exposes as no-arg globals
// (`IsMounted`, `IsStealthed`, etc.) but vanilla 1.12 doesn't bind
// to Lua. The underlying data IS in the engine — broadcast in
// UpdateFields for some, tracked locally for others — we just
// read it directly off the player descriptor / object.

#include "Game.h"
#include "Offsets.h"
#include "aura/Data.h"
#include "spell/Lookup.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Unit::State {

namespace {

using CancelAuraSend_t = void(__fastcall *)(int spellID);

uint32_t PlayerMovementFlags() {
    auto *player = Unit::Identity::PlayerObject();
    if (player == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        player + Offsets::OFF_PLAYER_MOVEMENT_FLAGS);
}

uint32_t MountDisplayID(const uint8_t *desc) {
    if (desc == nullptr)
        return 0;
    return *reinterpret_cast<const uint32_t *>(
        desc + Offsets::OFF_UNIT_FIELD_MOUNTDISPLAYID);
}

// `IsMounted()` — true iff the player is currently mounted. Reads
// `UNIT_FIELD_MOUNTDISPLAYID` from the descriptor; non-zero means
// the engine is rendering a mount under the player.
int __fastcall Script_IsMounted(void *L) {
    auto *desc = Unit::Identity::PlayerDescriptor();
    Game::Lua::PushBool(L, MountDisplayID(desc) != 0);
    return 1;
}

// `Dismount()` — drops the player's current mount. Walks the buff
// range scanning for a spell with `SPELL_AURA_MOUNTED` (`78`) in any
// effect slot, then sends `CMSG_CANCEL_AURA` for that spellID via
// the same direct sender `CancelSpellByID` uses.
//
// Vanilla 1.12 doesn't ship `Dismount` or any equivalent; mods that
// wanted programmatic dismount had to call `CancelPlayerBuff` on the
// right buff index by walking `GetPlayerBuff` themselves.
//
// No-op if the player isn't mounted (early-outs on the
// `UNIT_FIELD_MOUNTDISPLAYID` check before walking auras).
int __fastcall Script_Dismount(void *L) {
    auto *desc = Unit::Identity::PlayerDescriptor();
    if (MountDisplayID(desc) == 0)
        return 0;

    auto *player = Unit::Identity::PlayerObject();
    if (player == nullptr)
        return 0;

    for (int slot = 0; slot < Offsets::UNIT_AURA_BUFF_COUNT; ++slot) {
        if (!Aura::Data::IsSlotPopulated(player, slot))
            continue;
        const uint32_t spellID = Aura::Data::ReadSpellID(player, slot);
        const uint8_t *record = Spell::Lookup::RecordForID(static_cast<int>(spellID));
        if (record == nullptr)
            continue;

        auto *auraTypes = reinterpret_cast<const int32_t *>(
            record + Offsets::OFF_SPELL_RECORD_EFFECT_APPLY_AURA_NAME);
        for (int eff = 0; eff < Offsets::SPELL_RECORD_EFFECT_COUNT; ++eff) {
            if (auraTypes[eff] != Offsets::SPELL_AURA_MOUNTED)
                continue;
            auto fn = reinterpret_cast<CancelAuraSend_t>(
                static_cast<uintptr_t>(Offsets::FUN_CANCEL_AURA_SEND));
            fn(static_cast<int>(spellID));
            return 0;
        }
    }
    return 0;
}

// `IsStealthed()` — true iff the player is currently stealthed
// (Rogue Stealth / Druid Prowl). Checks `PLAYER_FIELD_VIS_BYTES`
// bit 0x02. Mount also sets that bit, so we AND-gate with
// `mountDisplayID == 0` to disambiguate. See the comment on
// `OFF_PLAYER_FIELD_VIS_BYTES` in Offsets.h for the full bit map
// and known limitations (untested for shapeshift forms).
int __fastcall Script_IsStealthed(void *L) {
    auto *desc = Unit::Identity::PlayerDescriptor();
    bool stealthed = false;
    if (desc != nullptr) {
        const uint32_t visBytes = *reinterpret_cast<const uint32_t *>(
            desc + Offsets::OFF_PLAYER_FIELD_VIS_BYTES);
        stealthed = (visBytes & Offsets::PLAYER_VIS_BIT_STEALTH) != 0
                    && MountDisplayID(desc) == 0;
    }
    Game::Lua::PushBool(L, stealthed);
    return 1;
}

// `IsFalling()` — true iff the player is currently mid-fall or
// jumping. Reads the local CGPlayer movement-flags word at
// `+0x9E8` and tests the FALLING / FALLING_FAR bits. Modern
// `IsFalling` returns true for the entire airtime of a jump — we
// match by accepting either bit (FALLING during the active jump,
// FALLING_FAR for sustained falls).
int __fastcall Script_IsFalling(void *L) {
    const uint32_t flags = PlayerMovementFlags();
    const uint32_t fallMask = Offsets::MOVEFLAG_FALLING |
                              Offsets::MOVEFLAG_FALLING_FAR;
    Game::Lua::PushBool(L, (flags & fallMask) != 0);
    return 1;
}

// `IsSwimming()` — true iff the player is currently swimming
// (in liquid, with the swim animation/control set). Single-bit
// check on the SWIMMING flag in the local movement-flags word.
int __fastcall Script_IsSwimming(void *L) {
    const uint32_t flags = PlayerMovementFlags();
    Game::Lua::PushBool(L, (flags & Offsets::MOVEFLAG_SWIMMING) != 0);
    return 1;
}

// `IsAssistingRitual()` — true iff the player is currently clicking
// a warlock summoning portal (the channel-on-GameObject state where
// movement breaks the channel and there's no castbar UI in vanilla).
//
// Distinct from spell channeling: this fires for the *participant*
// who clicked the portal, not the warlock who cast Ritual of
// Summoning. The warlock's own channel doesn't set this state.
//
// Detection: the engine writes the warlock's channel spell ID (698 =
// Ritual of Summoning) into the clicker's UNIT_CHANNEL_SPELL, AND
// sets a CGPlayer-local pointer at +0xB4 to the portal GameObject.
// Either signal alone is ambiguous:
//   - UNIT_CHANNEL_SPELL alone matches the warlock's own channel
//     (and any real spell channel — fishing, Mind Flay, etc.).
//   - The +0xB4 pointer alone matches any spell cast that targets a
//     GameObject (notably mining, which is a regular cast on an ore
//     node — desc[0x228] stays 0).
// The conjunction is portal-clicker-specific in 1.12.
//
// Local-player only: the +0xB4 pointer lives on CGPlayer, not in
// the broadcast UpdateField descriptor, so we can't read it for
// `target` / `party*` / etc. Modern WoW has no exact analog —
// vanilla's summoning ritual has no UI surface, which is what
// motivates this function.
int __fastcall Script_IsAssistingRitual(void *L) {
    auto *player = Unit::Identity::PlayerObject();
    if (player == nullptr) {
        Game::Lua::PushBool(L, 0);
        return 1;
    }
    auto *desc = Unit::Identity::PlayerDescriptor();
    if (desc == nullptr) {
        Game::Lua::PushBoolean(L, 0);
        return 1;
    }
    const uint32_t channelSpell = *reinterpret_cast<const uint32_t *>(
        desc + Offsets::OFF_UNIT_FIELD_CHANNEL_SPELL);
    const void *castGameObject = *reinterpret_cast<const void *const *>(
        player + Offsets::OFF_CGPLAYER_CAST_GAMEOBJECT_PTR);
    Game::Lua::PushBoolean(L,
        (channelSpell != 0 && castGameObject != nullptr));
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("IsMounted", &Script_IsMounted);
    Game::Lua::RegisterGlobalFunction("Dismount", &Script_Dismount);
    Game::Lua::RegisterGlobalFunction("IsStealthed", &Script_IsStealthed);
    Game::Lua::RegisterGlobalFunction("IsFalling", &Script_IsFalling);
    Game::Lua::RegisterGlobalFunction("IsSwimming", &Script_IsSwimming);
    Game::Lua::RegisterGlobalFunction("IsAssistingRitual",
                                       &Script_IsAssistingRitual);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Unit::State
