#include <stddef.h>
#include <stdbool.h>
#include "decomp/mario/geo.inc.h"
#include "decomp/model_luigi/geo.inc.h"
#include "decomp/model_steve/geo.inc.h"
#include "decomp/model_alex/geo.inc.h"
#include "decomp/model_necoarc/geo.inc.h"
#include "decomp/model_vibri/geo.inc.h"
// models done
#include "decomp/model_goomba/geo.inc.h"
#include "decomp/model_coin/geo.inc.h"
#include "decomp/model_star/geo.inc.h"
#include "decomp/engine/geo_layout.h"
#include "model_handler.h"
#include "load_tex_data.h"
#include "decomp/include/model_ids.h"
#include "decomp/tools/geolayout_parser.h"

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
    gLoadedGraphNodes[MODEL_GOOMBA]=process_geo_layout(s_actor_geo_pool,convertPtr_follow(rom,0x0F0006E4)); // parse_full_geolayout
    gLoadedGraphNodes[MODEL_YELLOW_COIN]=process_geo_layout(s_actor_geo_pool,yellow_coin_geo_ptr);
    gLoadedGraphNodes[MODEL_STAR]=process_geo_layout(s_actor_geo_pool,convertPtr_follow(rom,0x0F0006E4));
}

struct GraphNode* getModel(int model){
    if (model<0 || model>=ModelsUsed){
        model=0;
    }
    return model_nodes[model];
}