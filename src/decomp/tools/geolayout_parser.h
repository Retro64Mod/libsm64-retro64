#pragma once

#include "../include/types.h"

GeoLayout* parse_full_geolayout(unsigned char* rom,unsigned char* data,unsigned char* MI0_data);
_Bool parse_command(unsigned char* rom,unsigned char* data,GeoLayout* buf,uint8_t *cmd_length,unsigned char* MI0_data);
uintptr_t convertPtr(unsigned int oldPtr);
GeoLayout* convertPtr_follow(unsigned char* rom,unsigned int oldPtr,unsigned char* MI0_data);