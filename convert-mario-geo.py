#!/usr/bin/env python3
import os
import shutil

geo_inc_c_header = """
#include "../include/sm64.h"
#include "../include/types.h"
#include "../include/geo_commands.h"
#include "../game/rendering_graph_node.h"
#include "../shim.h"
#include "../game/object_stuff.h"
#include "../game/behavior_actions.h"
#include "model.inc.h"

#define SHADOW_CIRCLE_PLAYER 99
"""

geo_inc_c_footer = """
const GeoLayout mario_geo_libsm64[] = {
   GEO_SHADOW(SHADOW_CIRCLE_PLAYER, 0xB4, 100),
   GEO_OPEN_NODE(),
      GEO_ZBUFFER(1),
      GEO_OPEN_NODE(),
         GEO_SCALE(0x00, 16384),
         GEO_OPEN_NODE(),
            GEO_ASM(0, geo_mirror_mario_backface_culling),
            GEO_ASM(0, geo_mirror_mario_set_alpha),
            GEO_BRANCH(1, mario_geo_load_body),
            GEO_ASM(1, geo_mirror_mario_backface_culling),
         GEO_CLOSE_NODE(),
      GEO_CLOSE_NODE(),
   GEO_CLOSE_NODE(),
   GEO_END(),
};

void *mario_geo_ptr = (void*)mario_geo_libsm64;

"""

geo_inc_h = """
#pragma once

extern void *mario_geo_ptr;
"""

model_inc_h = """
#pragma once

#include "../include/types.h"
#include "../include/PR/gbi.h"
"""

def main():
    global model_inc_h

    print("Loading geo.inc.c")
    geo_inc_c = ""
    with open("src/decomp/mario/geo.inc.c") as file:
        geo_inc_c=file.read();
    print("Loading model.inc.c")
    model_inc_c = ""
    with open("src/decomp/mario/model.inc.c") as file:
        model_inc_c=file.read();

    lines = model_inc_c.splitlines()

    skip = 0
    for i in range(len(lines)):
        if skip > 0:
            skip = skip - 1
            lines[i] = "//" + lines[i]
        elif lines[i].startswith("ALIGNED8 static const u8 mario_"):# or lines[i].startswith("u8 mario_"):
            skip = 2
            lines[i] = "//" + lines[i]
        elif lines[i].startswith("const "):# or (lines[i].startswith("Gfx ")):
            model_inc_h += "\nextern " + lines[i].replace(" = {", ";")

    lines.insert(0, "#include \"../../gfx_macros.h\"")
    lines.insert(0, "#include \"../../load_tex_data.h\"")
    model_inc_c = "\n".join(lines)


    with open("src/decomp/mario/geo.inc.c", "w") as file:
        file.write(geo_inc_c_header + geo_inc_c + geo_inc_c_footer)

    with open("src/decomp/mario/model.inc.c", "w") as file:
        file.write(model_inc_c)

    with open("src/decomp/mario/model.inc.h", "w") as file:
        file.write(model_inc_h)

    with open("src/decomp/mario/geo.inc.h", "w") as file:
        file.write(geo_inc_h)

if __name__ == "__main__":
    main()
