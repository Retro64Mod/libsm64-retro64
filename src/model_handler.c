#include "decomp/mario/geo.inc.h"
#include "decomp/model_luigi/geo.inc.h"
#include "decomp/model_steve/geo.inc.h"
#include "decomp/model_alex/geo.inc.h"
#include "decomp/model_necoarc/geo.inc.h"
// models done
#include "decomp/engine/geo_layout.h"
#include "model_handler.h"
#include "load_tex_data.h"


static void* modelPointers[ModelsUsed]={0x0};

static struct GraphNode* model_nodes[ModelsUsed]={0x0};

void initModels(struct AllocOnlyPool *pool){
    modelPointers[MODEL_MARIO] = (void*)mario_geo_ptr;
    modelPointers[MODEL_LUIGI] = (void*)luigi_geo_ptr;
    modelPointers[MODEL_STEVE] = (void*)steve_geo_ptr;
    modelPointers[MODEL_ALEX] = (void*)alex_geo_ptr;
    modelPointers[MODEL_NECOARC] = (void*)necoarc_geo_ptr;

    for (int i = 0;i<ModelsUsed;i++){
        model_nodes[i]=process_geo_layout( pool, modelPointers[i] );
    }
}

struct GraphNode* getModel(int model){
    if (model<0 || model>=ModelsUsed){
        model=0;
    }
    return model_nodes[model];
}