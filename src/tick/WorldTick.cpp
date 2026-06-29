// This file is part of ClassicAPI.
//
// ClassicAPI is free software: you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software Foundation, either
// version 3 of the License, or (at your option) any later version.
//
// ClassicAPI is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.

#include "WorldTick.h"

#include "Game.h"
#include "Offsets.h"

namespace Tick::WorldTick {

namespace {

AutoSubscribe *g_head = nullptr;

using WorldTick_t = void(__fastcall *)(int, int, int);
WorldTick_t WorldTick_o = nullptr;

void __fastcall WorldTick_h(int a, int b, int c) {
    WorldTick_o(a, b, c);
    for (auto *node = g_head; node != nullptr; node = node->next)
        node->cb();
}

} // namespace

AutoSubscribe::AutoSubscribe(Callback cb) : cb(cb), next(g_head) {
    g_head = this;
}

static const Game::HookAutoRegister _hook{
    Offsets::FUN_WORLD_TICK,
    reinterpret_cast<void *>(&WorldTick_h),
    reinterpret_cast<void **>(&WorldTick_o)};

} // namespace Tick::WorldTick
