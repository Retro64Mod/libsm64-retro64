#include <stdbool.h>
#include <sys/time.h>
#include "fpsLimitHelper.h"
#define NULL 0x0
long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv,NULL);
    return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
}


bool fpsLimit(float targetFps,long long lastTime) {
    long long currentTime = timeInMilliseconds();
    long long delta = currentTime - lastTime;
    float targetTime = 1000.0f / (targetFps*1.15f);
    if (delta > targetTime) {
        return true;
    }
    return false;
}

