#pragma once

#define TYPE_CTL 1
#define TYPE_TBL 2

struct Sound{
    unsigned int sample_addr;
    float tuning;
};

struct Drum{
    unsigned char release_rate;
    unsigned char pan;
    unsigned char loaded;
    unsigned char pad;
    struct Sound snd;
    unsigned int env_addr;
};

struct Instrument{
    unsigned char loaded;
    unsigned char normal_range_lo;
    unsigned char normal_range_hi;
    unsigned char release_rate;
    unsigned int env_addr;
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