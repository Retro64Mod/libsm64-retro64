#pragma once
#include "decomp/memory.h"
enum MarioModels{
    MODEL_MARIO,
    MODEL_LUIGI,
    MODEL_STEVE,
    MODEL_ALEX,
};

#define ModelsUsed 4

void initModels(struct AllocOnlyPool *pool);
struct GraphNode* getModel(int model);