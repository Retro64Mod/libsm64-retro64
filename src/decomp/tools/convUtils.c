#pragma once
#include "convUtils.h"

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "convUtils.h"
#include "convTypes.h"
#include "../../debug_print.h"
struct seqFile* parse_seqfile(unsigned char* seq){ /* Read SeqFile data */
    short revision = read_u16_be(seq);
    short bankCount = read_u16_be(seq + 2);
    struct seqFile* seqFile = (struct seqFile*)malloc(sizeof(struct seqFile) + (bankCount - 1) * sizeof(struct seqObject));
    seqFile->revision = revision;
    seqFile->seqCount = bankCount;
    for (int i = 0; i < bankCount; i++){ // read bank offsets and sizes
        unsigned char* ptr = seq + 4 + i * 8;
        seqFile->seqArray[i].offset = (intptr_t)read_u32_be(ptr);
        seqFile->seqArray[i].len = read_u32_be((seq + 4 + i * 8 + 4));
    }

    if (revision == TYPE_CTL){
        for (int i = 0; i < bankCount; i++){
            struct CTL* ptr = parse_ctl_data(seq+(seqFile->seqArray[i].offset));
            seqFile->seqArray[i].offset = ptr;
        }
    }else if (revision == TYPE_TBL){
        for (int i = 0; i < bankCount; i++){
            struct TBL* ptr = parse_tbl_data(seq+(seqFile->seqArray[i].offset));
            seqFile->seqArray[i].offset = ptr;
        }
    }else if (revision == TYPE_SEQ){
        for (int i = 0; i < bankCount; i++){
            struct seqObject* ptr = parse_seq_data(seq+(seqFile->seqArray[i].offset));
            seqFile->seqArray[i].offset = ptr;
        }
    }

    return seqFile;
}

struct Sound* parse_sound(unsigned char* sound){
    struct Sound* snd = malloc(sizeof(struct Sound));
    snd->sample_addr=read_u32_be(sound);
    snd->tuning = (float)read_f32_be(sound+4);
    // if sample_addr is 0 then the sound is null
    return snd;
}

struct Drum* parse_drum(unsigned char* drum){ /* Read Drum data */
    struct Drum* drumData = malloc(sizeof(struct Drum));
    drumData->release_rate = drum[0];
    drumData->pan = drum[1];
    drumData->loaded = drum[2];
    drumData->pad = drum[3];
    drumData->snd=*parse_sound(drum+4);
    drumData->env_addr=read_u32_be(drum+sizeof(struct Sound)+4);
    return drumData;
}

struct Envelope* parse_envelope(unsigned char* env){
    struct Envelope* envData = malloc(sizeof(struct Envelope));
    for (int i = 0; i < 6; i++){
        envData->delay_args[i].delay = read_u16_be(env + i * 4);
        envData->delay_args[i].arg = read_u16_be(env + i * 4 + 2);
        if (1 <= (-envData->delay_args[i].delay) % 2 * 16 <= 3)
            break;
    }
    return envData;
}

struct Instrument* parse_instrument(unsigned char* instrument){
    struct Instrument* inst = malloc(sizeof(struct Instrument));
    inst->loaded = instrument[0];
    inst->normal_range_lo = instrument[1];
    inst->normal_range_hi = instrument[2];
    inst->release_rate = instrument[3];
    inst->env_addr=read_u32_be(instrument+4);
    inst->sound_lo=*parse_sound(instrument+8);
    inst->sound_med=*parse_sound(instrument+16);
    inst->sound_hi=*parse_sound(instrument+24);
    return inst;
}

struct TBL* parse_tbl_data(unsigned char* tbl){

}

struct SEQ* parse_seq_data(unsigned char* seq){

}

struct CTL* parse_ctl_data(unsigned char* ctlData){
    
    struct CTL* ctl = (struct CTL*)malloc(sizeof(struct CTL));
    #pragma region Parse CTL header
    ctl->numInstruments = read_u32_be(ctlData);
    ctl->numDrums = read_u32_be(ctlData + 4);
    ctl->shared = read_u32_be(ctlData + 8);
    ctl->iso_date = read_u32_be(ctlData + 12);
    #pragma endregion
    // header parsed, now read data
    ctl->drum_pointers= (int*)malloc(ctl->numDrums * sizeof(int));
    int drumTablePtr = read_u32_be(ctlData + 16);
    for (int i = 0; i < ctl->numDrums; i++){
        ctl->drum_pointers[i] = read_u32_be(ctlData + drumTablePtr+16 + i * 4);
        struct Drum* d = parse_drum(ctlData+ctl->drum_pointers[i]+16);
        d->env_addr=parse_envelope(ctlData+((int)d->env_addr)+16);
        ctl->drum_pointers[i] = (int)d;
    }
    // parse instrument data
    ctl->instrument_pointers = (int*)malloc(ctl->numInstruments * sizeof(int));
    int instTablePtr = 16+4;
    for (int i = 0; i < ctl->numInstruments; i++){
        ctl->instrument_pointers[i] = read_u32_be(ctlData + instTablePtr + i * 4);
        if (ctl->instrument_pointers[i] == 0)
            continue;
        struct Instrument* inst = parse_instrument(ctlData+ctl->instrument_pointers[i]+16);
        inst->env_addr=parse_envelope(ctlData+((int)inst->env_addr)+16);
        ctl->instrument_pointers[i] = (int)inst;
    }
    // 

    return ctl;
}