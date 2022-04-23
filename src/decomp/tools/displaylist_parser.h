#pragma once
#ifndef NO_GEO_PLS
#include "../../gfx_macros.h"
#endif

Gfx* convertDLPtr(unsigned char* rom,unsigned int ptr);
Gfx* parseGFX(unsigned char* mario_data,unsigned char* rawData);
Vtx* parseVTX(unsigned char* mario_data,unsigned char* rawData,int amount);