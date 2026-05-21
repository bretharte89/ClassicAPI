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

#include <cstdint>

namespace Unit::ShapeshiftForm {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// `GetShapeshiftFormID()` — returns the player's current shapeshift
// form ID from vanilla 1.12.1's `SpellShapeshiftForm.dbc`:
//   Druid:   1 Cat, 2 Tree, 3 Travel, 4 Aquatic, 5 Bear,
//            8 Dire Bear, 31 Moonkin
//   Warrior: 17 Battle, 18 Defensive, 19 Berserker
//   Shaman:  16 Ghost Wolf
//   Rogue:   30 Stealth
//   Priest:  28 Shadowform, 32 Spirit of Redemption
// Turtle WoW extends the DBC with custom rows: 9 Tree of Life Form,
// 11 Swift Travel Form. Both slots are empty on Blizzard 1.12.1.
// Returns `0` when the player isn't shapeshifted. **These are the
// 1.12 values; modern WoW renumbered the table** (e.g. modern
// Travel=17, Tree=35) — addons backporting modern code shouldn't
// reuse the constants verbatim.
//
// Reads byte 2 of `UNIT_BYTES_1` (descriptor `+0x212`) on the local
// player. Derived from `Script_GetShapeshiftFormInfo` at `0x004B45C0`,
// which uses the same byte to compare against each form-spell's
// effect-encoded form ID when answering "is this form active". The
// engine sets it from `SMSG_UPDATE_OBJECT` aura/form updates, so it
// tracks live state without us needing to listen on any event.
//
// Vanilla 1.12 has no native `GetShapeshiftFormID` — only the
// 1-based bar-index `GetShapeshiftFormInfo`. Modern API lets callers
// key behavior on the DBC ID directly (`if formID == CAT_FORM then`)
// without needing to iterate the bar.
int __fastcall Script_GetShapeshiftFormID(void *L) {
    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<const uint8_t *>(resolve("player"));
    if (player == nullptr) {
        Game::Lua::PushNumber(L, 0);
        return 1;
    }
    auto *fields = *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_UNIT_DESCRIPTOR);
    if (fields == nullptr) {
        Game::Lua::PushNumber(L, 0);
        return 1;
    }
    const uint32_t bytes1 = *reinterpret_cast<const uint32_t *>(
        fields + Offsets::OFF_UNIT_FIELD_BYTES_1);
    Game::Lua::PushNumber(L, static_cast<double>((bytes1 >> 16) & 0xFF));
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetShapeshiftFormID", &Script_GetShapeshiftFormID);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Unit::ShapeshiftForm
