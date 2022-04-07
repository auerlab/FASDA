#define MAX_CONDITIONS  128
#define MAX_SEQ_LEN     1024
#define GFF_MASK        BL_GFF_FIELD_SEQID|BL_GFF_FIELD_TYPE|\
			BL_GFF_FIELD_START|BL_GFF_FIELD_END|\
			BL_GFF_FIELD_ATTRIBUTES
#define SAM_MASK        BL_SAM_FIELD_RNAME|BL_SAM_FIELD_POS
#define SAMTOOLS_ARGS   "-@ 2 --exclude-flags UNMAP"

#include "diffanal-protos.h"

