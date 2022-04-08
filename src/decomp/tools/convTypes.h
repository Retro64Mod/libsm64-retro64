#pragma once

#define TYPE_CTL 1
#define TYPE_TBL 2

struct Loop
{
    unsigned int start;
    unsigned int end;
    int count;
    unsigned int pad;
    short state[16];
};


struct Book
{
    int order; // must be 2
    int npredictors; // must be 2
    short table[64]; // 16 * order * npredictors
};

struct Sound{
    unsigned int sample_addr;
    float tuning;
};

struct Sample{
    unsigned int zero;
    unsigned int addr;
    struct Loop* loop; // must not be null
    struct Book* book; // must not be null
    unsigned int sample_size;
};

struct delay_arg{
    unsigned short delay;
    unsigned short arg;
};

struct Envelope{
    struct delay_arg delay_args[6]; // array of [(delay,arg)]
};

struct Drum{
    unsigned char release_rate;
    unsigned char pan;
    unsigned char loaded;
    unsigned char pad;
    struct Sound snd;
    struct Envelope* env_addr;
};

struct Instrument{
    unsigned char loaded;
    unsigned char normal_range_lo;
    unsigned char normal_range_hi;
    unsigned char release_rate;
    struct Envelope* env_addr;
    struct Sound sound_lo;
    struct Sound sound_med;
    struct Sound sound_hi;
};

struct CTL
{
    unsigned int numInstruments;
    unsigned int numDrums;
    unsigned int shared;
    unsigned int iso_date;
    int* drum_pointers;
    int* instrument_pointers;
};

struct seqObject{
    unsigned int offset;
    unsigned int len;
};

struct seqFile{
    unsigned int revision;
    unsigned int seqCount;
    struct seqObject seqArray[1];
};