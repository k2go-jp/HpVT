#ifndef SCS_6_FEC_ENCODER_H_
#define SCS_6_FEC_ENCODER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/comatibility.h"
#include "scs/1/stdbool.h"
#include "scs/6/fec/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECEncoderEntry FECEncoderEntry;

struct FECEncoderEntry {
	FECData data;
	FECEncoderEntry * next;
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECEncoder {
	FECEncoderEntry * entries;
	size_t capacity;
	struct {
		FECEncoderEntry * head;
		size_t count;
	} queue;
	FECEncoderEntry * head;
	size_t count;

	FECEncodeLevel level;
	FECGroupId gid;
} FECEncoder;

EXTERN void FECEncoderInitialize(FECEncoder * self);
EXTERN void FECEncoderFinalize(FECEncoder * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECEncoderStandBy(FECEncoder * self, size_t capacity);

EXTERN bool FECEncoderAdd(FECEncoder * __restrict self, uint8_t * __restrict ptr, size_t size);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECEncoderEncode(
		FECEncoder * __restrict self,															//
		FECData * __restrict out_data,															//
		size_t * __restrict out_cnt);

/* ---------------------------------------------------------------------------------------------- */

EXTERN size_t FECEncoderGetCountWithParities(FECEncoder * self);

EXTERN bool FECEncoderSetGroupId(FECEncoder * self, FECGroupId value);
EXTERN FECGroupId FECEncoderGetGroupId(FECEncoder * self);

EXTERN bool FECEncoderSetLevel(FECEncoder * self, FECEncodeLevel value);
EXTERN FECEncodeLevel FECEncoderGetLevel(FECEncoder * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECEncoderCleanUp(FECEncoder * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECEncoderPrint(FECEncoder * self);

/* ============================================================================================== */

#endif /* SCS_6_FEC_ENCODER_H_ */
