#include "include/types.h"

u8 sSoundRequestCount = 0;

struct Sound {
    s32 soundBits;
    f32 *position;
}; // size = 0x8

struct Sound sSoundRequests[0x100];

void play_sound(s32 soundBits, f32 *pos) {
    // add a request to play the sound to the "queue"
    sSoundRequests[sSoundRequestCount].soundBits = soundBits;
    sSoundRequests[sSoundRequestCount].position = pos;
    sSoundRequestCount++;
}


__declspec(dllexport) u8 sm64_getSoundRequests(struct Sound* snds){
    u8 totalReqs = sSoundRequestCount;
    sSoundRequestCount=0; // reset queue counter
    for (int i = 0;i<totalReqs;i++){
        snds[i]=sSoundRequests[i]; // send all sound requests up
    }
    return totalReqs;
}