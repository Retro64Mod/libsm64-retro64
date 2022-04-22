#include "audioStructPrint.h"
#include "convTypes.h"
#include <stdio.h>

void printTabs(int tabs){
    for (int i = 0; i < tabs; i++){
        printf("\t");
    }
}

void printInst(ALInstrument* inst,int tabs){
    printTabs(tabs);
    printf("Instrument: volume %i, priority %i, flags %i, tremType %i, tremRate %i, tremDepth %i, tremDelay %i, vibType %i, vibRate %i, vibDepth %i, vibDelay %i, bendRange %i, soundCount %i\n",
        inst->volume,
        inst->priority,
        inst->flags,
        inst->tremType,
        inst->tremRate,
        inst->tremDepth,
        inst->tremDelay,
        inst->vibType,
        inst->vibRate,
        inst->vibDepth,
        inst->vibDelay,
        inst->bendRange,
        inst->soundCount);
}

void printCTL(ALBank* bank,int tabs){
    printTabs(tabs);
    printf("CTL: inst count: %i, samplerate %i, flags %i\n",bank->instCount,bank->sampleRate,bank->flags);
    for (int i = 0; i < bank->instCount; i++){
        printInst(bank->instArray[i],tabs+1);
    }
}

void printBank(ALBankFile* bank){
    printf("Bank revision %i, count %i\n",bank->revision,bank->bankCount);
    if (bank->revision==TYPE_CTL){
        printCTL(bank,1);
    }
}

