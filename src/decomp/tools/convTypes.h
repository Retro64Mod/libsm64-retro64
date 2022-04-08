#pragma once

#define TYPE_CTL 1
#define TYPE_TBL 2


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