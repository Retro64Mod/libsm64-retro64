#include "load_tex_data.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "libsm64.h"

#include "decomp/tools/libmio0.h"
#include "decomp/tools/n64graphics.h"

#define MARIO_TEX_ROM_OFFSET 1132368
#define ATLAS_WIDTH (NUM_USED_TEXTURES * 64)
#define ATLAS_HEIGHT 64

static void blt_image_to_atlas( rgba *img, int i, int w, int h, uint8_t *outTexture )
{
    for( int iy = 0; iy < h; ++iy )
    for( int ix = 0; ix < w; ++ix )
    {
        int o = (ix + 64 * i) + iy * ATLAS_WIDTH;
        int q = ix + iy * w;
        outTexture[4*o + 0] = img[q].red;
        outTexture[4*o + 1] = img[q].green;
        outTexture[4*o + 2] = img[q].blue;
        outTexture[4*o + 3] = img[q].alpha;
    }
}

void load_mario_textures_from_rom( uint8_t *rom, uint8_t *outTexture )
{
    memset( outTexture, 0, 4 * ATLAS_WIDTH * ATLAS_HEIGHT );

    mio0_header_t head;
    uint8_t *in_buf = rom + MARIO_TEX_ROM_OFFSET;

    mio0_decode_header( in_buf, &head );
    uint8_t *out_buf = malloc( head.dest_size );
    mio0_decode( in_buf, out_buf, NULL );

    for( int i = 0; i < NUM_USED_TEXTURES; ++i )
    {
        uint8_t *raw = out_buf + mario_tex_offsets[i];
        rgba *img = raw2rgba( raw, mario_tex_widths[i], mario_tex_heights[i], 16 );
        blt_image_to_atlas( img, i, mario_tex_widths[i], mario_tex_heights[i], outTexture );
        free( img );
    }

    free( out_buf );
}

#include "decomp/model_luigi/geo.inc.h"
#include "decomp/model_steve/geo.inc.h"
#include "decomp/model_alex/geo.inc.h"
#include "decomp/model_necoarc/geo.inc.h"
#include "decomp/model_vibri/geo.inc.h"
#include "decomp/model_sonic/geo.inc.h"

int getUsedTextures(){
    switch (getCurrentModel()){
        case MODEL_MARIO:
            return NUM_USED_TEXTURES;
        case MODEL_LUIGI:
            return LUGI_USED_TEXTURES;
        case MODEL_STEVE:
            return STEVE_USED_TEXTURES;
        case MODEL_ALEX:
            return ALEX_USED_TEXTURES;
        case MODEL_NECOARC:
            return NECOARC_USED_TEXTURES;
        case MODEL_VIBRI:
            return VIBRI_USED_TEXTURES;
        case MODEL_SONIC:
            return SONIC_USED_TEXTURES;
        default:
            return 0;
    }
}

int* getWidths(){
    switch (getCurrentModel()){
        case MODEL_MARIO:
            return mario_tex_widths;
        case MODEL_LUIGI:
            return luigi_tex_widths;
        case MODEL_STEVE:
            return steve_tex_widths;
        case MODEL_ALEX:
            return alex_tex_widths;
        case MODEL_NECOARC:
            return necoarc_tex_widths;
        case MODEL_VIBRI:
            return vibri_tex_widths;
        case MODEL_SONIC:
            return sonic_tex_widths;
        default:
            return NULL;
    }
}

int* getHeights(){
    switch (getCurrentModel()){
        case MODEL_MARIO:
            return mario_tex_heights;
        case MODEL_LUIGI:
            return luigi_tex_heights;
        case MODEL_STEVE:
            return steve_tex_heights;
        case MODEL_ALEX:
            return alex_tex_heights;
        case MODEL_NECOARC:
            return necoarc_tex_heights;
        case MODEL_VIBRI:
            return vibri_tex_heights;
        case MODEL_SONIC:
            return sonic_tex_heights;
        default:
            return NULL;
    }
}