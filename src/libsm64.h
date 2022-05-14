#ifndef LIB_SM64_H
#define LIB_SM64_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "fake_interaction.h"

#ifdef _WIN32
    #ifdef SM64_LIB_EXPORT
        #define SM64_LIB_FN __declspec(dllexport)
    #else
        #define SM64_LIB_FN __declspec(dllimport)
    #endif
#else
    #define SM64_LIB_FN
#endif

struct SM64Surface
{
    int16_t type;
    int16_t force;
    uint16_t terrain;
    int32_t vertices[3][3];
};

struct SM64MarioInputs
{
    float camLookX, camLookZ;
    float cameraPosition[3]; // used for sound calculations
    float stickX, stickY;
    uint8_t buttonA, buttonB, buttonZ;
};

struct SM64ObjectTransform
{
    float position[3];
    float eulerRotation[3];
};

struct SM64SurfaceObject
{
    struct SM64ObjectTransform transform;
    uint32_t surfaceCount;
    struct SM64Surface *surfaces;
};

struct SM64MarioState
{
    float position[3];
    float velocity[3];
    float faceAngle;
    int16_t health;
    uint32_t flags;
    uint32_t action;
    uint32_t currentModel;
};

struct SM64ActorState
{
    float position[3];
    float velocity[3];
    short rotation[3];
    float scale[3];
};

struct SM64MarioGeometryBuffers
{
    float *position;
    float *normal;
    float *color;
    float *uv;
    uint16_t numTrianglesUsed;
};

typedef void (*SM64DebugPrintFunctionPtr)( const char * );

enum
{
    SM64_TEXTURE_WIDTH = 64 * 11,
    SM64_TEXTURE_HEIGHT = 64,
    SM64_GEO_MAX_TRIANGLES = 2040,
};

extern SM64_LIB_FN void sm64_global_init( uint8_t *rom,uint8_t *bank_sets,uint8_t *sequences_bin,uint8_t *sound_data_ctl,uint8_t *sound_data_tbl,int bank_set_len,int sequences_len,int ctl_len,int tbl_len, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction );
extern SM64_LIB_FN void sm64_global_init_audioBin(uint8_t *rom,char* audioData, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction);
extern SM64_LIB_FN void sm64_global_terminate( void );

extern SM64_LIB_FN void sm64_static_surfaces_load( const struct SM64Surface *surfaceArray, uint32_t numSurfaces );

extern SM64_LIB_FN int32_t sm64_mChar_create( float x, float y, float z );
extern SM64_LIB_FN struct AnimInfo* sm64_get_anim_info(int32_t marioId,int16_t rot[3]);
extern SM64_LIB_FN void sm64_mChar_animTick(int32_t marioId, uint32_t stateFlags,struct AnimInfo* info,struct SM64MarioGeometryBuffers *outBuffers,int32_t model,int16_t rot[3]);
extern SM64_LIB_FN void sm64_mChar_tick( int32_t marioId, const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers );
extern SM64_LIB_FN void sm64_mChar_delete( int32_t marioId );
extern SM64_LIB_FN void sm64_mChar_teleport(int32_t marioId, float x, float y, float z);
extern SM64_LIB_FN void sm64_mChar_set_velocity( int32_t marioId, float x, float y, float z );
extern SM64_LIB_FN void sm64_mChar_apply_damage( int32_t marioId, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc);
extern SM64_LIB_FN void sm64_mChar_set_state(int32_t marioId, uint32_t capType);
extern SM64_LIB_FN void sm64_mChar_set_action( int32_t marioId, uint32_t actionId );
extern SM64_LIB_FN void sm64_mChar_set_action_state( int32_t marioId, u16 actionState );
extern SM64_LIB_FN void sm64_mChar_set_water_level( int32_t marioId, signed int yLevel );
extern SM64_LIB_FN void sm64_mChar_set_gas_level( int32_t marioId, signed int yLevel );
extern SM64_LIB_FN void sm64_mChar_set_angle( int32_t marioId, float angle );
extern SM64_LIB_FN void sm64_mChar_heal( int32_t marioId, char healCounter );

extern SM64_LIB_FN uint32_t sm64_surface_object_create( const struct SM64SurfaceObject *surfaceObject );
extern SM64_LIB_FN void sm64_surface_object_move( uint32_t objectId, const struct SM64ObjectTransform *transform );
extern SM64_LIB_FN void sm64_surface_object_delete( uint32_t objectId );

extern SM64_LIB_FN void sm64_seq_player_play_sequence(u8 player, u8 seqId, u16 arg2);
extern SM64_LIB_FN void sm64_play_music(u8 player, u16 seqArgs, u16 fadeTimer);
extern SM64_LIB_FN void sm64_stop_background_music(u16 seqId);
extern SM64_LIB_FN void sm64_fadeout_background_music(u16 arg0, u16 fadeOut);
extern SM64_LIB_FN u16 sm64_get_current_background_music();
extern SM64_LIB_FN void sm64_play_sound(s32 soundBits, f32 *pos);
extern SM64_LIB_FN void sm64_play_sound_global(s32 soundBits);
extern SM64_LIB_FN int sm64_set_volume(float vol);
extern SM64_LIB_FN int sm64_get_version();
extern bool hasAudio;
extern int getCurrentModel();
extern struct ObjPool s_mario_instance_pool;
void audio_tick();
void audio_thread();

// actorMgr.c
extern SM64_LIB_FN int initActor(int actorType,float x,float y,float z);
extern SM64_LIB_FN void destroyActor(int actorID);
extern SM64_LIB_FN struct AnimInfo* tickActor(int actorID,struct SM64ActorState* state,struct SM64MarioGeometryBuffers *outBuffers);
SM64_LIB_FN void tickActorAnim(int actorID,uint32_t stateFlags,struct AnimInfo* info,struct SM64MarioGeometryBuffers *outBuffers,int16_t rot[3],float scale[3]);
extern SM64_LIB_FN void sm64_actor_static_surfaces_load( int actorID,const struct SM64Surface *surfaceArray, uint32_t numSurfaces );

#endif//LIB_SM64_H
