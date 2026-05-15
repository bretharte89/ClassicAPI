// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU Lesser General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU Lesser General Public License for more details.

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"

#include <cstdint>

namespace Unit::ClassBase {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// `UnitClassBase(unit) → (classFile, classID)` — returns the locale-
// independent class token ("WARRIOR", "PALADIN", "HUNTER", "ROGUE",
// "PRIEST", "SHAMAN", "MAGE", "WARLOCK", "DRUID") plus the numeric
// classID (1-9 in vanilla), or `(nil, nil)` if the unit can't be
// resolved.
//
// Modern addons use this for class detection because `UnitClass`
// returns a localized name — usable for display but unsafe as a
// dictionary key when an addon runs across locales. The English
// token comes straight from `ChrClasses.dbc::Filename` at record
// `+0x38`, so it matches the GLOBAL_STRINGS / addon-side conventions
// (`LOCALIZED_CLASS_NAMES_MALE[token]`).
//
// The classID second return is a vanilla extension — modern's
// `UnitClassBase` returns the token only — but it's free here
// (we already read the byte) and saves callers from also calling
// `UnitClass(unit)` just to get the integer. Matches the values
// `UnitClass`'s third return surfaces: 1=Warrior, 2=Paladin,
// 3=Hunter, 4=Rogue, 5=Priest, 7=Shaman, 8=Mage, 9=Warlock,
// 11=Druid (6 and 10 are post-vanilla).
//
// Resolves the unit via the engine's token resolver (so `"player"`,
// `"target"`, `"partyN"`, `"raidN"`, `"mouseover"` all work) and
// reads the class byte from `UNIT_FIELD_BYTES_0` at descriptor
// `+0x79`. Looks the byte up in `ChrClasses.dbc`. Works for any
// synced unit; remote players' class is broadcast as an UpdateField.
//
// Returns `(nil, nil)` for:
//   - missing / unresolvable units (`target` with nothing targeted,
//     empty `partyN` slot, unloaded descriptor)
//   - non-player units (creatures have a class byte too but it
//     indexes a different table — `ChrClasses.dbc[0]` is unused
//     and `ChrClasses.dbc[N>9]` is out of range, so out-of-range
//     class bytes naturally return nil here)
//   - `ChrClasses.dbc` rows with no Filename (shouldn't happen on
//     real client data but we handle defensively)
//
// Throws a Lua error on missing / non-string `unit` argument — same
// semantics as `UnitClass` itself.
int __fastcall Script_UnitClassBase(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: UnitClassBase(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr) {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
        return 2;
    }

    auto resolve = reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *unit = static_cast<const uint8_t *>(resolve(token));
    if (unit == nullptr) {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
        return 2;
    }
    auto *desc = *reinterpret_cast<const uint8_t *const *>(
        unit + Offsets::OFF_UNIT_DESCRIPTOR);
    if (desc == nullptr) {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
        return 2;
    }
    const uint8_t classByte = *(desc + Offsets::OFF_UNIT_DESCRIPTOR_CLASS_BYTE);
    if (classByte == 0) {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
        return 2;
    }

    const char *classFile = DBC::StringField(
        Offsets::VAR_CHRCLASSES_RECORDS,
        Offsets::VAR_CHRCLASSES_COUNT,
        classByte,
        Offsets::OFF_CHRCLASSES_FILENAME);
    if (classFile == nullptr) {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
        return 2;
    }
    Game::Lua::PushString(L, classFile);
    Game::Lua::PushNumber(L, static_cast<double>(classByte));
    return 2;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("UnitClassBase", &Script_UnitClassBase);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

} // namespace Unit::ClassBase
