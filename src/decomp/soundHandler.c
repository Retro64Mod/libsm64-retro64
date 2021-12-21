#include "include/types.h"
#include "soundHandler.h"

u8 sSoundRequestCount = 0;

struct SM64Sound sSoundRequests[0x100];

void play_sound(s32 soundBits, f32 *pos) {
    // add a request to play the sound to the "queue"
    sSoundRequests[sSoundRequestCount].soundBits = soundBits;
    sSoundRequests[sSoundRequestCount].position = pos;
    sSoundRequestCount++;
}

SM64_LIB_FN u8 sm64_getSoundRequests(struct SM64Sound* snds){
    u8 totalReqs = sSoundRequestCount;
    sSoundRequestCount=0; // reset queue counter
    for (int i = 0;i<totalReqs;i++){
        snds[i]=sSoundRequests[i]; // send all sound requests up
    }
    return totalReqs;
}