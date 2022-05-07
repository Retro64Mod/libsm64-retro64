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
#include "model_handler.h"
#include "decomp/game/object_helpers.h"
#include "decomp/game/behavior_actions.h"
#include "decomp/include/behavior_data.h"
#include "decomp/include/object_fields.h"
#include "decomp/game/object_list_processor.h"
#include "decomp/game/object_collision.h"
#include "actorMgr.h"
struct ObjPool s_actor_instance_pool = { 0, 0 };

SM64_LIB_FN int initActor(int actorType,int x,int y,int z,int scale){
    int id = obj_pool_alloc_index( &s_actor_instance_pool, sizeof( struct GlobalState ));

    s_actor_instance_pool.objects[id] = global_state_create();
    global_state_bind( s_actor_instance_pool.objects[id] );
    g_state->mgCurrentObject=NULL;
    struct Object* obj = spawn_object_at_origin(gMarioState->marioObj,0,1,bhvGoomba);
    obj->oPosX=-1442;
    obj->oPosY=0;
    obj->oPosZ=-444;
    obj->parentObj=obj;
    gCurrentObject=obj;
    s_actor_instance_pool.objects[id] = obj;
    return id;
}

void tickAllActors(){
    for (int i = 0; i < s_actor_instance_pool.size; i++) {
        if (s_actor_instance_pool.objects[i] != NULL) {
            tickActor(i,NULL);
        }
    }
}

SM64_LIB_FN void tickActor(int actorID,struct SM64MarioGeometryBuffers *outBuffers){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    //global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    gCurrentObject = s_actor_instance_pool.objects[ actorID ];
    //gfx_adapter_bind_output_buffers( outBuffers );
    cur_obj_update();
    geo_process_root_hack_single_node_obj( getModel(-1) );

    gAreaUpdateCounter++;
    detect_object_collisions(); // temp
}

SM64_LIB_FN void tickActorAnim(int actorID,struct SM64MarioGeometryBuffers *outBuffers){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    //gfx_adapter_bind_output_buffers( outBuffers );

    geo_process_root_hack_single_node_obj( &g_state->mgCurrentObject->header.gfx.node );

    gAreaUpdateCounter++;
}

enum ObjectList getActorObjList(int actorID){
    struct Object* obj = s_actor_instance_pool.objects[ actorID ];
    if (obj==NULL) {
        return -1;
    }
    return (obj->behavior[0] >> 16) & 0xFFFF;
}

struct ObjPool* getActorPool(){
    return &s_actor_instance_pool;
}