#include "../include/PR/ultratypes.h"

#include "../audio/external.h"
#include "../engine/geo_layout.h"
#include "../engine/graph_node.h"
#include "../engine/math_util.h"
#include "../engine/surface_collision.h"
//#include "level_table.h"
#include "../include/object_constants.h"
#include "../include/object_fields.h"
#include "object_helpers.h"
#include "object_list_processor.h"
#include "spawn_object.h"
#include "../include/types.h"
#include "../shim.h"
#include "object_stuff.h"
/**
 * An unused linked list struct that seems to have been replaced by ObjectNode.
 */
struct LinkedList {
    struct LinkedList *next;
    struct LinkedList *prev;
};

/**
 * Clear the doubly linked usedList. Singly link each item in the pool into
 * a list, and return this list in pFreeList.
 * Appears to have been replaced by init_free_object_list.
 */
void unused_init_free_list(struct LinkedList *usedList, struct LinkedList **pFreeList,
                           struct LinkedList *pool, s32 itemSize, s32 poolLength) {
    s32 i;
    struct LinkedList *node = pool;

    usedList->next = usedList;
    usedList->prev = usedList;

    *pFreeList = pool;

    for (i = 0; i < poolLength - 1; i++) {
        // Add next node to free list
        node = (struct LinkedList *) ((u8 *) node + itemSize);
        pool->next = node;
        pool = node;
    }

    // End the list
    pool->next = NULL;
}

/**
 * Attempt to allocate a node from freeList (singly linked) and append it
 * to the end of destList (doubly linked). Return the object, or NULL if
 * freeList is empty.
 * Appears to have been replaced by try_allocate_object.
 */
struct LinkedList *unused_try_allocate(struct LinkedList *destList,
                                       struct LinkedList *freeList) {
    struct LinkedList *node = freeList->next;

    if (node != NULL) {
        // Remove from free list
        freeList->next = node->next;

        // Insert at the end of destination list
        node->prev = destList->prev;
        node->next = destList;
        destList->prev->next = node;
        destList->prev = node;
    }

    return node;
}

/**
 * Remove the node from the doubly linked list it's in, and place it in the
 * singly linked freeList.
 * This function seems to have been replaced by deallocate_object.
 */
void unused_deallocate(struct LinkedList *freeList, struct LinkedList *node) {
    // Remove from doubly linked list
    node->next->prev = node->prev;
    node->prev->next = node->next;

    // Insert at beginning of singly linked list
    node->next = freeList->next;
    freeList->next = node;
}
/**
 * Remove the given object from the object list that it's currently in, and
 * insert it at the beginning of the free list (singly linked).
 */
static void deallocate_object(struct ObjectNode *freeList, struct ObjectNode *obj) {
    // Remove from object list
    obj->next->prev = obj->prev;
    obj->prev->next = obj->next;

    // Insert at beginning of free list
    obj->next = freeList->next;
    freeList->next = obj;
}

/**
 * Add every object in the pool to the free object list.
 */
void init_free_object_list(void) {
    s32 i;
    s32 poolLength = OBJECT_POOL_CAPACITY;

    // Add the first object in the pool to the free list
    struct Object *obj = &gObjectPool[0];
    gFreeObjectList.next = (struct ObjectNode *) obj;

    // Link each object in the pool to the following object
    for (i = 0; i < poolLength - 1; i++) {
        obj->header.next = &(obj + 1)->header;
        obj++;
    }

    // End the list
    obj->header.next = NULL;
}

/**
 * Clear each object list, without adding the objects back to the free list.
 */
void clear_object_lists(struct ObjectNode *objLists) {
    s32 i;

    for (i = 0; i < NUM_OBJ_LISTS; i++) {
        objLists[i].next = &objLists[i];
        objLists[i].prev = &objLists[i];
    }
}

/**
 * This function looks broken, but it appears to attempt to delete the leaf
 * graph nodes under obj and obj's siblings.
 */
UNUSED static void unused_delete_leaf_nodes(struct Object *obj) {
    struct Object *children;
    struct Object *sibling;
    struct Object *obj0 = obj;

    if ((children = (struct Object *) obj->header.gfx.node.children) != NULL) {
        unused_delete_leaf_nodes(children);
    } else {
        // No children
        mark_obj_for_deletion(obj);
    }

    // Probably meant to be !=
    while ((sibling = (struct Object *) obj->header.gfx.node.next) == obj0) {
        unused_delete_leaf_nodes(sibling);
        obj = (struct Object *) sibling->header.gfx.node.next;
    }
}

/**
 * Free the given object.
 */
void unload_object(struct Object *obj) {
    obj->activeFlags = ACTIVE_FLAG_DEACTIVATED;
    obj->prevObj = NULL;

    obj->header.gfx.throwMatrix = NULL;
    stop_sounds_from_source(obj->header.gfx.cameraToObject);
    geo_remove_child(&obj->header.gfx.node);
    geo_add_child(&gObjParentGraphNode, &obj->header.gfx.node);

    obj->header.gfx.node.flags &= ~GRAPH_RENDER_BILLBOARD;
    obj->header.gfx.node.flags &= ~GRAPH_RENDER_ACTIVE;

    deallocate_object(&gFreeObjectList, &obj->header);
}

/**
 * If the object is close to being on the floor, move it to be exactly on the floor.
 */
static void snap_object_to_floor(struct Object *obj) {
    struct Surface *surface;

    obj->oFloorHeight = find_floor(obj->oPosX, obj->oPosY, obj->oPosZ, &surface);

    if (obj->oFloorHeight + 2.0f > obj->oPosY && obj->oPosY > obj->oFloorHeight - 10.0f) {
        obj->oPosY = obj->oFloorHeight;
        obj->oMoveFlags |= OBJ_MOVE_ON_GROUND;
    }
}

/**
 * Mark an object to be unloaded at the end of the frame.
 */
void mark_obj_for_deletion(struct Object *obj) {
    //! Same issue as obj_mark_for_deletion
    obj->activeFlags = ACTIVE_FLAG_DEACTIVATED;
}
