#pragma once
#ifndef NO_GEO_PLS
#include "../../gfx_macros.h"
#endif

Gfx* convertDLPtrMD(unsigned char* uncompressed_data,unsigned int ptr);
Gfx* parseGFX(unsigned char* mario_data,unsigned char* rawData);
Vtx* parseVTX(unsigned char* mario_data,unsigned char* rawData,int amount);