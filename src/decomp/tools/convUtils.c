#pragma once
#include "convUtils.h"

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include "convUtils.h"
#include "convTypes.h"
#include "../../debug_print.h"

/**
 * This code is based on the only documentation that exists (that I know of) for the SM64 CTL/TBL format.
 * https://github.com/n64decomp/sm64/blob/1372ae1bb7cbedc03df366393188f4f05dcfc422/tools/disassemble_sound.py
 * It is currently not working for reasons unknown. Possible areas to look at are update_CTL_sample_pointers, update_sample_ptr, and general pointers
 * This also requires PATCH macros to be disabled in load.c as this replaces all offsets with pointers
 * 
 * Also worth noting is that currently memory is allocated for some objects more than once instead of re-using pointers to existing structures.
 */

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
        // CTL file, contains instrument and drum data, this is really the only one that needs to be parsed, the rest only needs a header change
        for (int i = 0; i < bankCount; i++){
            struct CTL* ptr = parse_ctl_data(seq+(seqFile->seqArray[i].offset));
            seqFile->seqArray[i].offset = ptr;
        }
    }else if (revision == TYPE_TBL){
        // TBL file, contains raw audio data
        for (int i = 0; i < bankCount; i++){
            seqFile->seqArray[i].offset = seq+(seqFile->seqArray[i].offset);
        }
    }else if (revision == TYPE_SEQ){
        // SEQ file, contains music files (*.m64)
        for (int i = 0; i < bankCount; i++){
            seqFile->seqArray[i].offset = seq+(seqFile->seqArray[i].offset);
        }
    }

    return seqFile;
}

void update_sample_ptr(struct Sample* snd,struct seqFile* tbl,int bank){
    DEBUG_PRINT("have to update sample with offset %i for bank %i",snd->addr,bank);
    snd->addr = tbl->seqArray[bank].offset + snd->addr; // i'm not sure this is correct.
}

void update_CTL_sample_pointers(struct seqFile* ctl,struct seqFile* tbl){ // Update sample pointers to TBL data(?)
    DEBUG_PRINT("Updating CTL sample pointers\n");
    if (ctl->revision != TYPE_CTL){
        DEBUG_PRINT("CTL file is not a CTL file\n");
        return;
    }
    if (tbl->revision != TYPE_TBL){
        DEBUG_PRINT("TBL file is not a TBL file\n");
        return;
    }
    for (int i = 0; i < ctl->seqCount; i++){
        struct CTL* ptr = (struct CTL*)ctl->seqArray[i].offset;
        // find all samples in the CTL file
        for (int j = 0; j < ptr->numInstruments; j++){
            struct Instrument* inst = ptr->instrument_pointers[j];
            if (inst==0x0)
                continue; // null instrument.
            if (inst->sound_hi.sample_addr!=0x0){
                update_sample_ptr(inst->sound_hi.sample_addr,tbl,j);
            }
            if (inst->sound_med.sample_addr!=0x0){
                update_sample_ptr(inst->sound_med.sample_addr,tbl,j);
            }
            if (inst->sound_lo.sample_addr!=0x0){
                update_sample_ptr(inst->sound_lo.sample_addr,tbl,j);
            }
        }
        for (int j = 0; j < ptr->numDrums; j++){
            struct Drum* drum = ptr->drum_pointers[j];
            if (drum==0x0)
                continue; // null drum.
            if (drum->snd.sample_addr!=0x0){
                update_sample_ptr(drum->snd.sample_addr,tbl,j);
            }
        }

    }
}

struct Loop* parse_loop(unsigned char* loop){
    struct Loop* loop_ptr = (struct Loop*)malloc(sizeof(struct Loop));
    loop_ptr->start = read_u32_be(loop);
    loop_ptr->end = read_u32_be(loop + 4);
    loop_ptr->count = read_u32_be(loop + 8); // variable is signed, but the data is being read as unsigned.

    if (loop_ptr->count!=0){
        for (int i = 0;i<16;i++){
            loop_ptr->state[i]=read_u16_be(loop + 16 + i*2);
        }
    }

    loop_ptr->pad = read_u32_be(loop + 12);
    return loop_ptr;
}

struct Book* parse_book(unsigned char* book){
    struct Book* book_ptr = (struct Book*)malloc(sizeof(struct Book));
    book_ptr->order = read_u32_be(book);
    book_ptr->npredictors = read_u32_be(book + 4); // both are signed
    unsigned char* table_data = book+8;
    for (int i = 0; i < 16 * book_ptr->order * book_ptr->npredictors; i++){
        book_ptr->table[i] = read_u16_be(table_data + i*2);
    }
    return book_ptr;
}

struct Sample* parse_sample(unsigned char* sample,unsigned char* ctl){
    struct Sample* samp = malloc(sizeof(struct Sample));
    samp->zero=read_u32_be(sample);
    samp->addr=read_u32_be(sample+4);
    samp->loop=read_u32_be(sample+8);// loop address
    samp->book=read_u32_be(sample+12);// book address
    samp->sample_size=read_u32_be(sample+16);

    samp->loop=parse_loop(ctl+((uintptr_t)samp->loop));
    samp->book=parse_book(ctl+((uintptr_t)samp->book));
    return samp;
}

struct Sound* parse_sound(unsigned char* sound,unsigned char* ctl){
    struct Sound* snd = malloc(sizeof(struct Sound));
    snd->sample_addr=read_u32_be(sound);
    snd->tuning = (float)read_f32_be(sound+4);
    // if sample_addr is 0 then the sound is null
    if (snd->sample_addr!=0){
        snd->sample_addr = parse_sample(ctl+((uintptr_t)snd->sample_addr),ctl);
    }
    return snd;
}

struct Drum* parse_drum(unsigned char* drum,unsigned char* ctl){ /* Read Drum data */
    struct Drum* drumData = malloc(sizeof(struct Drum));
    drumData->release_rate = drum[0];
    drumData->pan = drum[1];
    drumData->loaded = drum[2];
    drumData->pad = drum[3];
    drumData->snd=*parse_sound(drum+4,ctl);
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

struct Instrument* parse_instrument(unsigned char* instrument,unsigned char* ctl){
    struct Instrument* inst = malloc(sizeof(struct Instrument));
    inst->loaded = instrument[0];
    inst->normal_range_lo = instrument[1];
    inst->normal_range_hi = instrument[2];
    inst->release_rate = instrument[3];
    inst->env_addr=read_u32_be(instrument+4);
    inst->sound_lo=*parse_sound(instrument+8,ctl);
    inst->sound_med=*parse_sound(instrument+16,ctl);
    inst->sound_hi=*parse_sound(instrument+24,ctl);
    return inst;
}

struct TBL* parse_tbl_data(unsigned char* tbl){
    struct TBL* tblData = malloc(sizeof(struct TBL));
    tblData->data = tbl;
    return tblData;
}

struct SEQ* parse_seq_data(unsigned char* seq){
    struct SEQ* seqData = malloc(sizeof(struct SEQ));
    seqData->data = seq;
    return seqData;
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
    ctl->drum_pointers= (struct Drum**)malloc(sizeof(struct Drum*)*ctl->numDrums);
    int drumTablePtr = read_u32_be(ctlData + 16);
    for (int i = 0; i < ctl->numDrums; i++){
        uint32_t data = read_u32_be(ctlData + drumTablePtr+16 + i * 4);
        
        struct Drum* d = parse_drum(ctlData+data+16,ctlData+16);
        d->env_addr=parse_envelope(ctlData+((uintptr_t)d->env_addr)+16);
        ctl->drum_pointers[i] = d;
    }
    // parse instrument data
    ctl->instrument_pointers = (struct Instrument**)malloc(sizeof(struct Instrument*)*ctl->numInstruments);
    int instTablePtr = 16+4;
    for (int i = 0; i < ctl->numInstruments; i++){
        uint32_t data = read_u32_be(ctlData + instTablePtr + i * 4);
        if (data == 0)
            continue;
        struct Instrument* inst = parse_instrument(ctlData+data+16,ctlData+16);
        inst->env_addr=parse_envelope(ctlData+((uintptr_t)inst->env_addr)+16);
        ctl->instrument_pointers[i] = inst;
    }
    // 

    return ctl;
}
