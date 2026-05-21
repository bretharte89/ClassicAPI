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

// `GetMirrorTimerInfo(index)` / `GetMirrorTimerProgress(label)` — modern
// (3.0+) readers for the BREATH / FATIGUE / EXHAUSTION / FEIGNDEATH
// timer state. Vanilla 1.12's engine fires `MIRROR_TIMER_START` /
// `MIRROR_TIMER_PAUSE` / `MIRROR_TIMER_STOP` events with the full
// packet payload, but doesn't cache anything internally — so we hook
// the SMSG handler (`FUN_005E7990`) and build a 3-slot side cache.
//
// The hook peeks the packet via cursor save/restore on the CDataStore
// (offset `+0x14`) before the original consumes the bytes. After
// peeking, we restore the cursor and let the original handler run
// normally, which fires the existing Lua events untouched.
//
// Timer types in vanilla 1.12 (per `FUN_005E7AE0`):
//   0 → "EXHAUSTION"  (sliding outside playable bounds; map edges)
//   1 → "BREATH"      (underwater drowning)
//   2 → "FEIGNDEATH"  (Hunter Feign Death duration)
//
// Modern uses "FATIGUE" instead of "EXHAUSTION" for the same concept;
// we preserve the engine's wording. Addons that hardcode "FATIGUE"
// from backported code need to handle either.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>
#include <cstring>
#include <windows.h>

namespace Unit::MirrorTimer {

namespace {

using DataStoreReadU32_t = void(__thiscall *)(void *self, uint32_t *out);
using DataStoreReadU8_t = void(__thiscall *)(void *self, uint8_t *out);
using TimerTypeName_t = const char *(__fastcall *)(int type);
using TimerLabel_t = const char *(__fastcall *)(int type, int spellID);

// Windows `GetTickCount` rather than the engine's `FUN_ENGINE_TICK_MS`
// — the engine helper has two internal branches (`GetTickCount` vs a
// high-res counter with a different epoch) selected by a runtime flag,
// and the caller chain runs the result through `__ftol`, suggesting at
// least one path returns via the FPU rather than EAX. Reading those
// bytes as a plain `uint64_t` return picks up garbage. `GetTickCount`
// is single-epoch (ms since boot), monotonic, and unambiguous.
uint32_t NowMs() {
    return ::GetTickCount();
}

const char *TypeName(int type) {
    auto fn = reinterpret_cast<TimerTypeName_t>(Offsets::FUN_MIRROR_TIMER_TYPE_NAME);
    return fn(type);
}

const char *LabelFor(int type, int spellID) {
    auto fn = reinterpret_cast<TimerLabel_t>(Offsets::FUN_MIRROR_TIMER_LABEL);
    return fn(type, spellID);
}

struct Slot {
    bool active = false;
    int type = 0;
    int initialValue = 0;   // value as received in the START packet (ms)
    int maxValue = 0;
    int scale = 0;          // packet rate (positive = decreasing)
    bool paused = false;
    int spellID = 0;        // populated for FEIGNDEATH; 0 otherwise
    uint32_t baseMs = 0;    // engine ms at which `initialValue` was true
};

Slot g_slots[Offsets::MIRROR_TIMER_SLOT_COUNT];

// Compute the live current value given a slot's snapshot + a current
// engine tick. Matches what the vanilla FrameXML breath/fatigue bars
// do internally: linear interpolation `value = initial + elapsed *
// scale`. Vanilla sends `scale = -1` for depleting timers (breath
// drains 60000 → 0 over 60s = `+ 25000 * -1 = -25000` per 25s
// real-time), so the sign in the formula is `+`, not `-`. Modern's
// docs describe positive scale as depleting; that's a post-vanilla
// convention flip — we follow the wire format the 1.12 server
// actually sends. Returns the snapshot verbatim while paused.
int LiveValue(const Slot &s, uint32_t now) {
    if (!s.active)
        return 0;
    if (s.paused)
        return s.initialValue;
    const int elapsed = static_cast<int>(now - s.baseMs);
    const int value = s.initialValue + elapsed * s.scale;
    if (value < 0)
        return 0;
    if (s.maxValue > 0 && value > s.maxValue)
        return s.maxValue;
    return value;
}

void OnStart(int type, int value, int maxValue, int scale,
             bool paused, int spellID) {
    if (type < 0 || type >= Offsets::MIRROR_TIMER_SLOT_COUNT)
        return;
    Slot &s = g_slots[type];
    s.active = true;
    s.type = type;
    s.initialValue = value;
    s.maxValue = maxValue;
    s.scale = scale;
    s.paused = paused;
    s.spellID = spellID;
    s.baseMs = NowMs();
}

void OnPause(int type, bool paused) {
    if (type < 0 || type >= Offsets::MIRROR_TIMER_SLOT_COUNT)
        return;
    Slot &s = g_slots[type];
    if (!s.active)
        return;
    if (paused) {
        // Freeze: snapshot the live value as the new initial, and
        // mark paused so `LiveValue` returns it verbatim.
        s.initialValue = LiveValue(s, NowMs());
        s.paused = true;
    } else if (s.paused) {
        // Unpause: reset the base tick so live interpolation
        // resumes from the frozen value.
        s.paused = false;
        s.baseMs = NowMs();
    }
}

void OnStop(int type) {
    if (type < 0 || type >= Offsets::MIRROR_TIMER_SLOT_COUNT)
        return;
    g_slots[type] = Slot{};
}

// Peek the packet by reading the fields out of the data store, then
// rewind the cursor so the original handler reads the same bytes.
// `out`-only — caches into our slot table; returns whether the read
// successfully observed the expected fields.
void PeekStartPacket(void *dataStore) {
    auto readU32 = reinterpret_cast<DataStoreReadU32_t>(
        Offsets::FUN_DATASTORE_READ_U32);
    auto readU8 = reinterpret_cast<DataStoreReadU8_t>(
        Offsets::FUN_DATASTORE_READ_U8);
    auto cursorPtr = reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(dataStore) + Offsets::OFF_DATASTORE_CURSOR);
    const uint32_t saved = *cursorPtr;

    uint32_t type = 0, value = 0, maxValue = 0, scale = 0, spellID = 0;
    uint8_t paused = 0;
    readU32(dataStore, &type);
    readU32(dataStore, &value);
    readU32(dataStore, &maxValue);
    readU32(dataStore, &scale);
    readU8(dataStore, &paused);
    readU32(dataStore, &spellID);

    *cursorPtr = saved;

    OnStart(static_cast<int>(type), static_cast<int>(value),
            static_cast<int>(maxValue), static_cast<int>(scale),
            paused != 0, static_cast<int>(spellID));
}

void PeekPausePacket(void *dataStore) {
    auto readU32 = reinterpret_cast<DataStoreReadU32_t>(
        Offsets::FUN_DATASTORE_READ_U32);
    auto readU8 = reinterpret_cast<DataStoreReadU8_t>(
        Offsets::FUN_DATASTORE_READ_U8);
    auto cursorPtr = reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(dataStore) + Offsets::OFF_DATASTORE_CURSOR);
    const uint32_t saved = *cursorPtr;

    uint32_t type = 0;
    uint8_t paused = 0;
    readU32(dataStore, &type);
    readU8(dataStore, &paused);

    *cursorPtr = saved;

    OnPause(static_cast<int>(type), paused != 0);
}

void PeekStopPacket(void *dataStore) {
    auto readU32 = reinterpret_cast<DataStoreReadU32_t>(
        Offsets::FUN_DATASTORE_READ_U32);
    auto cursorPtr = reinterpret_cast<uint32_t *>(
        static_cast<uint8_t *>(dataStore) + Offsets::OFF_DATASTORE_CURSOR);
    const uint32_t saved = *cursorPtr;

    uint32_t type = 0;
    readU32(dataStore, &type);

    *cursorPtr = saved;

    OnStop(static_cast<int>(type));
}

// Slot lookup by type-name string. Used by `GetMirrorTimerProgress`,
// whose caller passes the engine's literal name (e.g. "BREATH").
const Slot *FindSlotByName(const char *name) {
    if (name == nullptr)
        return nullptr;
    for (int i = 0; i < Offsets::MIRROR_TIMER_SLOT_COUNT; ++i) {
        const Slot &s = g_slots[i];
        if (!s.active)
            continue;
        const char *engineName = TypeName(s.type);
        if (engineName != nullptr && std::strcmp(engineName, name) == 0)
            return &s;
    }
    return nullptr;
}

// `GetMirrorTimerInfo(index)` — returns `(timer, value, maxValue,
// scale, paused, label)` for the timer in slot `index` (1..3), or
// nothing if that slot is empty. Matches modern's 6-tuple shape.
// `value` is the snapshot from the last server packet (not
// live-interpolated) — modern documents the same "possibly outdated"
// caveat. Use `GetMirrorTimerProgress(timer)` for the live value.
int __fastcall Script_GetMirrorTimerInfo(void *L) {
    if (!Game::Lua::IsNumber(L, 1)) {
        Game::Lua::Error(L, "Usage: GetMirrorTimerInfo(index)");
        return 0;
    }
    const int index1 = static_cast<int>(Game::Lua::ToNumber(L, 1));
    const int slotIdx = index1 - 1;
    if (slotIdx < 0 || slotIdx >= Offsets::MIRROR_TIMER_SLOT_COUNT)
        return 0;
    const Slot &s = g_slots[slotIdx];
    if (!s.active)
        return 0;

    const char *name = TypeName(s.type);
    const char *label = LabelFor(s.type, s.spellID);

    Game::Lua::PushString(L, name != nullptr ? name : "");
    Game::Lua::PushNumber(L, static_cast<double>(s.initialValue));
    Game::Lua::PushNumber(L, static_cast<double>(s.maxValue));
    Game::Lua::PushNumber(L, static_cast<double>(s.scale));
    Game::Lua::PushBool(L, s.paused);
    Game::Lua::PushString(L, label != nullptr ? label : "");
    return 6;
}

// `GetMirrorTimerProgress(label)` — returns the live current value of
// the timer matching `label` (one of the engine's type-name strings:
// "EXHAUSTION", "BREATH", "FEIGNDEATH"), in milliseconds. Returns 0
// when no matching timer is currently active. Computed each call by
// linearly interpolating from the last packet snapshot using the
// engine's millisecond clock — same logic the stock FrameXML bars
// run in their `OnUpdate` handlers.
int __fastcall Script_GetMirrorTimerProgress(void *L) {
    if (!Game::Lua::IsString(L, 1)) {
        Game::Lua::Error(L, "Usage: GetMirrorTimerProgress(\"timer\")");
        return 0;
    }
    const char *name = Game::Lua::ToString(L, 1);
    const Slot *s = FindSlotByName(name);
    if (s == nullptr) {
        Game::Lua::PushNumber(L, 0);
        return 1;
    }
    Game::Lua::PushNumber(L, static_cast<double>(LiveValue(*s, NowMs())));
    return 1;
}

void RegisterLuaFunctions() {
    Game::Lua::RegisterGlobalFunction("GetMirrorTimerInfo",
                                       &Script_GetMirrorTimerInfo);
    Game::Lua::RegisterGlobalFunction("GetMirrorTimerProgress",
                                       &Script_GetMirrorTimerProgress);
}

const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace

// `FUN_005E7990` is a true `__fastcall` with 4 real args — `param_1` in
// ECX, the opcode (`param_2`) in EDX, then `param_3` and `param_4` on the
// stack. NOT a thiscall — there's no dummy EDX slot. Declaring it with
// the thiscall shape (extra `void *edx` parameter) shifts every
// subsequent arg by one slot, leaving `dataStore` reading garbage off the
// end of the stack frame and crashing inside `FUN_00418E30`'s
// vtable-fallback path when the cursor bounds check fails.
using SMSGHandler_t = int(__fastcall *)(void *param_1, int opcode,
                                         void *param_3, void *dataStore);
SMSGHandler_t SMSGHandler_o = nullptr;

int __fastcall SMSGHandler_h(void *param_1, int opcode,
                              void *param_3, void *dataStore) {
    if (dataStore != nullptr) {
        switch (opcode) {
            case Offsets::SMSG_OPCODE_MIRROR_TIMER_START:
                PeekStartPacket(dataStore);
                break;
            case Offsets::SMSG_OPCODE_MIRROR_TIMER_PAUSE:
                PeekPausePacket(dataStore);
                break;
            case Offsets::SMSG_OPCODE_MIRROR_TIMER_STOP:
                PeekStopPacket(dataStore);
                break;
        }
    }
    return SMSGHandler_o(param_1, opcode, param_3, dataStore);
}

static const Game::HookAutoRegister _hookreg{
    Offsets::FUN_SMSG_MIRROR_TIMER_HANDLER,
    reinterpret_cast<void *>(&SMSGHandler_h),
    reinterpret_cast<void **>(&SMSGHandler_o)};

} // namespace Unit::MirrorTimer
