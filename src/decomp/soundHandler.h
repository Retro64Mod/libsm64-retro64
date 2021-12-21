#pragma once
#include "../libsm64.h"

struct SM64Sound {
    s32 soundBits;
    f32 *position;
};

extern SM64_LIB_FN u8 sm64_getSoundRequests(struct SM64Sound*);