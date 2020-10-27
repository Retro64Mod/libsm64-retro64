#include "load_surfaces.h"

#include <stdlib.h>
#include <string.h>

#include "decomp/include/types.h"
#include "decomp/include/surface_terrains.h"
#include "decomp/engine/math_util.h"
#include "decomp/shim.h"

struct LoadedSurfaceObject
{
    struct SurfaceObjectTransform transform;
    uint32_t surfaceCount;
    struct SM64Surface *surfaces;
};

static uint32_t s_static_surface_count = 0;
static struct Surface *s_static_surface_list = NULL;

static uint32_t s_surface_object_count = 0;
static struct LoadedSurfaceObject *s_surface_object_list = NULL;

static uint32_t s_dynamic_surface_count = 0;
static struct Surface *s_dynamic_surface_list = NULL;

#define CONVERT_ANGLE( x ) ((s16)( -(x) / 180.0f * 32768.0f ))

static void init_transform( struct SurfaceObjectTransform *out, const struct SM64ObjectTransform *in )
{
    out->aVelX = 0.0f;
    out->aVelY = 0.0f;
    out->aVelZ = 0.0f;
    out->aPosX = in->position[0];
    out->aPosY = in->position[1];
    out->aPosZ = in->position[2];

    out->aAngleVelPitch = 0.0f;
    out->aAngleVelYaw   = 0.0f;
    out->aAngleVelRoll  = 0.0f;
    out->aFaceAnglePitch = CONVERT_ANGLE(in->eulerRotation[0]);
    out->aFaceAngleYaw   = CONVERT_ANGLE(in->eulerRotation[1]);
    out->aFaceAngleRoll  = CONVERT_ANGLE(in->eulerRotation[2]);
}

static void update_transform( struct SurfaceObjectTransform *out, const struct SM64ObjectTransform *in )
{
    out->aVelX = in->position[0] - out->aPosX;
    out->aVelY = in->position[1] - out->aPosY;
    out->aVelZ = in->position[2] - out->aPosZ;
    out->aPosX = in->position[0];
    out->aPosY = in->position[1];
    out->aPosZ = in->position[2];

    s16 inX = CONVERT_ANGLE(in->eulerRotation[0]);
    s16 inY = CONVERT_ANGLE(in->eulerRotation[1]);
    s16 inZ = CONVERT_ANGLE(in->eulerRotation[2]);

    out->aAngleVelPitch = inX - out->aFaceAnglePitch;
    out->aAngleVelYaw   = inY - out->aFaceAngleYaw;
    out->aAngleVelRoll  = inZ - out->aFaceAngleRoll;
    out->aFaceAnglePitch = inX;
    out->aFaceAngleYaw   = inY;
    out->aFaceAngleRoll  = inZ;
}

/**
 * Returns whether a surface has exertion/moves Mario
 * based on the surface type.
 */
static s32 surface_has_force(s16 surfaceType) {
    s32 hasForce = FALSE;

    switch (surfaceType) {
        case SURFACE_0004: // Unused
        case SURFACE_FLOWING_WATER:
        case SURFACE_DEEP_MOVING_QUICKSAND:
        case SURFACE_SHALLOW_MOVING_QUICKSAND:
        case SURFACE_MOVING_QUICKSAND:
        case SURFACE_HORIZONTAL_WIND:
        case SURFACE_INSTANT_MOVING_QUICKSAND:
            hasForce = TRUE;
            break;

        default:
            break;
    }
    return hasForce;
}

static void engine_surface_from_lib_surface( struct Surface *surface, const struct SM64Surface *libSurf, const struct SurfaceObjectTransform *transform )
{
    int16_t type = libSurf->type;
    int16_t force = libSurf->force;
    s32 x1 = libSurf->vertices[0][0];
    s32 y1 = libSurf->vertices[0][1];
    s32 z1 = libSurf->vertices[0][2];
    s32 x2 = libSurf->vertices[1][0];
    s32 y2 = libSurf->vertices[1][1];
    s32 z2 = libSurf->vertices[1][2];
    s32 x3 = libSurf->vertices[2][0];
    s32 y3 = libSurf->vertices[2][1];
    s32 z3 = libSurf->vertices[2][2];

    s32 maxY, minY;
    f32 nx, ny, nz;
    f32 mag;

    if( transform != NULL )
    {
        Mat4 m;
        Vec3s rotation = { transform->aFaceAnglePitch, transform->aFaceAngleYaw, transform->aFaceAngleRoll };
        Vec3f position = { transform->aPosX, transform->aPosY, transform->aPosZ };
        mtxf_rotate_zxy_and_translate(m, position, rotation);

        Vec3f v1 = { x1, y1, z1 };
        Vec3f v2 = { x2, y2, z2 };
        Vec3f v3 = { x3, y3, z3 };

        mtxf_mul_vec3f( m, v1 );
        mtxf_mul_vec3f( m, v2 );
        mtxf_mul_vec3f( m, v3 );

        x1 = v1[0]; y1 = v1[1]; z1 = v1[2];
        x2 = v2[0]; y2 = v2[1]; z2 = v2[2];
        x3 = v3[0]; y3 = v3[1]; z3 = v3[2];

        surface->object = (struct Object *)(transform);
    }
    else
    {
        surface->object = NULL;
    }

    // (v2 - v1) x (v3 - v2)
    nx = (y2 - y1) * (z3 - z2) - (z2 - z1) * (y3 - y2);
    ny = (z2 - z1) * (x3 - x2) - (x2 - x1) * (z3 - z2);
    nz = (x2 - x1) * (y3 - y2) - (y2 - y1) * (x3 - x2);
    mag = sqrtf(nx * nx + ny * ny + nz * nz);

    // Could have used min_3 and max_3 for this...
    minY = y1;
    if (y2 < minY) {
        minY = y2;
    }
    if (y3 < minY) {
        minY = y3;
    }

    maxY = y1;
    if (y2 > maxY) {
        maxY = y2;
    }
    if (y3 > maxY) {
        maxY = y3;
    }

    if (mag < 0.0001)
        DEBUG_LOG("ERROR: normal magnitude is very close to zero");

    mag = (f32)(1.0 / mag);
    nx *= mag;
    ny *= mag;
    nz *= mag;

    surface->vertex1[0] = x1;
    surface->vertex2[0] = x2;
    surface->vertex3[0] = x3;

    surface->vertex1[1] = y1;
    surface->vertex2[1] = y2;
    surface->vertex3[1] = y3;

    surface->vertex1[2] = z1;
    surface->vertex2[2] = z2;
    surface->vertex3[2] = z3;

    surface->normal.x = nx;
    surface->normal.y = ny;
    surface->normal.z = nz;

    surface->originOffset = -(nx * x1 + ny * y1 + nz * z1);

    surface->lowerY = minY - 5;
    surface->upperY = maxY + 5;

    s16 hasForce = surface_has_force(type);
    s16 flags = 0; // surf_has_no_cam_collision(type);

    surface->room = 0;
    surface->type = type;
    surface->flags = (s8) flags;

    if (hasForce) {
        surface->force = force;
    } else {
        surface->force = 0;
    }
}

void update_dynamic_surface_list( void )
{
    free( s_dynamic_surface_list );
    s_dynamic_surface_count = 0;
    s_dynamic_surface_list = NULL;

    for( int i = 0; i < s_surface_object_count; ++i )
    {
        for( int j = 0; j < s_surface_object_list[i].surfaceCount; ++j )
        {
            s_dynamic_surface_count++;
            s_dynamic_surface_list = realloc( s_dynamic_surface_list, s_dynamic_surface_count * sizeof( struct Surface ));
            engine_surface_from_lib_surface( &s_dynamic_surface_list[s_dynamic_surface_count - 1], &s_surface_object_list[i].surfaces[j], &s_surface_object_list[i].transform );
        }
    }
}


struct Surface *loaded_surface_get_at_index( uint32_t index )
{
    if( index < s_static_surface_count )
        return &s_static_surface_list[ index ];
    
    return &s_dynamic_surface_list[ index - s_static_surface_count ];
}

uint32_t loaded_surface_get_count()
{
    return s_static_surface_count + s_dynamic_surface_count;
}

void surfaces_load_static_libsm64( const struct SM64Surface *surfaceArray, uint32_t numSurfaces )
{
    s_static_surface_count = numSurfaces;
    s_static_surface_list = malloc( sizeof( struct Surface ) * numSurfaces );

    for( int i = 0; i < numSurfaces; ++i )
        engine_surface_from_lib_surface( &s_static_surface_list[i], &surfaceArray[i], NULL );
}

uint32_t surfaces_load_object( const struct SM64SurfaceObject *surfaceObject )
{
    uint32_t idx = s_surface_object_count;
    s_surface_object_count++;
    s_surface_object_list = realloc( s_surface_object_list, s_surface_object_count * sizeof( struct LoadedSurfaceObject ));

    struct LoadedSurfaceObject *obj = &s_surface_object_list[idx];
    init_transform( &obj->transform, &surfaceObject->transform );
    obj->surfaceCount = surfaceObject->surfaceCount;
    obj->surfaces = malloc( obj->surfaceCount * sizeof( struct SM64Surface ));
    memcpy( obj->surfaces, surfaceObject->surfaces, obj->surfaceCount * sizeof( struct SM64Surface ));

    return idx;
}

void surfaces_unload_object( uint32_t objId )
{
    // TODO implement
}

void surface_object_update_transform( uint32_t objId, const struct SM64ObjectTransform *newTransform )
{
    update_transform( &s_surface_object_list[objId].transform, newTransform );
}

void surfaces_unload_all( void )
{
    free( s_static_surface_list );
    s_static_surface_count = 0;
    s_static_surface_list = NULL;

    for( int i = 0; i < s_surface_object_count; ++i )
        free( s_surface_object_list[i].surfaces );
    free( s_surface_object_list );
    s_surface_object_count = 0;
    s_surface_object_list = NULL;

    free( s_dynamic_surface_list );
    s_dynamic_surface_count = 0;
    s_dynamic_surface_list = NULL;
}
