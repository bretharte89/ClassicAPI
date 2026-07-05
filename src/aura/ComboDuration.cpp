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

// See ComboDuration.h for the contract and the protocol findings that
// motivate the design (the server never transmits target-debuff
// durations in 1.12).

#include "ComboDuration.h"

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "net/PacketReader.h"
#include "spell/Mod.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Aura::ComboDuration {

namespace {

using Net::CDataStore;

uint32_t NowMs() {
    using TickCount_t = uint32_t(__fastcall *)();
    return reinterpret_cast<TickCount_t>(
        static_cast<uintptr_t>(Offsets::FUN_OS_TICKCOUNT_MS))();
}

// ---- Combo-point snapshot (NetClient send co-hook) ------------------------

// The player's live combo points, read the same way Script_GetComboPoints
// (0x0051A190) does: the CP byte on the CGPlayer +0xE68 sub-struct. The
// player is resolved via the non-throwing object resolver (guid comes from
// the login global; 0 pre-world -> resolver returns null -> 0 CP), so this
// is safe from any context the send hook can fire in.
uint8_t CurrentComboPoints() {
    const uint64_t guid = Unit::Identity::PlayerGuid();
    if (guid == 0)
        return 0;
    using ResolveByGUID_t = void *(__fastcall *)(uint32_t typeMask,
                                                 const char *debugName,
                                                 uint32_t guidLo, uint32_t guidHi,
                                                 int line);
    auto resolve =
        reinterpret_cast<ResolveByGUID_t>(Offsets::FUN_OBJECT_RESOLVE_BY_GUID);
    auto *player = static_cast<const uint8_t *>(
        resolve(Offsets::OBJ_TYPE_PLAYER, "ClassicAPI",
                static_cast<uint32_t>(guid), static_cast<uint32_t>(guid >> 32),
                0x1029));
    if (player == nullptr)
        return 0;
    const uint8_t *info = *reinterpret_cast<const uint8_t *const *>(
        player + Offsets::OFF_CGPLAYER_INFO);
    if (info == nullptr)
        return 0;
    return *(info + Offsets::OFF_CGPLAYER_COMBO_POINTS);
}

// One cast in flight at a time is the practical case (a second finisher
// can't be cast until the server consumes the first one's combo points),
// so a single slot with a spellID match and a short TTL suffices.
struct Capture {
    uint32_t spellId;
    uint8_t comboPoints;
    uint32_t capturedMs;
};
Capture g_capture{0, 0, 0};

// Worst-case send -> SMSG_SPELL_GO roundtrip. Past this the snapshot is
// stale (the cast failed / was rejected) and must not scale a later cast.
constexpr uint32_t kCaptureTtlMs = 2000;

int ConsumeCapturedCP(uint32_t spellId) {
    const Capture c = g_capture;
    g_capture = {0, 0, 0};
    if (c.spellId != spellId || c.comboPoints == 0)
        return 0;
    if (NowMs() - c.capturedMs >= kCaptureTtlMs)
        return 0;
    return c.comboPoints;
}

// Co-hook on the NetClient send — the one funnel every outgoing CMSG
// passes through, including casts released from nampower's spell queue
// (which bypass any co-hook on Spell_C_CastSpell itself via trampoline
// re-entry, but still build and send CMSG_CAST_SPELL through the engine).
// Snapshot the combo points at the moment the cast request leaves: the
// server consumes them before SMSG_SPELL_GO arrives, so reading at
// SpellGo time is too late.
using NetSend_t = void(__fastcall *)(void *conn, void *edx,
                                     CDataStore *packet);
NetSend_t g_origNetSend = nullptr;

void __fastcall NetSend_h(void *conn, void *edx, CDataStore *packet) {
    if (packet != nullptr) {
        const uint32_t saved = packet->m_read;
        const uint32_t opcode = Net::Read<uint32_t>(packet);
        if (opcode == Offsets::OP_CMSG_CAST_SPELL) {
            const uint32_t spellId = Net::Read<uint32_t>(packet);
            if (spellId != 0)
                g_capture = {spellId, CurrentComboPoints(), NowMs()};
        }
        packet->m_read = saved;
    }
    g_origNetSend(conn, edx, packet);
}

const Game::HookAutoRegister _sendHook{
    Offsets::FUN_NET_SEND, reinterpret_cast<void *>(&NetSend_h),
    reinterpret_cast<void **>(&g_origNetSend)};

// ---- Duration data (SpellDuration.dbc + Lua overrides) --------------------

// Lua-registered overrides for spells whose client DBC row is missing or
// wrong. Turtle's reworked Rip is the motivating case: its DurationIndex
// dangles (records[87] == NULL in Turtle's SpellDuration.dbc) because the
// rework lives entirely server-side, so the values must come from config.
struct Override {
    uint32_t spellId;
    int32_t baseMs;
    int32_t maxMs;
};
constexpr int kMaxOverrides = 64;
Override g_overrides[kMaxOverrides];
int g_overrideCount = 0;

const Override *FindOverride(uint32_t spellId) {
    for (int i = 0; i < g_overrideCount; ++i) {
        if (g_overrides[i].spellId == spellId)
            return &g_overrides[i];
    }
    return nullptr;
}

// Known values for spells whose duration row DANGLES — a non-zero
// DurationIndex pointing at a SpellDuration row the client doesn't have.
// Consulted ONLY when that exact condition holds, so the gate is the bug
// itself, not a server fingerprint: on a stock client every one of these
// resolves to a valid row and this table is never reached, and if the
// broken client data is ever fixed the DBC row takes over automatically.
//
// Turtle's reworked Rip is the known case: all six ranks point at
// SpellDuration row 87, which Turtle added on the SERVER only
// ({8000, 0, 18000} — live server data shows 10/12/14/16/18s at 1..5 CP,
// i.e. 8s + 2s per combo point); their client patch never shipped the
// row (the client table jumps 86 -> 105). Verified the only dangling
// duration index in the entire client Spell.dbc.
constexpr Override kDanglingRowKnown[] = {
    {1079, 8000, 18000}, {9492, 8000, 18000}, {9493, 8000, 18000},
    {9752, 8000, 18000}, {9894, 8000, 18000}, {9896, 8000, 18000},
};

const Override *FindDanglingRowKnown(uint32_t spellId) {
    for (const auto &k : kDanglingRowKnown) {
        if (k.spellId == spellId)
            return &k;
    }
    return nullptr;
}

// `C_UnitAuras.RegisterComboDuration(spellID, baseSeconds, maxSeconds)`
// -> true on success. Re-registering a spellID replaces its entry.
// Values are in seconds to match the C_UnitAuras duration convention;
// the formula applied is the server's own:
// `base + (max - base) * comboPoints / 5`.
int __fastcall Script_RegisterComboDuration(void *L) {
    if (!Game::Lua::IsNumber(L, 1) || !Game::Lua::IsNumber(L, 2) ||
        !Game::Lua::IsNumber(L, 3)) {
        Game::Lua::Error(
            L, "Usage: C_UnitAuras.RegisterComboDuration(spellID, "
               "baseSeconds, maxSeconds)");
        return 0;
    }
    const auto spellId = static_cast<uint32_t>(Game::Lua::ToNumber(L, 1));
    const auto baseMs = static_cast<int32_t>(Game::Lua::ToNumber(L, 2) * 1000.0);
    const auto maxMs = static_cast<int32_t>(Game::Lua::ToNumber(L, 3) * 1000.0);
    if (spellId == 0 || baseMs < 0 || maxMs <= 0) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    for (int i = 0; i < g_overrideCount; ++i) {
        if (g_overrides[i].spellId == spellId) {
            g_overrides[i] = {spellId, baseMs, maxMs};
            Game::Lua::PushBool(L, true);
            return 1;
        }
    }
    if (g_overrideCount >= kMaxOverrides) {
        Game::Lua::PushBool(L, false);
        return 1;
    }
    g_overrides[g_overrideCount++] = {spellId, baseMs, maxMs};
    Game::Lua::PushBool(L, true);
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_UnitAuras", "RegisterComboDuration",
                                     &Script_RegisterComboDuration);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

uint32_t TryComboScaledMs(const uint8_t *spellRecord, uint32_t spellId) {
    if (spellRecord == nullptr || spellId == 0)
        return 0;
    const int cp = ConsumeCapturedCP(spellId);
    if (cp <= 0 || cp > 5)
        return 0;

    int32_t baseMs = 0;
    int32_t maxMs = 0;
    if (const Override *o = FindOverride(spellId)) {
        baseMs = o->baseMs;
        maxMs = o->maxMs;
    } else {
        const int idx = *reinterpret_cast<const int *>(
            spellRecord + Offsets::OFF_SPELL_RECORD_DURATION_INDEX);
        const uint8_t *row =
            DBC::Record(Offsets::VAR_SPELL_DURATION_RECORDS,
                        Offsets::VAR_SPELL_DURATION_COUNT,
                        static_cast<uint32_t>(idx));
        if (row != nullptr) {
            baseMs = *reinterpret_cast<const int32_t *>(row + 0x4);
            maxMs = *reinterpret_cast<const int32_t *>(row + 0xC);
        } else if (idx != 0) {
            // Dangling duration index — the client data is broken for this
            // spell. Fall back to the built-in known values (Turtle Rip).
            const Override *k = FindDanglingRowKnown(spellId);
            if (k == nullptr)
                return 0;
            baseMs = k->baseMs;
            maxMs = k->maxMs;
        } else {
            return 0; // no duration row at all
        }
    }

    // The server scales only when the row has a combo range: base != max
    // ((v)mangos CalculateSpellDuration). base < 0 is the "infinite"
    // sentinel; fixed-duration rows (base == max) take the regular path.
    if (baseMs < 0 || maxMs <= 0 || baseMs == maxMs)
        return 0;

    int32_t durationMs = baseMs + (maxMs - baseMs) * cp / 5;
    if (durationMs <= 0)
        return 0;

    // Player duration SpellMods apply on top of the combo-scaled value,
    // matching the server's order (scale, then ApplySpellMod).
    durationMs = static_cast<int32_t>(Spell::Mod::Apply(
        spellRecord, Offsets::SPELLMOD_OP_DURATION,
        static_cast<float>(durationMs)));
    return durationMs > 0 ? static_cast<uint32_t>(durationMs) : 0;
}

} // namespace Aura::ComboDuration
