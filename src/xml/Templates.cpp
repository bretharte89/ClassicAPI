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

// `C_XMLUtil.GetTemplates()` — enumerate every registered virtual XML frame
// template as `{ name = <string>, type = <frameType> }`.
//
// The engine keeps virtual templates (the `<Frame virtual="true">` etc. that
// `inherits=` targets) in a Storm hash table. The XML parser (FUN_006ede10)
// registers each one via FUN_006ee500, and the inherits resolver reads the
// same table via FUN_006ee6f0 — so it's the authoritative template store, and
// walking it yields exactly what modern `C_XMLUtil.GetTemplates` returns.
// Fonts are a separate registry and are (correctly) not included; non-virtual
// named frames take a different instantiate path and never enter this table.
//
// Table shape (see Offsets.h): a POINTER to a bucket array at
// VAR_XML_TEMPLATE_TABLE, a bucket-count mask at VAR_XML_TEMPLATE_MASK
// (0xFFFFFFFF until the first template registers). Each bucket is a Storm
// intrusive list; each node carries the template name (+0x14) and the parsed
// definition node (+0x18), whose element tag string (+0x8) is the frame type.
// Contents reflect the currently-loaded XML and are rebuilt on `/reload`.

#include "Game.h"
#include "Offsets.h"

#include <cstdint>

namespace Xml::Templates {

namespace {

// `C_XMLUtil.GetTemplates()` -> XMLTemplateListInfo[] of { name, type }.
int __fastcall Script_GetTemplates(void *L) {
    Game::Lua::NewTable(L); // the result array

    const uint32_t mask =
        *reinterpret_cast<const uint32_t *>(Offsets::VAR_XML_TEMPLATE_MASK);
    if (mask == 0xFFFFFFFFu)
        return 1; // no template has ever registered — empty array

    const uint8_t *base = *reinterpret_cast<const uint8_t *const *>(
        Offsets::VAR_XML_TEMPLATE_TABLE);
    if (base == nullptr)
        return 1;

    int outIdx = 0;
    for (uint32_t b = 0; b <= mask; ++b) {
        const uint8_t *bucket = base + b * Offsets::XML_TEMPLATE_BUCKET_STRIDE;
        const int linkOff = *reinterpret_cast<const int *>(
            bucket + Offsets::OFF_XML_TEMPLATE_BUCKET_LINKOFF);
        const uint8_t *node = *reinterpret_cast<const uint8_t *const *>(
            bucket + Offsets::OFF_XML_TEMPLATE_BUCKET_HEAD);

        // Walk the intrusive chain; the tail sentinel has its low bit set
        // (mirrors the engine's own traversal in FUN_006ee6f0).
        while (node != nullptr && (reinterpret_cast<uintptr_t>(node) & 1) == 0) {
            const char *name = *reinterpret_cast<const char *const *>(
                node + Offsets::OFF_XML_TEMPLATE_NODE_NAME);
            const uint8_t *def = *reinterpret_cast<const uint8_t *const *>(
                node + Offsets::OFF_XML_TEMPLATE_NODE_DEF);
            const char *type =
                (def != nullptr)
                    ? *reinterpret_cast<const char *const *>(
                          def + Offsets::OFF_XML_NODE_TAG)
                    : nullptr;

            ++outIdx;
            Game::Lua::PushNumber(L, static_cast<double>(outIdx));
            Game::Lua::NewTable(L);
            Game::Lua::SetFieldString(L, "name", name != nullptr ? name : "");
            Game::Lua::SetFieldString(L, "type", type != nullptr ? type : "");
            Game::Lua::SetTable(L, -3);

            node = *reinterpret_cast<const uint8_t *const *>(node + linkOff + 4);
        }
    }
    return 1;
}

} // namespace

static void RegisterLuaFunctions() {
    Game::Lua::RegisterTableFunction("C_XMLUtil", "GetTemplates",
                                     &Script_GetTemplates);
}

static const Game::ModuleAutoRegister _autoreg{&RegisterLuaFunctions};

} // namespace Xml::Templates
