#pragma once

#define SONIC_USED_TEXTURES 8

static const int sonic_tex_widths[SONIC_USED_TEXTURES] = { 64,64,64,64,32,32,32,32 };
static const int sonic_tex_heights[SONIC_USED_TEXTURES] = { 64,64,64,64,32,32,64,64 };

extern void *sonic_geo_ptr;