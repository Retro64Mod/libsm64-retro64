#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "libsm64.h"
#include "decomp/include/PR/os_cont.h"
#include "decomp/include/sm64.h"
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
#include "model_handler.h"
struct ObjPool s_actor_instance_pool = { 0, 0 };


int putObjectInActorPool(struct Object* obj){
    int id = obj_pool_alloc_index( &s_actor_instance_pool, sizeof( struct GlobalState ));
    s_actor_instance_pool.objects[id] = global_state_create();
    ((struct GlobalState*)s_actor_instance_pool.objects[ id ])->mgCurrentObject=obj;
    return id;

}

SM64_LIB_FN int initActor(int actorType,float x,float y,float z){
    struct Object* obj;
    //g_state->mgCurrentObject=NULL;
    if (actorType==ACTOR_TYPE_GOOMBA){
        obj = spawn_object_at_origin(NULL,0,MODEL_GOOMBA,bhvGoomba);
    }else if (actorType==ACTOR_TYPE_STAR){
        obj = spawn_object_at_origin(NULL,0,MODEL_STAR,bhvStar);
    }else{
        return -2;
    }
    obj->oPosX=x;
    obj->oPosY=y;
    obj->oPosZ=z;
    obj->parentObj=obj;
    return obj->unused2;
}

SM64_LIB_FN void destroyActor(int actorID){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to destroy non-existant Actor with ID: %u", actorID);
        return;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    free(gCurrentObject);
    // todo: do we have to free vertices/surfaces as well?
    obj_pool_free_index( &s_actor_instance_pool, actorID );
}

SM64_LIB_FN struct AnimInfo* tickActor(int actorID,struct SM64ActorState* state,struct SM64MarioGeometryBuffers *outBuffers){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );
    // TODO: since we're in the object's state now, we need to set it's mario object to the closest mario inst
    if (s_mario_instance_pool.objects == NULL || s_mario_instance_pool.objects[ 0 ]==NULL)
        gMarioObject=NULL; // may cause mobs to be deleted in java side
    else
        gMarioObject=(*((struct GlobalState **)s_mario_instance_pool.objects[ 0 ]))->mgMarioObject;//->oPosX
    if (outBuffers!=NULL)
        gfx_adapter_bind_output_buffers( outBuffers );
    cur_obj_update();
    if (gCurrentObject->header.gfx.sharedChild!=0x0)
        geo_process_root_hack_single_node_obj( g_state->mgCurrentObject->header.gfx.sharedChild );

    if (state!=NULL){
        state->position[0]=gCurrentObject->oPosX;
        state->position[1]=gCurrentObject->oPosY;
        state->position[2]=gCurrentObject->oPosZ;
        state->velocity[0]=gCurrentObject->oVelX;
        state->velocity[1]=gCurrentObject->oVelY;
        state->velocity[2]=gCurrentObject->oVelZ;
        state->scale[0]=gCurrentObject->header.gfx.scale[0];
        state->scale[1]=gCurrentObject->header.gfx.scale[1];
        state->scale[2]=gCurrentObject->header.gfx.scale[2];
        state->rotation[0]=gCurrentObject->header.gfx.angle[0];
        state->rotation[1]=gCurrentObject->header.gfx.angle[1];
        state->rotation[2]=gCurrentObject->header.gfx.angle[2];
    }
    gAreaUpdateCounter++;   
    detect_object_collisions(); // temp
    return &gCurrentObject->header.gfx.animInfo;
}

SM64_LIB_FN void tickActorAnim(int actorID,uint32_t stateFlags,struct AnimInfo* info,struct SM64MarioGeometryBuffers *outBuffers,int16_t rot[3],float scale[3]){
    if( actorID >= s_actor_instance_pool.size || s_actor_instance_pool.objects[actorID] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Actor with ID: %u", actorID);
        return NULL;
    }
    global_state_bind( s_actor_instance_pool.objects[ actorID ] );

    //gCurrentObject->activeFlags=stateFlags;
    if (gCurrentObject->header.gfx.animInfo.curAnim==NULL)
        gCurrentObject->header.gfx.animInfo.curAnim=info->curAnim;

    if (gCurrentObject->header.gfx.animInfo.animID!=info->animID && info->animID!=-1){
        gCurrentObject->header.gfx.animInfo.animAccel=info->animAccel;
        gCurrentObject->header.gfx.animInfo.animFrameAccelAssist=info->animFrameAccelAssist;
        gCurrentObject->header.gfx.animInfo.animID=info->animID;
        gCurrentObject->header.gfx.animInfo.animFrame=0;
        //gCurrentObject->header.gfx.animInfo.curAnim=goomba_seg8_anims_0801DA4C[0];
        //gCurrentObject->header.gfx.throwMatrix=NULL;
    }else if (gCurrentObject->header.gfx.animInfo.animAccel!=info->animAccel){
        gCurrentObject->header.gfx.animInfo.animAccel=info->animAccel; // only change accel
    }

    gCurrentObject->header.gfx.scale[0] = scale[0];
    gCurrentObject->header.gfx.scale[1] = scale[1];
    gCurrentObject->header.gfx.scale[2] = scale[2];
    gCurrentObject->header.gfx.angle[0] = rot[0];
    gCurrentObject->header.gfx.angle[1] = rot[1];
    gCurrentObject->header.gfx.angle[2] = rot[2];

    gfx_adapter_bind_output_buffers( outBuffers );

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

int getActorUsedTextures(){
    if (gCurrentObject->behavior==bhvGoomba){
        return 1;
    }else if (gCurrentObject->behavior==bhvStar){
        return 2;
    }
    return 0;
}
static const int texGoomba [1] = { 32};
static const int texStar [1] = { 32,32};
int* getActorWidths(){
    if (gCurrentObject->behavior==bhvGoomba){
        return texGoomba;
    }else if (gCurrentObject->behavior==bhvStar){
        return texStar;
    }
    return texGoomba;
}

int* getActorHeights(){
    if (gCurrentObject->behavior==bhvGoomba){
        return texGoomba;
    }else if (gCurrentObject->behavior==bhvStar){
        return texStar;
    }
    return texGoomba;
}