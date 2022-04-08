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
    struct seqFile* seqFile = (struct seqFile*)malloc(sizeof(seqFile) + (bankCount - 1) * sizeof(struct seqObject));
    seqFile->revision = revision;
    seqFile->seqCount = bankCount;
    for (int i = 0; i < bankCount; i++){ // read bank offsets and sizes
        unsigned char* ptr = seq + 4 + i * 8;
        seqFile->seqArray[i].offset = read_u32_be(ptr);
        seqFile->seqArray[i].len = read_u32_be((seq + 4 + i * 8 + 4));
    }

    // test
    for (int i = 0; i < bankCount; i++){
        parse_ctl_data(seq+(seqFile->seqArray[i].offset));
    }
    // test

    return seqFile;
}

struct Sound parse_sound(unsigned char* sound){
    struct Sound* snd = (struct Sound*)sound;
    snd->sample_addr=read_u32_be(sound);
    snd->tuning = (float)read_f32_be(sound+4);
    // if sample_addr is 0 then the sound is null
}

struct Drum parse_drum(unsigned char* drum){ /* Read Drum data */
    struct Drum* drumData = (struct Drum*)drum;
    // first 4 bytes don't need reading
    parse_sound(&drumData->snd);
    drumData->env_addr=read_u32_be(drum+sizeof(struct Sound));
    
}

struct Instrument parse_instrument(unsigned char* instrument){
    struct Instrument* inst = (struct Instrument*)instrument;
    // first 4 bytes don't need reading
    inst->env_addr=read_u32_be(instrument+4);
    parse_sound(&inst->sound_lo);
    parse_sound(&inst->sound_med);
    parse_sound(&inst->sound_hi);
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
        struct Drum d = parse_drum(ctlData+ctl->drum_pointers[i]+16);
    }
    // parse instrument data
    ctl->instrument_pointers = (int*)malloc(ctl->numInstruments * sizeof(int));
    int instTablePtr = 16+4;
    for (int i = 0; i < ctl->numInstruments; i++){
        ctl->instrument_pointers[i] = read_u32_be(ctlData + instTablePtr + i * 4);
        if (ctl->instrument_pointers[i] == 0)
            continue;
        struct Instrument inst = parse_instrument(ctlData+ctl->instrument_pointers[i]+16);
    }
    // 

    return ctl;
}