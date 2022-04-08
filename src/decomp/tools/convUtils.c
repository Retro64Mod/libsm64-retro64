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
        parse_ctl_data(seqFile->seqArray[i].offset);
    }
    // test

    return seqFile;
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
    for (int i = 0; i < ctl->numDrums; i++){
        ctl->drum_pointers[i] = read_u32_be(ctlData + 16 + i * 4);
    }
    ctl->instrument_pointers = (void*)malloc(ctl->numInstruments * sizeof(void*));
    for (int i = 0; i < ctl->numInstruments; i++){
        ctl->instrument_pointers[i] = ctlData + 16 + ctl->numDrums * 4 + i * 4;
    }

    return ctl;
}