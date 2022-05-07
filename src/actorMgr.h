#pragma once
#include "obj_pool.h"
enum ObjectList getActorObjList(int actorID);
struct ObjPool* getActorPool();
void tickAllActors();