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

// `ClosestUnitPosition(creatureID)` → `xPos, yPos, distance` — the world
// position of the nearest creature with the given NPC (creature-template)
// ID, and its distance from the player in yards. Returns nothing when no
// matching creature is found (the modern `MayReturnNothing` contract).
//
// Semantic note vs. retail: modern WoW's `ClosestUnitPosition` reads a
// static client-side spawn database and only works for starting-zone mobs
// (a tutorial/help helper). Vanilla 1.12 has no such database, so we
// implement the useful, 1.12-native reading: the closest creature of that
// entry that is CURRENTLY VISIBLE (in the client's object manager /
// broadcast window). That covers "where's the nearest <mob> I can see"
// but, unlike retail, can't point at un-synced spawns across the zone.
//
// Reuses the object-manager enumeration pattern from Loot::Nearby and the
// creature-entry extraction from guid/CreatureInfo (bits 24-47 of a
// creature GUID hold the template entry).

#include "Game.h"
#include "Offsets.h"
#include "guid/Guid.h"
#include "unit/Position.h"

#include <cmath>
#include <cstdint>

namespace Unit::ClosestPosition {

namespace {

// `ClntObjMgrEnumVisibleObjects(cb, ctx)` — walks the visible-object
// table; the callback's dummy EDX slot absorbs the second __fastcall
// register so the trailing `uint64_t guid` lands where the engine puts it.
// Return non-zero to keep iterating.
using EnumCallback_t = int(__fastcall *)(void *ctx, void *unusedEdx,
                                         uint64_t guid);
using EnumVisibleObjects_t = int(__fastcall *)(EnumCallback_t cb, void *ctx);

// `ClntObjMgrObjectPtr(typeMask, dbg, guidLo, guidHi, dbgCode)` — resolves
// a GUID to its live `CGObject_C *`, filtered by typeMask; NULL if no live
// object matches.
using ObjectPtr_t = void *(__fastcall *)(uint32_t typeMask, const char *dbg,
                                         uint32_t guidLo, uint32_t guidHi,
                                         int dbgCode);

struct ScanCtx {
    uint32_t wantEntry;
    float px, py, pz; // player position
    bool found;
    float bestX, bestY, bestDistSq;
};

int __fastcall ScanCallback(ScanCtx *ctx, void * /*unusedEdx*/, uint64_t guid) {
    // Entry check first — it's a pure GUID bit-read, so we skip the object
    // resolve for the (vast majority of) creatures that don't match.
    if (!Guid::IsCreature(guid))
        return 1;
    const uint32_t entry = static_cast<uint32_t>((guid >> 24) & 0xFFFFFFu);
    if (entry != ctx->wantEntry)
        return 1;

    auto ObjectPtr =
        reinterpret_cast<ObjectPtr_t>(Offsets::FUN_CLNT_OBJ_MGR_OBJECT_PTR);
    void *obj = ObjectPtr(Offsets::TYPEMASK_UNIT, nullptr,
                          static_cast<uint32_t>(guid),
                          static_cast<uint32_t>(guid >> 32), 0);
    if (obj == nullptr)
        return 1;

    float pos[3];
    if (!Unit::Position::Read(obj, pos))
        return 1;

    const float dx = pos[0] - ctx->px;
    const float dy = pos[1] - ctx->py;
    const float dz = pos[2] - ctx->pz;
    const float distSq = dx * dx + dy * dy + dz * dz;
    if (!ctx->found || distSq < ctx->bestDistSq) {
        ctx->found = true;
        ctx->bestX = pos[0];
        ctx->bestY = pos[1];
        ctx->bestDistSq = distSq;
    }
    return 1;
}

int __fastcall Script_ClosestUnitPosition(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: ClosestUnitPosition(creatureID)");
        return 0;
    }
    const auto wantEntry = static_cast<uint32_t>(Game::Lua::ToNumber(L, 1));
    if (wantEntry == 0)
        return 0; // no valid entry → nothing

    // The enumerator derefs the object manager unconditionally; it's NULL
    // during glue / character-select. Bail with nothing rather than crash.
    if (*reinterpret_cast<void *volatile *>(Offsets::VAR_LOCAL_PLAYER_PTR) ==
        nullptr)
        return 0;

    float ppos[3];
    if (!Unit::Position::Read(Unit::Position::ResolveToken("player"), ppos))
        return 0;

    ScanCtx ctx{wantEntry, ppos[0], ppos[1], ppos[2], false, 0.0f, 0.0f, 0.0f};
    auto Enum = reinterpret_cast<EnumVisibleObjects_t>(
        Offsets::FUN_CLNT_OBJ_MGR_ENUM_VISIBLE_OBJECTS);
    Enum(reinterpret_cast<EnumCallback_t>(&ScanCallback), &ctx);
    if (!ctx.found)
        return 0; // no visible creature of that entry → nothing

    Game::Lua::PushNumber(L, static_cast<double>(ctx.bestX));
    Game::Lua::PushNumber(L, static_cast<double>(ctx.bestY));
    Game::Lua::PushNumber(L, std::sqrt(static_cast<double>(ctx.bestDistSq)));
    return 3;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("ClosestUnitPosition",
                                      &Script_ClosestUnitPosition);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Unit::ClosestPosition
