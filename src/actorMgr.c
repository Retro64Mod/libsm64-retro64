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
#include "decomp/include/model_ids.h"
#include "actorMgr.h"
struct ObjPool s_actor_instance_pool = { 0, 0 };

int putObjectInActorPool(struct Object* obj){
    int id = obj_pool_alloc_index( &s_actor_instance_pool, sizeof( struct GlobalState ));
    s_actor_instance_pool.objects[id] = global_state_create();
    ((struct GlobalState*)s_actor_instance_pool.objects[ id ])->mgCurrentObject=obj;
    return id;

}

SM64_LIB_FN int initActor(int actorType,float x,float y,float z){
    g_state->mgCurrentObject=NULL;
    struct Object* obj = spawn_object_at_origin(gMarioState->marioObj,0,MODEL_GOOMBA,bhvGoomba);
    obj->oPosX=x;
    obj->oPosY=y;
    obj->oPosZ=z;
    obj->parentObj=obj;
    return obj->unused2;
}

void tickAllActors(){
    for (int i = 0; i < s_actor_instance_pool.size; i++) {
        if (s_actor_instance_pool.objects[i] != NULL) {
            tickActor(i,NULL,NULL);
        }
    }
}

SM64_LIB_FN void tickActor(int actorID,struct SM64ActorState* state,struct SM64MarioGeometryBuffers *outBuffers){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    // since we're in the object's state now, we need to set it's mario object to the closest mario inst
    
    gMarioObject=(*((struct GlobalState **)s_mario_instance_pool.objects[ 0 ]))->mgMarioObject;//->oPosX
    if (outBuffers!=NULL)
        gfx_adapter_bind_output_buffers( outBuffers );
    cur_obj_update();
    if (gCurrentObject->header.gfx.sharedChild!=0x0)
    geo_process_root_hack_single_node_obj( g_state->mgCurrentObject->header.gfx.sharedChild );

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

    geo_process_root_hack_single_node_obj( g_state->mgCurrentObject->header.gfx.sharedChild );

    gAreaUpdateCounter++;
}

enum ObjectList getActorObjList(int actorID){
    struct GlobalState* obj = s_actor_instance_pool.objects[ actorID ];
    if (obj==NULL) {
        return -1;
    }
    return (obj->mgCurrentObject->behavior[0] >> 16) & 0xFFFF;
}

struct ObjPool* getActorPool(){
    return &s_actor_instance_pool;
}

struct Object* getActor(int id){
    struct GlobalState* gs = ((struct GlobalState*)s_actor_instance_pool.objects[ id ]);
    if (gs==NULL)
        return NULL;
    return gs->mgCurrentObject;
}