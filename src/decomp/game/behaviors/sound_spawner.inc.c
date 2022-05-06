#include "../interaction.h"
#include "../../include/object_constants.h"
#include "../../include/audio_defines.h"
#include "../../include/model_ids.h"
#include "../obj_behaviors_2.h"
#include "../object_helpers.h"
#include "../../include/object_fields.h"
#include "../../include/behavior_data.h"
#include "../../shim.h"
#include "../behavior_actions.h"
#define o gCurrentObject
// sound_spawner.inc.c

void bhv_sound_spawner_init(void) {
    s32 sp1C = o->oSoundEffectUnkF4;
    play_sound(sp1C, o->header.gfx.cameraToObject);
}
