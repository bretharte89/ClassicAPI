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

// Cast / channel info — `UnitCastingInfo`, `CastingInfo`,
// `UnitChannelInfo`, `ChannelInfo`.
//
// Vanilla 1.12 stores NO cast start/end times anywhere readable (unlike
// 3.3.5+, which keeps them per-unit on the CGUnit). The cast bar is
// Lua-driven off `SPELLCAST_START(duration)` + `GetTime()`. So we
// self-track the LOCAL PLAYER's cast/channel on `WorldTick`:
//   - cast: `VAR_CURRENT_CAST_SPELL` (the player cast-bar spellID
//     global). On a 0->nonzero transition we stamp startMs = now and
//     endMs = now + base cast time (SpellCastTimes.dbc). Instant casts
//     (cast time 0) and channels are filtered out here — a channel has
//     no cast time and surfaces through the channel field instead.
//   - channel: the player's broadcast `UNIT_FIELD_CHANNEL_SPELL`
//     (descriptor +0x228). endMs = now + base channel duration
//     (SpellDuration.dbc).
// `now` is the engine ms clock (`FUN_OS_TICKCOUNT_MS`), the same source
// Lua's `GetTime()` scales by 0.001 — so startMs/endMs are directly
// comparable to `GetTime()*1000` in addon progress math.
//
// Other units have no engine-stored cast record, so we capture their casts
// from the `SMSG_SPELL_START` packet (the only place the client sees
// another unit's cast + a server-authoritative cast time) into a per-caster
// cache — see the SpellStart co-hook below. Scope:
//   - Player casts/channels: full timing (self-tracked, exact engine math).
//   - Other units' regular CASTS: full timing while within the cast window,
//     from the SpellStart cache. No clean per-unit interrupt signal exists
//     in 1.12, so an interrupted remote cast lingers until its computed end.
//   - Other units' CHANNELS: the broadcast +0x228 field gates "channeling
//     now"; the SpellStart cache adds real start/end times when we observed
//     the channel begin (else spellID/name/texture with nil times).
//
// Modern-signature fields vanilla can't fill are structurally-correct
// placeholders: castID / castBarID = nil, notInterruptible = false,
// isTradeskill = false (no readable flag in 1.12), delayTimeMs = 0.

#include "Game.h"
#include "Offsets.h"
#include "dbc/Lookup.h"
#include "net/PacketReader.h"
#include "spell/Lookup.h"
#include "tick/WorldTick.h"
#include "unit/Identity.h"

#include <cstdint>

namespace Spell::Cast {

namespace {

using ResolveUnitToken_t = void *(__fastcall *)(const char *token);
using TickMs_t = uint32_t(__fastcall *)();
using GetCastTime_t = uint32_t(__fastcall *)(int spellID, int unit, int flag);
using GetDuration_t = int(__fastcall *)(const uint8_t *spellRecord, int unit, int skipMod);

// Spell.dbc record field offsets (mirrors Spell::Info's locals).
constexpr int OFF_NAME = 0x1E0;            // localized name[9]
constexpr int OFF_ICON_ID = 0x1D4;         // -> SpellIcon.dbc
constexpr int OFF_ATTRIBUTES = 0x18;       // u32 Attributes flags
constexpr int OFF_SUBRECORD_VALUE = 0x04;  // SpellIcon path field

// SPELL_ATTR_TRADESPELL — set on profession recipe casts (enchant,
// cooking with a cast, etc.). The version-stable `isTradeskill` source:
// 3.3.5's Script_UnitCastingInfo reads bit 0x20 of Attributes (at +0x10
// there, +0x18 here — the bit value is the same across builds).
constexpr uint32_t SPELL_ATTR_TRADESPELL = 0x20;

struct TrackedSpell {
    int spellID; // 0 = not casting / channeling
    int startMs;
    int endMs;
};

TrackedSpell g_cast{0, 0, 0};
TrackedSpell g_channel{0, 0, 0};
int g_lastCast = 0;
int g_lastChannel = 0;

int NowMs() { return static_cast<int>(reinterpret_cast<TickMs_t>(Offsets::FUN_OS_TICKCOUNT_MS)()); }

void *Resolve(const char *token) {
    return reinterpret_cast<ResolveUnitToken_t>(Offsets::FUN_RESOLVE_UNIT_TOKEN)(token);
}

// Effective cast time (ms) for the local player, 0 if instant. Uses the
// engine's own cast-time helper (base + level scaling + cast-time SpellMod
// op 10 + the caster's haste/cast-speed multiplier), so our end time
// matches the engine's cast bar exactly — including talents like a Mage's
// faster casts. Unit arg 0 = local player; flag 0 clamps negatives to 0.
int CastTimeMs(int spellID) {
    return static_cast<int>(
        reinterpret_cast<GetCastTime_t>(Offsets::FUN_GET_CAST_TIME)(spellID, 0, 0));
}

// Effective channel duration (ms) for the local player, 0 if none. Uses
// the engine's own duration helper (SpellDuration base + level scaling +
// duration SpellMod op 1), mirroring how the cast time is sourced, so the
// channel end time matches the engine. Unit arg 0 = local player.
int ChannelDurationMs(int spellID) {
    const uint8_t *rec = Spell::Lookup::RecordForID(spellID);
    if (rec == nullptr)
        return 0;
    const int ms = reinterpret_cast<GetDuration_t>(Offsets::FUN_GET_SPELL_DURATION)(rec, 0, 0);
    return ms > 0 ? ms : 0; // negative = "infinite"; channels are finite
}

// The local player's currently-channeled spellID (broadcast +0x228), or 0.
int PlayerChannelSpell() {
    void *p = Resolve("player");
    if (p == nullptr)
        return 0;
    auto *desc = *reinterpret_cast<const uint8_t *const *>(
        static_cast<const uint8_t *>(p) + Offsets::OFF_UNIT_DESCRIPTOR);
    if (desc == nullptr)
        return 0;
    return *reinterpret_cast<const int *>(desc + Offsets::OFF_UNIT_FIELD_CHANNEL_SPELL);
}

const char *SpellName(const uint8_t *rec) {
    const int locale = *reinterpret_cast<int *>(Offsets::VAR_LOCALE_INDEX);
    return *reinterpret_cast<const char *const *>(rec + OFF_NAME + locale * 4);
}

bool IsTradeskill(const uint8_t *rec) {
    return (*reinterpret_cast<const uint32_t *>(rec + OFF_ATTRIBUTES) & SPELL_ATTR_TRADESPELL) != 0;
}

// Spell icon texture path, or "" if there's no icon record. SpellIcon.dbc
// already stores the full "Interface\Icons\..." path (unlike item icons,
// which are bare filenames) — so it's used verbatim, no prefix.
const char *SpellIconPath(const uint8_t *rec) {
    const int iconID = *reinterpret_cast<const int *>(rec + OFF_ICON_ID);
    const uint8_t *iconRec = DBC::Record(Offsets::VAR_SPELL_ICON_RECORDS,
                                         Offsets::VAR_SPELL_ICON_COUNT,
                                         static_cast<uint32_t>(iconID));
    if (iconRec == nullptr)
        return "";
    const char *path = *reinterpret_cast<const char *const *>(iconRec + OFF_SUBRECORD_VALUE);
    return (path != nullptr) ? path : "";
}

// Pushes UnitCastingInfo's 11-tuple from a tracked cast, or nothing (nil)
// if there's no active cast.
int PushCastInfo(void *L, const TrackedSpell &c) {
    if (c.spellID == 0)
        return 0;
    const uint8_t *rec = Spell::Lookup::RecordForID(c.spellID);
    if (rec == nullptr)
        return 0;
    const char *name = SpellName(rec);
    Game::Lua::PushString(L, name);                          // 1 name
    Game::Lua::PushString(L, name);                          // 2 displayName
    Game::Lua::PushString(L, SpellIconPath(rec));            // 3 textureID (path)
    Game::Lua::PushNumber(L, static_cast<double>(c.startMs)); // 4 startTimeMs
    Game::Lua::PushNumber(L, static_cast<double>(c.endMs));   // 5 endTimeMs
    Game::Lua::PushBool(L, IsTradeskill(rec));               // 6 isTradeskill
    Game::Lua::PushNil(L);                                   // 7 castID
    Game::Lua::PushBool(L, false);                           // 8 notInterruptible
    Game::Lua::PushNumber(L, static_cast<double>(c.spellID)); // 9 castingSpellID
    Game::Lua::PushNil(L);                                   // 10 castBarID
    Game::Lua::PushNumber(L, 0);                             // 11 delayTimeMs
    return 11;
}

// Pushes UnitChannelInfo's 8-tuple. `haveTimes` is false for remote units
// (we only track the local player's channel start), pushing nil times.
int PushChannelInfo(void *L, int spellID, int startMs, int endMs, bool haveTimes) {
    if (spellID == 0)
        return 0;
    const uint8_t *rec = Spell::Lookup::RecordForID(spellID);
    if (rec == nullptr)
        return 0;
    const char *name = SpellName(rec);
    Game::Lua::PushString(L, name);                          // 1 name
    Game::Lua::PushString(L, name);                          // 2 text / displayName
    Game::Lua::PushString(L, SpellIconPath(rec));            // 3 textureID (path)
    if (haveTimes) {
        Game::Lua::PushNumber(L, static_cast<double>(startMs)); // 4 startTimeMs
        Game::Lua::PushNumber(L, static_cast<double>(endMs));   // 5 endTimeMs
    } else {
        Game::Lua::PushNil(L);
        Game::Lua::PushNil(L);
    }
    Game::Lua::PushBool(L, IsTradeskill(rec));               // 6 isTradeskill
    Game::Lua::PushBool(L, false);                           // 7 notInterruptible
    Game::Lua::PushNumber(L, static_cast<double>(spellID));   // 8 spellID
    return 8;
}

// True if the unit at stack index 1 resolves to the local player.
bool ArgIsPlayer(void *L) {
    if (!Game::Lua::IsString(L, 1))
        return false;
    const char *token = Game::Lua::ToString(L, 1);
    if (token == nullptr)
        return false;
    void *u = Resolve(token);
    return u != nullptr && u == Resolve("player");
}

// ---- Remote-unit cast tracking (SMSG_SPELL_START) ------------------------

// Vanilla stores no cast record for other units, so we capture their casts
// from SMSG_SPELL_START — the only packet carrying another unit's cast with
// a server-authoritative cast time. Keyed by caster GUID (a unit casts one
// thing at a time). Regular casts back `UnitCastingInfo`; channels add real
// times to `UnitChannelInfo` (validated against the live +0x228 field).

constexpr int OFF_ATTRIBUTES_EX = 0x1C; // Spell.dbc AttributesEx
constexpr uint32_t SPELL_ATTR_EX_CHANNELED = 0x4 | 0x40; // IS_CHANNELED | SELF

bool IsChannelSpell(const uint8_t *rec) {
    return (*reinterpret_cast<const uint32_t *>(rec + OFF_ATTRIBUTES_EX) &
            SPELL_ATTR_EX_CHANNELED) != 0;
}

// Channel duration for a non-player caster — base (skipMod=1), since we
// only have the local player's spell-mod tables.
int RemoteChannelDurationMs(const uint8_t *rec) {
    const int ms = reinterpret_cast<GetDuration_t>(
        Offsets::FUN_GET_SPELL_DURATION)(rec, 0, /*skipMod*/ 1);
    return ms > 0 ? ms : 0;
}

struct RemoteCast {
    uint64_t casterGuid;
    int spellID;
    int startMs;
    int endMs;
    bool isChannel;
    bool used;
};

constexpr int kRemoteCastSlots = 64;
RemoteCast g_remoteCasts[kRemoteCastSlots];
int g_remoteCursor = 0;

void StoreRemoteCast(uint64_t caster, int spellID, int startMs, int endMs,
                     bool isChannel) {
    const RemoteCast entry{caster, spellID, startMs, endMs, isChannel, true};
    // One active cast per unit — replace any existing entry for this caster.
    for (auto &e : g_remoteCasts) {
        if (e.used && e.casterGuid == caster) {
            e = entry;
            return;
        }
    }
    const int now = NowMs();
    for (auto &e : g_remoteCasts) {
        if (!e.used || now >= e.endMs) {
            e = entry;
            return;
        }
    }
    g_remoteCasts[g_remoteCursor] = entry;
    g_remoteCursor = (g_remoteCursor + 1) % kRemoteCastSlots;
}

const RemoteCast *FindRemoteCast(uint64_t caster) {
    if (caster == 0)
        return nullptr;
    for (const auto &e : g_remoteCasts) {
        if (e.used && e.casterGuid == caster)
            return &e;
    }
    return nullptr;
}

void RecordRemoteStart(uint64_t caster, int spellID, uint32_t castTime) {
    if (caster == 0 || spellID == 0)
        return;
    // The local player uses the self-tracked g_cast / g_channel path.
    if (caster == Unit::Identity::PlayerGuid())
        return;
    const uint8_t *rec = Spell::Lookup::RecordForID(spellID);
    if (rec == nullptr)
        return;
    const int now = NowMs();
    const bool channel = IsChannelSpell(rec);
    const int endMs = channel ? now + RemoteChannelDurationMs(rec)
                              : now + static_cast<int>(castTime);
    StoreRemoteCast(caster, spellID, now, endMs, channel);
}

// Co-hook on the SMSG_SPELL_START handler. Gated on opCode 0x131 so every
// other packet this handler processes is a cheap early-out. Packet layout
// mirrors nampower's SpellStartHandlerHook.
using SpellStartHandler_t = int(__fastcall *)(uint32_t unk, uint32_t opCode,
                                              uint32_t unk2,
                                              Net::CDataStore *packet);
SpellStartHandler_t g_origSpellStart = nullptr;

int __fastcall SpellStartHandler_h(uint32_t unk, uint32_t opCode,
                                   uint32_t unk2, Net::CDataStore *packet) {
    if (opCode == 0x131 && packet != nullptr) {
        const uint32_t saved = packet->m_read;
        Net::ReadPackedGuid(packet); // itemGuid (unused)
        const uint64_t caster = Net::ReadPackedGuid(packet);
        const int spellID = static_cast<int>(Net::Read<uint32_t>(packet));
        Net::Read<uint16_t>(packet); // castFlags (unused)
        const uint32_t castTime = Net::Read<uint32_t>(packet);
        packet->m_read = saved; // restore before the engine re-parses
        RecordRemoteStart(caster, spellID, castTime);
    }
    return g_origSpellStart(unk, opCode, unk2, packet);
}

const Game::HookAutoRegister _spellStartHook{
    Offsets::FUN_SPELL_START_HANDLER,
    reinterpret_cast<void *>(&SpellStartHandler_h),
    reinterpret_cast<void **>(&g_origSpellStart)};

} // namespace

// Per-frame: detect the player's cast/channel start, stamp times.
void OnWorldTick() {
    const int now = NowMs();

    const int cast = *reinterpret_cast<const int *>(Offsets::VAR_CURRENT_CAST_SPELL);
    if (cast != g_lastCast) {
        const int dur = (cast != 0) ? CastTimeMs(cast) : 0;
        if (cast != 0 && dur > 0) {
            g_cast = {cast, now, now + dur};
        } else {
            g_cast.spellID = 0; // 0, instant, or channel — no cast bar
        }
        g_lastCast = cast;
    }

    const int chan = PlayerChannelSpell();
    if (chan != g_lastChannel) {
        if (chan != 0) {
            g_channel = {chan, now, now + ChannelDurationMs(chan)};
        } else {
            g_channel.spellID = 0;
        }
        g_lastChannel = chan;
    }
}

static const Tick::WorldTick::AutoSubscribe _tickSub{&OnWorldTick};

// `CastingInfo()` — local player's cast, no token lookup.
static int __fastcall Script_CastingInfo(void *L) { return PushCastInfo(L, g_cast); }

// `UnitCastingInfo(unit)` — local player from self-tracking; other units
// from the SMSG_SPELL_START cache (regular casts still within their cast
// window). An interrupted remote cast lingers until its computed end time
// (no clean per-unit interrupt signal in 1.12) — acceptable best-effort.
static int __fastcall Script_UnitCastingInfo(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.UnitCastingInfo(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    void *u = (token != nullptr) ? Resolve(token) : nullptr;
    if (u == nullptr)
        return 0;
    if (u == Resolve("player"))
        return PushCastInfo(L, g_cast);

    const RemoteCast *rc = FindRemoteCast(Unit::Identity::GuidForObject(u));
    if (rc != nullptr && !rc->isChannel && NowMs() < rc->endMs)
        return PushCastInfo(L, TrackedSpell{rc->spellID, rc->startMs, rc->endMs});
    return 0;
}

// `ChannelInfo()` — local player's channel, no token lookup.
static int __fastcall Script_ChannelInfo(void *L) {
    return PushChannelInfo(L, g_channel.spellID, g_channel.startMs, g_channel.endMs, true);
}

// `UnitChannelInfo(unit)` — full timing for the player; spellID/name/
// texture (no times) for other units via the broadcast +0x228 field.
static int __fastcall Script_UnitChannelInfo(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: C_Spell.UnitChannelInfo(\"unit\")");
        return 0;
    }
    const char *token = Game::Lua::ToString(L, 1);
    void *u = (token != nullptr) ? Resolve(token) : nullptr;
    if (u == nullptr)
        return 0;
    if (u == Resolve("player"))
        return PushChannelInfo(L, g_channel.spellID, g_channel.startMs, g_channel.endMs, true);

    auto *desc = *reinterpret_cast<const uint8_t *const *>(
        static_cast<const uint8_t *>(u) + Offsets::OFF_UNIT_DESCRIPTOR);
    if (desc == nullptr)
        return 0;
    // The live +0x228 field is authoritative for "is this unit channeling
    // right now"; the SMSG_SPELL_START cache adds real start/end times when
    // we observed the channel begin (and still matches the current spell).
    const int spellID = *reinterpret_cast<const int *>(desc + Offsets::OFF_UNIT_FIELD_CHANNEL_SPELL);
    if (spellID == 0)
        return 0;
    const RemoteCast *rc = FindRemoteCast(Unit::Identity::GuidForObject(u));
    if (rc != nullptr && rc->isChannel && rc->spellID == spellID && NowMs() < rc->endMs)
        return PushChannelInfo(L, spellID, rc->startMs, rc->endMs, /*haveTimes*/ true);
    return PushChannelInfo(L, spellID, 0, 0, /*haveTimes*/ false);
}

static void RegisterLuaFunctions() {
    // Registered under C_Spell rather than as globals to avoid clobbering
    // the global `UnitCastingInfo` / `UnitChannelInfo` names. Addons that
    // ship their own vanilla cast-tracking libraries use the
    // `_G.UnitCastingInfo or <fallback>` idiom (e.g. ShaguTweaks'
    // libcast.lua scrapes the combat log for remote/enemy casts the 1.12
    // engine never exposes). Occupying the global makes them adopt our
    // player-only version and drop their superior fallback — so we cede
    // the global names and expose the functions here instead.
    Game::Lua::RegisterTableFunction("C_Spell", "UnitCastingInfo", &Script_UnitCastingInfo);
    Game::Lua::RegisterTableFunction("C_Spell", "CastingInfo", &Script_CastingInfo);
    Game::Lua::RegisterTableFunction("C_Spell", "UnitChannelInfo", &Script_UnitChannelInfo);
    Game::Lua::RegisterTableFunction("C_Spell", "ChannelInfo", &Script_ChannelInfo);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Spell::Cast
