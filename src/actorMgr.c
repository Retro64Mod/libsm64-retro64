#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "libsm64.h"
#include "decomp/model_goomba/anims/table.inc.c"
#include "decomp/include/PR/os_cont.h"
#include "decomp/include/sm64.h"
#include "decomp/shim.h"
#include "decomp/memory.h"
#include "decomp/global_state.h"
#include "decomp/game/mario.h"
#include "decomp/game/object_stuff.h"
#include "decomp/engine/surface_collision.h"
#include "decomp/engine/graph_node.h"
#include "decomp/engine/geo_layout.h"
#include "decomp/game/rendering_graph_node.h"
#include "decomp/mario/geo.inc.h"
#include "decomp/game/platform_displacement.h"
#include "debug_print.h"
#include "load_surfaces.h"
#include "gfx_adapter.h"
#include "load_anim_data.h"
#include "load_tex_data.h"
#include "obj_pool.h"
#include "model_handler.h"


struct ObjPool s_actor_instance_pool = { 0, 0 };

SM64_LIB_FN int initActor(int actorType,int x,int y,int z,int scale){
    int id = obj_pool_alloc_index( &s_actor_instance_pool, sizeof( struct GlobalState ));

    s_actor_instance_pool.objects[id] = global_state_create();
    global_state_bind( s_actor_instance_pool.objects[id] );
    g_state->mgCurrentObject=NULL;
    
    g_state->mgCurrentObject=malloc(sizeof(struct Object));
    gCurrentObject->header.gfx.animInfo.animAccel=0x10000;
    gCurrentObject->header.gfx.animInfo.animFrameAccelAssist=1;
    gCurrentObject->header.gfx.animInfo.animID=0;
    gCurrentObject->header.gfx.animInfo.animFrame=0;
    gCurrentObject->header.gfx.animInfo.curAnim=goomba_seg8_anims_0801DA4C[0];
    gCurrentObject->header.gfx.throwMatrix=NULL;

    gCurrentObject->header.gfx.scale[0]=scale;
    gCurrentObject->header.gfx.scale[1]=scale;
    gCurrentObject->header.gfx.scale[2]=scale;

    gCurrentObject->header.gfx.pos[0]=x;
    gCurrentObject->header.gfx.pos[1]=y;
    gCurrentObject->header.gfx.pos[2]=z;

    gCurrentObject->header.gfx.angle[0]=0;
    gCurrentObject->header.gfx.angle[1]=0;
    gCurrentObject->header.gfx.angle[2]=0;
    
    return id;
}

SM64_LIB_FN void tickActor(int actorID,struct SM64MarioGeometryBuffers *outBuffers){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    gfx_adapter_bind_output_buffers( outBuffers );
    
    geo_process_root_hack_single_node_obj( getModel(-1) );

    gAreaUpdateCounter++;
}