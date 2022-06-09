#include <stddef.h>
#include <stdbool.h>
#include "decomp/mario/geo.inc.h"
#include "decomp/model_luigi/geo.inc.h"
#include "decomp/model_steve/geo.inc.h"
#include "decomp/model_alex/geo.inc.h"
#include "decomp/model_necoarc/geo.inc.h"
#include "decomp/model_vibri/geo.inc.h"
// models done
#include "decomp/model_coin/geo.inc.h"
#include "decomp/model_star/geo.inc.h"
#include "model_handler.h"
#include "decomp/engine/geo_layout.h"
#include "load_tex_data.h"
#include "decomp/include/model_ids.h"
#include "decomp/tools/geolayout_parser.h"
#include "decomp/tools/anim_parser.h"
#include "decomp/tools/utils.h"
#include "anims.h"

static void* modelPointers[ModelsUsed]={0x0};

static struct GraphNode* model_nodes[ModelsUsed]={0x0};
struct GraphNode* goomba_test;

struct GraphNode *D_8033A160[0x100];
struct GraphNode **gLoadedGraphNodes = D_8033A160;

static struct AllocOnlyPool *s_actor_geo_pool = NULL;
static bool s_init_one_actor = false;

void initModels(struct AllocOnlyPool *pool){
    modelPointers[MODEL_MARIO] = (void*)mario_geo_ptr;
    modelPointers[MODEL_LUIGI] = (void*)luigi_geo_ptr;
    modelPointers[MODEL_STEVE] = (void*)steve_geo_ptr;
    modelPointers[MODEL_ALEX] = (void*)alex_geo_ptr;
    modelPointers[MODEL_NECOARC] = (void*)necoarc_geo_ptr;
    modelPointers[MODEL_VIBRI] = (void*)vibri_geo_ptr;

    for (int i = 0;i<ModelsUsed;i++){
        model_nodes[i]=process_geo_layout( pool, modelPointers[i] );
    }
}

void initActorModels(char* rom){
    if (!s_init_one_actor){
        s_init_one_actor=true;
        s_actor_geo_pool = alloc_only_pool_init();
    }

    // GOOMBA //
    unsigned char* gData = load_MI0_data_from_rom(rom,0x1F2200); 
    GeoLayout* layout = convertPtr_follow(rom,0x0F0006E4,gData);
    gLoadedGraphNodes[MODEL_GOOMBA]=process_geo_layout(s_actor_geo_pool,layout);
    loadAnims(gData,MODEL_GOOMBA);
}

struct GraphNode* getModel(int model){
    if (model<0 || model>=ModelsUsed){
        model=0;
    }
    return model_nodes[model];
}