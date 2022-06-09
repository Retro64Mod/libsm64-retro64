#include "../../debug_print.h"
#include "anim_parser.h"
#include "utils.h"
#include <stdarg.h>
#include "convUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Animation** parse_anim_table(unsigned char* MI0_data,unsigned int table_offset,int s_num_entries){
    // todo: take in Animation** directly instead of returning it
    table_offset = table_offset & 0x00FFFFFF;
    #define GET_OFFSET( n ) (read_u32_be(MI0_data + table_offset+(n*4))&0x00FFFFFF)

    uint8_t *read_ptr = MI0_data + table_offset;

    struct Animation** anims = malloc(s_num_entries*sizeof(struct Animation*));

    for( int i = 0; i < s_num_entries; ++i )
    {
        
        read_ptr = MI0_data+GET_OFFSET(i);
        if (read_ptr==MI0_data){ // offset is 0x0
            anims[i]=0x0;
            continue;
        }
        anims[i]=(struct Animation*)malloc(sizeof(struct Animation));
        
        anims[i]->flags             = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i]->animYTransDivisor = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i]->startFrame        = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i]->loopStart         = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i]->loopEnd           = read_s16_be( read_ptr ); read_ptr += 2;
        anims[i]->unusedBoneCount   = read_s16_be( read_ptr ); read_ptr += 2;
        uint32_t values_offset = read_u32_be( read_ptr )&0x00FFFFFF; read_ptr += 4;
        uint32_t index_offset  = read_u32_be( read_ptr )&0x00FFFFFF; read_ptr += 8;
        uint32_t end_offset    = read_u32_be( read_ptr )&0x00FFFFFF;

        read_ptr            = MI0_data  + index_offset;
        uint8_t *values_ptr = MI0_data  + values_offset;
        uint8_t *end_ptr    = MI0_data + end_offset;

        anims[i]->index = malloc( end_offset - index_offset );
        anims[i]->values = malloc( index_offset - values_offset );
        int values = (index_offset - values_offset)/2;
        int indexes=(end_offset - index_offset)/2;
        int j = 0;
        while( j<indexes )
        {
            anims[i]->index[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }

        j = 0;
        read_ptr=values_ptr;
        while( j<values )
        {
            anims[i]->values[j++] = read_u16_be( read_ptr );
            read_ptr += 2;
        }
    }

    #undef GET_OFFSET

    return anims;
}