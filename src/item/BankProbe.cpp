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

// Diagnostic only — not part of the public API. Registered as the global
// `_classicapi_BankProbe` Lua function. Used to investigate whether the
// engine clears bank slot data when the bank window closes, or only
// flips the banker-GUID gate (in which case we can bypass the gate by
// reading the GUID array directly).
//
// Test sequence:
//   1. Fresh login, never opened bank: expect gateOpen=false, count=0
//   2. Open bank: expect gateOpen=true, count=N, guids populated
//   3. Close bank: KEY OBSERVATION — does count drop to 0 (data cleared)
//      or stay at N (data persists, gate just closed)?
//
// If data persists, ship Option D (direct GUID-array read for bank
// slots, bypasses the gate). If cleared, fall back to Option B'
// (DLL-side cache populated via FireEvent hook).

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstdio>

namespace Item::BankProbe {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);

// `_classicapi_BankProbe()` — returns four values:
//   gateOpen   (bool)   — is `[0x00BDD038]` (the banker-GUID gate) non-zero?
//   gateGuid   (string) — hex of the gate GUID, "0xHHHHHHHHLLLLLLLL"
//   bankCount  (number) — non-zero GUIDs in linear slots 39..62 (main bank)
//   bankGuids  (string) — comma-separated "slot:0xHHH..." for populated slots
static int __fastcall Script_BankProbe(void *L) {
    auto ResolveUnitToken =
        reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN);
    auto *player = static_cast<uint8_t *>(ResolveUnitToken("player"));
    if (player == nullptr) {
        Game::Lua::PushNil(L);
        return 1;
    }
    auto *invMgr = player + Offsets::OFF_PLAYER_INVENTORY_MANAGER;

    // Gate at [0x00BDD038] — 8-byte qword (GUID of the active banker NPC,
    // or 0 when no bank window is open).
    const uint32_t gateLo = *reinterpret_cast<const uint32_t *>(0x00BDD038);
    const uint32_t gateHi = *reinterpret_cast<const uint32_t *>(0x00BDD03C);
    const bool gateOpen = (gateLo | gateHi) != 0;

    char gateBuf[32];
    std::snprintf(gateBuf, sizeof(gateBuf), "0x%08x%08x", gateHi, gateLo);

    // GUID array at invMgr+0x04. Each slot is 8 bytes (low + high dword).
    auto *guidArray =
        *reinterpret_cast<const uint64_t *const *>(invMgr + 4);

    char details[2048];
    char *p = details;
    int remaining = static_cast<int>(sizeof(details));
    int nonZero = 0;
    details[0] = 0;

    if (guidArray != nullptr) {
        // Linear slots 39..62 = main bank (24 slots). See PackBagSlot
        // (`0x004F9820`): lua bagID=-1 → after dec → -2 → linear += 0x27.
        for (int slot = 39; slot <= 62 && remaining > 64; slot++) {
            const uint64_t guid = guidArray[slot];
            if (guid == 0)
                continue;
            const uint32_t lo = static_cast<uint32_t>(guid);
            const uint32_t hi = static_cast<uint32_t>(guid >> 32);
            const int written = std::snprintf(p, remaining,
                "%s%d:0x%08x%08x", nonZero ? "," : "", slot, hi, lo);
            if (written > 0 && written < remaining) {
                p += written;
                remaining -= written;
                nonZero++;
            }
        }
    }

    Game::Lua::PushBoolean(L, gateOpen ? 1 : 0);
    Game::Lua::PushString(L, gateBuf);
    Game::Lua::PushNumber(L, static_cast<double>(nonZero));
    Game::Lua::PushString(L, details);
    return 4;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("_classicapi_BankProbe", &Script_BankProbe);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Item::BankProbe
