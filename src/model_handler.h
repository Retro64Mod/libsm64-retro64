#pragma once
#include "decomp/memory.h"
enum MarioModels{
    MODEL_MARIO,
    MODEL_LUIGI,
    MODEL_STEVE,
    MODEL_ALEX,
    MODEL_NECOARC,
    MODEL_VIBRI,
    MODEL_SONIC
};

#define ModelsUsed 7

void initModels(struct AllocOnlyPool *pool);
void initActorModels(char* rom);
struct GraphNode* getModel(int model);
extern struct GraphNode **gLoadedGraphNodes;