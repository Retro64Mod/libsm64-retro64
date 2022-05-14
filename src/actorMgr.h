#pragma once
#include "obj_pool.h"
#include "decomp/shim.h"
#define ACTOR_TYPE_GOOMBA 0
#define ACTOR_TYPE_STAR 1

enum ObjectList getActorObjList(int actorID);
struct ObjPool* getActorPool();
void tickAllActors();
int putObjectInActorPool(struct Object* obj);
struct Object* getActor(int id);
#define isTickingActor gCurrentObject!=gMarioObject && gCurrentObject->behavior!=0x0
int getActorUsedTextures();
int* getActorWidths();
int* getActorHeights();