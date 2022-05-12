#pragma once
#include "obj_pool.h"
enum ObjectList getActorObjList(int actorID);
struct ObjPool* getActorPool();
void tickAllActors();
int putObjectInActorPool(struct Object* obj);
struct Object* getActor(int id);
#define isTickingActor gCurrentObject!=gMarioObject && gCurrentObject->behavior!=0x0