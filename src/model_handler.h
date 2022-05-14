#pragma once
#include "decomp/memory.h"
enum MarioModels{
    MODEL_MARIO,
    MODEL_LUIGI,
    MODEL_STEVE,
    MODEL_ALEX,
    MODEL_NECOARC,
    MODEL_VIBRI,
};

#define ModelsUsed 6

void initModels(struct AllocOnlyPool *pool);
void initActorModels(struct AllocOnlyPool *pool);
struct GraphNode* getModel(int model);
extern struct GraphNode **gLoadedGraphNodes;