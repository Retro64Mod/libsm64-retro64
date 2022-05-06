#ifndef OBJ_BEHAVIORS_2_H
#define OBJ_BEHAVIORS_2_H

#include "../include/PR/ultratypes.h"

#include "../include/types.h"

#define ATTACK_HANDLER_NOP 0
#define ATTACK_HANDLER_DIE_IF_HEALTH_NON_POSITIVE 1
#define ATTACK_HANDLER_KNOCKBACK 2
#define ATTACK_HANDLER_SQUISHED 3
#define ATTACK_HANDLER_SPECIAL_KOOPA_LOSE_SHELL 4
#define ATTACK_HANDLER_SET_SPEED_TO_ZERO 5
#define ATTACK_HANDLER_SPECIAL_WIGGLER_JUMPED_ON 6
#define ATTACK_HANDLER_SPECIAL_HUGE_GOOMBA_WEAKLY_ATTACKED 7
#define ATTACK_HANDLER_SQUISHED_WITH_BLUE_COIN 8

void shelled_koopa_attack_handler(s32 attackType);
void obj_spit_fire(s16 relativePosX, s16 relativePosY, s16 relativePosZ, f32 scale, s32 model,
                   f32 startSpeed, f32 endSpeed, s16 movePitch);
void obj_set_speed_to_zero(void);
s32 obj_forward_vel_approach(f32 target, f32 delta);
s32 obj_y_vel_approach(f32 target, f32 delta);
s32 obj_move_pitch_approach(s16 target, s16 delta);
s32 cur_obj_play_sound_at_anim_range(s8 arg0, s8 arg1, u32 sound);
s32 obj_resolve_collisions_and_turn(s16 targetYaw, s16 turnSpeed);
s16 random_linear_offset(s16 base, s16 range);
s32 obj_bounce_off_walls_edges_objects(s32 *targetYaw);
s16 obj_random_fixed_turn(s16 delta);
void obj_update_blinking(s32 *blinkTimer, s16 baseCycleLength, s16 cycleLengthRange,s16 blinkLength);
obj_handle_attacks(struct ObjectHitbox *hitbox, s32 attackedMarioAction,u8 *attackHandlers);
s32 obj_update_standard_actions(f32 scale);
void treat_far_home_as_mario(f32 threshold);
void obj_die_if_health_non_positive(void);
s32 obj_resolve_object_collisions(s32 *targetYaw);
#endif // OBJ_BEHAVIORS_2_H
