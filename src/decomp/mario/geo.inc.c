
#include "../include/sm64.h"
#include "../include/types.h"
#include "../include/geo_commands.h"
#include "../game/rendering_graph_node.h"
#include "../shim.h"
#include "../game/object_stuff.h"
#include "../game/behavior_actions.h"
#include "../tools/geolayout_parser.h"
#include "../tools/convUtils.h"
#include "../tools/utils.h"

#define SHADOW_CIRCLE_PLAYER 99

GeoLayout* mario_geo_ptr=0x0;

void initMarioGeo(unsigned char* rom){
   if (mario_geo_ptr != 0x0){
      return;
   }
   unsigned char* mario_data=load_MI0_data_from_rom(rom,0x114750);
   uintptr_t ptr = convertPtr_follow(rom,0x17002CE0,mario_data); // 17002CE0 -> mario_geo_load_body
   uintptr_t data[]={
      GEO_SHADOW(SHADOW_CIRCLE_PLAYER, 0xB4, 100),
      GEO_OPEN_NODE(),
         GEO_ZBUFFER(1),
         GEO_OPEN_NODE(),
            GEO_SCALE(0x00, 16384),
            GEO_OPEN_NODE(),
               GEO_ASM(0, geo_mirror_mario_backface_culling),
               GEO_ASM(0, geo_mirror_mario_set_alpha),
               GEO_BRANCH(1, ptr), 
               GEO_ASM(1, geo_mirror_mario_backface_culling),
            GEO_CLOSE_NODE(),
         GEO_CLOSE_NODE(),
      GEO_CLOSE_NODE(),
      GEO_END(),
   };
   // transfer memory allocated in stack to actual memory
   mario_geo_ptr=malloc(sizeof(GeoLayout)*20);
   memcpy(mario_geo_ptr,data,sizeof(GeoLayout)*20);
}