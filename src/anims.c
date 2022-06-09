#include "decomp/game/actors/common0.h"
#include "decomp/include/model_ids.h"
#include "decomp/tools/anim_parser.h"

struct Animation * goomba_seg8_anims_0801DA4C[]={
    NULL,NULL,NULL
};

void loadAnims(unsigned char* MI0_data,int model){
    switch (model){
        case MODEL_GOOMBA:
            struct Animation** goombaAnims = parse_anim_table(MI0_data,0x801DA4C,3);
            goomba_seg8_anims_0801DA4C[0] = goombaAnims[0];
            goomba_seg8_anims_0801DA4C[1] = goombaAnims[1];
            goomba_seg8_anims_0801DA4C[2] = goombaAnims[2];
            
            break;
    }
}