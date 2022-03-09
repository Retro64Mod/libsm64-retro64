#include "decomp/global_state.h"

#include "debug_print.h"
#include "load_surfaces.h"
#include "gfx_adapter.h"
#include "load_anim_data.h"
#include "load_tex_data.h"
#include "obj_pool.h"

#include "decomp/game/interaction.h"
#include "decomp/include/object_fields.h"
#include "decomp/include/sm64.h"
#include "decomp/shim.h"
#include "decomp/game/mario.h"
#include "decomp/engine/math_util.h"
#include "fake_interaction.h"

#define sDelayInvincTimer (g_state->msDelayInvincTimer)
#define sInvulnerable     (g_state->msInvulnerable)

static u32 sForwardKnockbackActions[][3] = {
    { ACT_SOFT_FORWARD_GROUND_KB, ACT_FORWARD_GROUND_KB, ACT_HARD_FORWARD_GROUND_KB },
    { ACT_FORWARD_AIR_KB,         ACT_FORWARD_AIR_KB,    ACT_HARD_FORWARD_AIR_KB },
    { ACT_FORWARD_WATER_KB,       ACT_FORWARD_WATER_KB,  ACT_FORWARD_WATER_KB },
};

static u32 sBackwardKnockbackActions[][3] = {
    { ACT_SOFT_BACKWARD_GROUND_KB, ACT_BACKWARD_GROUND_KB, ACT_HARD_BACKWARD_GROUND_KB },
    { ACT_BACKWARD_AIR_KB,         ACT_BACKWARD_AIR_KB,    ACT_HARD_BACKWARD_AIR_KB },
    { ACT_BACKWARD_WATER_KB,       ACT_BACKWARD_WATER_KB,  ACT_BACKWARD_WATER_KB },
};

uint32_t fake_damage_knock_back(struct MarioState *m, uint32_t damage,uint32_t interactionSubtype,float xSrc,float ySrc,float zSrc) {

    if (!sInvulnerable && !(m->flags & MARIO_VANISH_CAP)
        && !(interactionSubtype & INT_SUBTYPE_DELAY_INVINCIBILITY)) {
        //o->oInteractStatus = INT_STATUS_INTERACTED | INT_STATUS_ATTACKED_MARIO;
        //m->interactObj = o;

        // calculate damage, substitute for take_damage_from_interact_object
        if (!(m->flags & MARIO_CAP_ON_HEAD)) {
            damage += (damage + 1) / 2;
        }

        if (m->flags & MARIO_METAL_CAP) {
            damage = 0;
        }

        m->hurtCounter += 4 * damage; // apply hurt counter

        if (interactionSubtype & INT_SUBTYPE_BIG_KNOCKBACK) {
            m->forwardVel = 40.0f;
        }

        if (damage > 0) {
            play_sound(SOUND_MARIO_ATTACKED, m->marioObj->header.gfx.cameraToObject);
        }

        //update_mario_sound_and_camera(m);
        return drop_and_set_mario_action(m, fake_determine_knockback_action(m, damage,xSrc,ySrc,zSrc),
                                         damage);
    }

    return FALSE;
}

s16 fake_mario_obj_angle_to_object(struct MarioState *m, float xSrc,float zSrc) {
    f32 dx = xSrc - m->pos[0];
    f32 dz = zSrc - m->pos[2];

    return atan2s(dz, dx);
}

u32 fake_determine_knockback_action(struct MarioState *m, s32 damage,float xSrc,float ySrc,float zSrc) {
    u32 bonkAction;

    s16 terrainIndex = 0; // 1 = air, 2 = water, 0 = default
    s16 strengthIndex = 0;

    s16 angleToObject = fake_mario_obj_angle_to_object(m, xSrc,zSrc);
    s16 facingDYaw = angleToObject - m->faceAngle[1];
    s16 remainingHealth = m->health - 0x40 * m->hurtCounter;

    if (m->action & (ACT_FLAG_SWIMMING | ACT_FLAG_METAL_WATER)) {
        terrainIndex = 2;
    } else if (m->action & (ACT_FLAG_AIR | ACT_FLAG_ON_POLE | ACT_FLAG_HANGING)) {
        terrainIndex = 1;
    }

    if (remainingHealth < 0x100) {
        strengthIndex = 2;
    } else if (damage >= 4) {
        strengthIndex = 2;
    } else if (damage >= 2) {
        strengthIndex = 1;
    }

    m->faceAngle[1] = angleToObject;

    if (terrainIndex == 2) {
        if (m->forwardVel < 28.0f) {
            mario_set_forward_vel(m, 28.0f);
        }

        if (m->pos[1] >= ySrc) {
            if (m->vel[1] < 20.0f) {
                m->vel[1] = 20.0f;
            }
        } else {
            if (m->vel[1] > 0.0f) {
                m->vel[1] = 0.0f;
            }
        }
    } else {
        if (m->forwardVel < 16.0f) {
            mario_set_forward_vel(m, 16.0f);
        }
    }

    if (-0x4000 <= facingDYaw && facingDYaw <= 0x4000) {
        m->forwardVel *= -1.0f;
        bonkAction = sBackwardKnockbackActions[terrainIndex][strengthIndex];
    } else {
        m->faceAngle[1] += 0x8000;
        bonkAction = sForwardKnockbackActions[terrainIndex][strengthIndex];
    }

    return bonkAction;
}