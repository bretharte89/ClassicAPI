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

#pragma once

namespace Item::Swap {

// Sends a server-side swap to equip `cgItem` into the given paperdoll
// slot, by way of the engine's own packet builder/sender at
// FUN_INVENTORY_SWAP (0x005E0C40). Same code path drag-and-drop equip
// uses — the cursor is never read or written.
//
// Variants differ only in how they encode the *source* location:
//
// - `FromBag(item, bagID, slotInBag, dst)` — item currently in
//   container `bagID` at 1-based `slotInBag`. `bagID = 0` is the
//   backpack (linear-slot 22 + slotInBag, container = player);
//   `bagID = 1..4` is an equipped bag (linear-slot `slotInBag - 1`,
//   container = that bag's GUID).
//
// - `FromPaperdoll(item, srcSlot, dst)` — item currently in
//   paperdoll slot `srcSlot` (1-based). Used for paperdoll-to-
//   paperdoll swaps like rings 11/12, trinkets 13/14.
//
// All slot args are 1-based Lua-facing; helpers do the
// linear-slot conversion. Returns `false` on bad args (slot out of
// range, no player object, missing instance block on the item or
// player), `true` after dispatching the packet (does NOT wait for
// server confirmation).
//
// Caller must already hold the source CGItem pointer — typically
// obtained from the location walk that discovered the swap target.
bool FromBag(const void *cgItem, int bagID, int slotInBag, int dstPaperdollSlot);

bool FromPaperdoll(const void *cgItem, int srcPaperdollSlot, int dstPaperdollSlot);

// General bag-to-bag swap — neither side is constrained to the
// paperdoll. Same engine helper (`FUN_INVENTORY_SWAP`), which routes
// to opcode 0x10D (same-container slot swap, e.g. backpack ↔ backpack)
// or 0x10C (cross-container swap, e.g. backpack → equipped bag) based
// on whether the two container GUIDs match.
//
// bagID convention matches `C_Container.GetContainerItemInfo`:
//   0    = backpack
//   1..4 = equipped bags (in equipment slots 20..23)
// slot is 1-based. Returns false if either bag is unequipped, either
// slot is out of bounds, or the source slot is empty. Destination is
// allowed to be empty — the engine treats that as a move; or occupied
// — engine atomically swaps.
//
// Requires a valid Lua state pointer because the source-side CGItem
// lookup goes through `Item::Location::ResolveBag`, which uses the
// engine's `PackBagSlot` helper. Stomps the Lua stack — callers must
// read every arg they need before calling.
bool Containers(void *L, int srcBag, int srcSlot, int dstBag, int dstSlot);

// Atomic server-side "split N items from src and place at dst" — same
// wire-level primitive vanilla emits when you `SplitContainerItem` →
// drop-on-target, but bundled into one packet (`CMSG_SPLIT_ITEM`,
// opcode 0x10E). No cursor involvement.
//
// Semantics (server-enforced, all-or-nothing):
//   - `dst` empty                        → places `count` there
//   - `dst` same item & fits maxStack    → merges `count` into dst
//   - `dst` different item / overflow    → server rejects (no partial)
//
// Source must be occupied with at least `count` items; server rejects
// if not. Returns false locally only on bad args or unresolvable bags.
// All other validation is server-side — same as `Containers` above.
//
// Requires Lua state for the same reason as `Containers`: the source
// CGItem lookup goes through `Item::Location::ResolveBag`. Stomps the
// Lua stack.
bool MoveCount(void *L, int srcBag, int srcSlot, int dstBag, int dstSlot, int count);

} // namespace Item::Swap
