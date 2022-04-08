#pragma once
#include "../pc/libaudio_internal.h"

struct seqFile* parse_seqfile(unsigned char* seq);
struct CTL* parse_ctl_data(unsigned char* ctlData);