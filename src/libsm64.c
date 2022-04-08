#ifndef SM64_LIB_EXPORT
    #define SM64_LIB_EXPORT
#endif

#define LIB_VER 3

#include "libsm64.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "decomp/include/PR/os_cont.h"
#include "decomp/engine/math_util.h"
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
#include "fpsLimitHelper.h"
#include "decomp/pc/audio/audio_null.h"
#include "decomp/pc/audio/audio_wasapi.h"
#include "decomp/pc/audio/audio_pulse.h"
#include "decomp/pc/audio/audio_alsa.h"
#include "decomp/audio/external.h"

#include "decomp/audio/load_dat.h"
#include "decomp/tools/convUtils.h"

static struct AllocOnlyPool *s_mario_geo_pool = NULL;

static s16 lastWedges = 8;
//static struct GraphNode *s_mario_graph_node = NULL;
static int currentModel=0;
static struct AudioAPI *audio_api;

static bool s_init_global = false;
static bool s_init_one_mario = false;
bool hasAudio;
struct MarioInstance
{
    struct GlobalState *globalState;
};
struct ObjPool s_mario_instance_pool = { 0, 0 };

static void update_button( bool on, u16 button )
{
    gController.buttonPressed &= ~button;

    if( on )
    {
        if(( gController.buttonDown & button ) == 0 )
            gController.buttonPressed |= button;

        gController.buttonDown |= button;
    }
    else 
    {
        gController.buttonDown &= ~button;
    }
}

static struct Area *allocate_area( void )
{
    struct Area *result = malloc( sizeof( struct Area ));
    memset( result, 0, sizeof( struct Area ));

    result->flags = 1;
    result->camera = malloc( sizeof( struct Camera ));
    memset( result->camera, 0, sizeof( struct Camera ));

    return result;
}

static void free_area( struct Area *area )
{
    free( area->camera );
    free( area );
}

int getCurrentModel(){
    return currentModel;
}

pthread_t gSoundThread;

SM64_LIB_FN void sm64_global_init( uint8_t *rom,uint8_t *bank_sets,uint8_t *sequences_bin,uint8_t *sound_data_ctl,uint8_t *sound_data_tbl,int bank_set_len,int sequences_len,int ctl_len,int tbl_len, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction )
{
    hasAudio=false;
    if (bank_set_len != 0 & sequences_len != 0 & ctl_len != 0 & tbl_len != 0)
    {
        hasAudio=true;
        gBankSetsData=malloc(bank_set_len);
        gMusicData=malloc(sequences_len);
        gSoundDataADSR=malloc(ctl_len);
        gSoundDataRaw=malloc(tbl_len);
        memcpy(gBankSetsData,bank_sets,bank_set_len);
        memcpy(gMusicData,sequences_bin,sequences_len);
        memcpy(gSoundDataADSR,sound_data_ctl,ctl_len);
        memcpy(gSoundDataRaw,sound_data_tbl,tbl_len);
    }

    if( s_init_global )
        sm64_global_terminate();
    s_init_global = true;
    g_debug_print_func = debugPrintFunction;

    load_mario_textures_from_rom( rom, outTexture );
    load_mario_anims_from_rom( rom );

    memory_init();
//#define HAVE_WASAPI 1
    if (hasAudio){
    #if HAVE_WASAPI
        if (audio_api == NULL && audio_wasapi.init()) {
            audio_api = &audio_wasapi;
        }
    #endif
    #if HAVE_PULSE_AUDIO
        if (audio_api == NULL && audio_pulse.init()) {
            audio_api = &audio_pulse;
        }
    #endif
    #if HAVE_ALSA
        if (audio_api == NULL && audio_alsa.init()) {
            audio_api = &audio_alsa;
        }
    #endif
    #ifdef TARGET_WEB
        if (audio_api == NULL && audio_sdl.init()) {
            audio_api = &audio_sdl;
        }
    #endif
        if (audio_api == NULL) {
            audio_api = &audio_null;
        }
        
        audio_init();
        sound_init();
        sound_reset(0);
        // start audio thread
        pthread_create(&gSoundThread, NULL, audio_thread, NULL);
    }else{
        DEBUG_PRINT("No audio support");
    }

    /// test
    //sound_bank_header sbh = read_sound_bank(rom,5748512);
    //sound_data_header sdh = read_sound_data(rom,5846368);
    ALSeqFile* asq = parse_seqfile(rom+5748512);
}

SM64_LIB_FN void sm64_global_init_audioBin(uint8_t *rom,char* audioData, uint8_t *outTexture, SM64DebugPrintFunctionPtr debugPrintFunction){
    // file format: audioDataTblSize,soundDataCtlSize,bankSetsSize,sequencesSize,soundDataTBL,soundDataCtl,bankSets,sequences
    int audioDataTblSize = *(int*)audioData;
    int soundDataCtlSize = *(int*)(audioData+4);
    int bankSetsSize = *(int*)(audioData+8);
    int sequencesSize = *(int*)(audioData+12);
    char* soundDataTBL = audioData+16;
    char* soundDataCTL = audioData+16+audioDataTblSize;
    char* bankSets = audioData+16+audioDataTblSize+soundDataCtlSize;
    char* sequences = audioData+16+audioDataTblSize+soundDataCtlSize+bankSetsSize;
    sm64_global_init(rom,bankSets,sequences,soundDataCTL,soundDataTBL,bankSetsSize,sequencesSize,soundDataCtlSize,audioDataTblSize,outTexture,debugPrintFunction);
}

SM64_LIB_FN void sm64_global_terminate( void )
{
    if( !s_init_global ) return;

    global_state_bind( NULL );
    
    if( s_init_one_mario )
    {
        for( int i = 0; i < s_mario_instance_pool.size; ++i )
            if( s_mario_instance_pool.objects[i] != NULL )
                sm64_mChar_delete( i );

        obj_pool_free_all( &s_mario_instance_pool );
    }

    s_init_global = false;
    s_init_one_mario = false;
       
    alloc_only_pool_free( s_mario_geo_pool );
    surfaces_unload_all();
    unload_mario_anims();
    memory_terminate();
}

SM64_LIB_FN void sm64_static_surfaces_load( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    surfaces_load_static( surfaceArray, numSurfaces );
}

SM64_LIB_FN int32_t sm64_mChar_create( float x, float y, float z )
{
    int32_t marioIndex = obj_pool_alloc_index( &s_mario_instance_pool, sizeof( struct MarioInstance ));
    struct MarioInstance *newInstance = s_mario_instance_pool.objects[marioIndex];

    newInstance->globalState = global_state_create();
    global_state_bind( newInstance->globalState );

    if( !s_init_one_mario )
    {
        s_init_one_mario = true;
        s_mario_geo_pool = alloc_only_pool_init();
        initModels(s_mario_geo_pool);
        //s_mario_graph_node = getModel(MODEL_LUIGI);
    }

    gCurrSaveFileNum = 1;
    gMarioObject = hack_allocate_mario();
    gCurrentArea = allocate_area();
    gCurrentObject = gMarioObject;

    gMarioSpawnInfoVal.startPos[0] = x;
    gMarioSpawnInfoVal.startPos[1] = y;
    gMarioSpawnInfoVal.startPos[2] = z;

    gMarioSpawnInfoVal.startAngle[0] = 0;
    gMarioSpawnInfoVal.startAngle[1] = 0;
    gMarioSpawnInfoVal.startAngle[2] = 0;

    gMarioSpawnInfoVal.areaIndex = 0;
    gMarioSpawnInfoVal.activeAreaIndex = 0;
    gMarioSpawnInfoVal.behaviorArg = 0;
    gMarioSpawnInfoVal.behaviorScript = NULL;
    gMarioSpawnInfoVal.unk18 = NULL;
    gMarioSpawnInfoVal.next = NULL;

    init_mario_from_save_file();

    if( init_mario() < 0 )
    {
        sm64_mChar_delete( marioIndex );
        return -1;
    }

    set_mario_action( gMarioState, ACT_SPAWN_SPIN_AIRBORNE, 0);
    find_floor( x, y, z, &gMarioState->floor );

    return marioIndex;
}

SM64_LIB_FN struct AnimInfo* sm64_get_anim_info(int32_t marioId,int16_t rot[3]){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to get anim for non-existant Mario with ID: %u", marioId);
        return;
    }
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    (rot)[0]=gMarioState->marioObj->header.gfx.angle[0];
    (rot)[1]=gMarioState->marioObj->header.gfx.angle[1];
    (rot)[2]=gMarioState->marioObj->header.gfx.angle[2];
    return &gMarioState->marioObj->header.gfx.animInfo;
}

SM64_LIB_FN void sm64_mChar_animTick(int32_t marioId, uint32_t stateFlags,struct AnimInfo* info,struct SM64MarioGeometryBuffers *outBuffers,int32_t model,int16_t rot[3]){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Mario with ID: %u", marioId);
        return;
    }
    
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    gMarioState->marioObj->header.gfx.angle[0]=rot[0];
    gMarioState->marioObj->header.gfx.angle[1]=rot[1];
    gMarioState->marioObj->header.gfx.angle[2]=rot[2];

    if (!fpsLimit(30,gMarioState->lastTime))
        return;
    gMarioState->lastTime = timeInMilliseconds();

    gMarioState->flags = stateFlags;
    if (gMarioState->marioObj->header.gfx.animInfo.animFrame!=info->animID && info->animID!=-1)
        set_mario_anim_with_accel( gMarioState, info->animID,info->animAccel );
    gMarioState->marioObj->header.gfx.animInfo.animAccel=info->animAccel;

    currentModel = model;
    gfx_adapter_bind_output_buffers( outBuffers );
    geo_process_root_hack_single_node( getModel(currentModel) );
    gAreaUpdateCounter++;
}

SM64_LIB_FN void sm64_mChar_tick( int32_t marioId, const struct SM64MarioInputs *inputs, struct SM64MarioState *outState, struct SM64MarioGeometryBuffers *outBuffers )
{
    
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to tick non-existant Mario with ID: %u", marioId);
        return;
    }

    currentModel = outState->currentModel;
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    gMarioState->marioObj->header.gfx.cameraToObject[0]=0;
    gMarioState->marioObj->header.gfx.cameraToObject[1]=0;
    gMarioState->marioObj->header.gfx.cameraToObject[2]=0;
    if (!fpsLimit(30,gMarioState->lastTime))
        return;
    gMarioState->lastTime = timeInMilliseconds();

    update_button( inputs->buttonA, A_BUTTON );
    update_button( inputs->buttonB, B_BUTTON );
    update_button( inputs->buttonZ, Z_TRIG );

    gMarioState->area->camera->yaw = atan2s( inputs->camLookZ, inputs->camLookX );

    gController.stickX = -64.0f * inputs->stickX;
    gController.stickY = 64.0f * inputs->stickY;
    gController.stickMag = sqrtf( gController.stickX*gController.stickX + gController.stickY*gController.stickY );

    apply_mario_platform_displacement();
    bhv_mario_update();
    update_mario_platform(); // TODO platform grabbed here and used next tick could be a use-after-free

    gfx_adapter_bind_output_buffers( outBuffers );

    geo_process_root_hack_single_node( getModel(currentModel) );

    gAreaUpdateCounter++;

    outState->health = gMarioState->health;
    vec3f_copy( outState->position, gMarioState->pos );
    vec3f_copy( outState->velocity, gMarioState->vel );
    outState->faceAngle = (float)gMarioState->faceAngle[1] / 32768.0f * 3.14159f;
    outState->flags = gMarioState->flags;
    outState->action = gMarioState->action;

    s16 numHealthWedges = gMarioState->health > 0 ? gMarioState->health >> 8 : 0;
    if (numHealthWedges > lastWedges) {
            play_sound(SOUND_MENU_POWER_METER, gGlobalSoundSource);
        }
        lastWedges = numHealthWedges;

}

SM64_LIB_FN void sm64_mChar_delete( int32_t marioId )
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to delete non-existant Mario with ID: %u", marioId);
        return;
    }

    struct GlobalState *globalState = ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState;
    global_state_bind( globalState );

    free( gMarioObject );
    free_area( gCurrentArea );

    global_state_delete( globalState );
    obj_pool_free_index( &s_mario_instance_pool, marioId );
}

SM64_LIB_FN void sm64_mChar_teleport( int32_t marioId, float x, float y, float z )
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to change position of non-existant Mario with ID: %u", marioId);
        return;
    }
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    gMarioState->pos[0]=x;
    gMarioState->pos[1]=y;
    gMarioState->pos[2]=z;
    gMarioState->marioObj->header.gfx.pos[0]=x;
    gMarioState->marioObj->header.gfx.pos[1]=y;
    gMarioState->marioObj->header.gfx.pos[2]=z;
}

SM64_LIB_FN void sm64_mChar_set_velocity( int32_t marioId, float x, float y, float z )
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to change position of non-existant Mario with ID: %u", marioId);
        return;
    }
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    gMarioState->vel[0]=x;
    gMarioState->vel[1]=y;
    gMarioState->vel[2]=z;
}

SM64_LIB_FN void sm64_mChar_apply_damage( int32_t marioId, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc)
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to apply damage to non-existant Mario with ID: %u", marioId);
        return;
    }
    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    fake_damage_knock_back(gMarioState,damage,interactionSubtype,xSrc,ySrc,zSrc);
}

SM64_LIB_FN void sm64_mChar_set_state( int32_t marioId, uint32_t stateFlags )
{
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set state of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    gMarioState->flags = stateFlags;
}

SM64_LIB_FN void sm64_mChar_set_action( int32_t marioId, uint32_t actionId ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set action of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    set_mario_action( gMarioState, actionId, 0);
}

SM64_LIB_FN void sm64_mChar_set_action_state( int32_t marioId, u16 actionState ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set action-state of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );

    gMarioState->actionState = actionState;
}

SM64_LIB_FN void sm64_mChar_set_water_level( int32_t marioId, signed int yLevel ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set water level of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    gMarioState->waterLevel = yLevel;
}

SM64_LIB_FN void sm64_mChar_set_gas_level( int32_t marioId, signed int yLevel ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set gas level of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    gMarioState->gasLevel = yLevel;
}

SM64_LIB_FN void sm64_mChar_heal( int32_t marioId, char healCounter ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to heal non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    gMarioState->healCounter += healCounter;
}

SM64_LIB_FN void sm64_mChar_set_angle( int32_t marioId, float angle ){
    if( marioId >= s_mario_instance_pool.size || s_mario_instance_pool.objects[marioId] == NULL )
    {
        DEBUG_PRINT("Tried to set angle of non-existant Mario with ID: %u", marioId);
        return;
    }

    global_state_bind( ((struct MarioInstance *)s_mario_instance_pool.objects[ marioId ])->globalState );
    gMarioState->faceAngle[1] = (short)(angle * (32768.0f * 3.14159f));
    gMarioState->marioObj->header.gfx.angle[1]=(short)(angle * (32768.0f * 3.14159f));
    //gMarioState->usedObj->rawData.asS32[(0x0F+1)]=gMarioState->faceAngle[1]; // oMoveAngleYaw = faceAngle[1];
}

SM64_LIB_FN uint32_t sm64_surface_object_create( const struct SM64SurfaceObject *surfaceObject )
{
    uint32_t id = surfaces_load_object( surfaceObject );
    return id;
}

SM64_LIB_FN void sm64_surface_object_move( uint32_t objectId, const struct SM64ObjectTransform *transform )
{
    surface_object_update_transform( objectId, transform );
}

SM64_LIB_FN void sm64_surface_object_delete( uint32_t objectId )
{
    // A mario standing on the platform that is being destroyed will have a pointer to freed memory if we don't clear it.
    for( int i = 0; i < s_mario_instance_pool.size; ++i )
    {
        struct GlobalState *state = ((struct MarioInstance *)s_mario_instance_pool.objects[ i ])->globalState;
        if( state->mgMarioObject->platform == surfaces_object_get_transform_ptr( objectId ))
            state->mgMarioObject->platform = NULL;
    }

    surfaces_unload_object( objectId );
}

SM64_LIB_FN void sm64_seq_player_play_sequence(u8 player, u8 seqId, u16 arg2){
    seq_player_play_sequence(player,seqId,arg2);
}

SM64_LIB_FN void sm64_play_music(u8 player, u16 seqArgs, u16 fadeTimer){
    play_music(player,seqArgs,fadeTimer);
}

SM64_LIB_FN void sm64_stop_background_music(u16 seqId){
    stop_background_music(seqId);
}

SM64_LIB_FN void sm64_fadeout_background_music(u16 arg0, u16 fadeOut){
    fadeout_background_music(arg0,fadeOut);
}

SM64_LIB_FN u16 sm64_get_current_background_music(){
    return get_current_background_music();
}

SM64_LIB_FN void sm64_play_sound(s32 soundBits, f32 *pos){
    play_sound(soundBits,pos);
}

SM64_LIB_FN void sm64_play_sound_global(s32 soundBits){
    play_sound(soundBits,gGlobalSoundSource);
}

SM64_LIB_FN int sm64_set_volume(float vol){
    if (vol>1)
        vol=1;
    if (vol<0)
        vol=0;
    globalVolume=vol;
}

SM64_LIB_FN int sm64_get_version(){
    return LIB_VER; // used for compability checking with Retro64
}

#ifdef VERSION_EU
#define SAMPLES_HIGH 656
#define SAMPLES_LOW 640
#else
#define SAMPLES_HIGH 544
#define SAMPLES_LOW 528
#endif

void audio_tick(){
    
    int samples_left = audio_api->buffered();
    u32 num_audio_samples = samples_left < audio_api->get_desired_buffered() ? SAMPLES_HIGH : SAMPLES_LOW;
    //printf("Audio samples: %d %u\n", samples_left, num_audio_samples);
    s16 audio_buffer[SAMPLES_HIGH * 2 * 2];
    for (int i = 0; i < 2; i++) {
        /*if (audio_cnt-- == 0) {
            audio_cnt = 2;
        }
        u32 num_audio_samples = audio_cnt < 2 ? 528 : 544;*/
        create_next_audio_buffer(audio_buffer + i * (num_audio_samples * 2), num_audio_samples);
    }

    // lower the volume of the audio by a factor of 2

    audio_api->play((u8 *)audio_buffer, 2 * num_audio_samples * 4);
}

#include<sys/time.h>
void audio_thread(){
    long long currentTime=timeInMilliseconds();
    long long targetTime=0;
    while(1){
        audio_signal_game_loop_tick();
        audio_tick();
        targetTime=currentTime+33;
        while (timeInMilliseconds() < targetTime) {
            usleep(100);
        }
        currentTime=timeInMilliseconds();
    }
}