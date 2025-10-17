#ifndef SCS_6_FEC_DECODER_H_
#define SCS_6_FEC_DECODER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/comatibility.h"
#include "scs/1/stdbool.h"
#include "scs/6/fec/types.h"

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECDecoderEntry FECDecoderEntry;

struct FECDecoderEntry {
	FECHeader hdr;
	FECData data;

	struct {
		FECDecoderEntry * next;
		size_t count;
	} child;

	FECDecoderEntry * next;
};

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECDecoder {
	FECDecoderEntry * entries;
	size_t capacity;
	struct {
		FECDecoderEntry * head;
		size_t count;
	} queue;
	FECDecoderEntry * head;
	size_t count;
	FECGroupId gid;

	struct {
		size_t parity;
		size_t recovery_success;
		size_t recovery_failure;
	} statistics;

} FECDecoder;

EXTERN void FECDecoderInitialize(FECDecoder * self);
EXTERN void FECDecoderFinalize(FECDecoder * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECDecoderStandBy(FECDecoder * self, size_t capacity);

EXTERN bool FECDecoderAdd(FECDecoder * __restrict self, uint8_t * __restrict ptr, size_t size);

EXTERN bool FECDecoderDecode(																	//
		FECDecoder * __restrict self, 															//
		FECData * __restrict out_data, 															//
		size_t * __restrict out_cnt);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECDecoderCleanUp(FECDecoder * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void FECDecoderPrint(FECDecoder * self);

/* ============================================================================================== */

#endif /* SCS_6_FEC_DECODER_H_ */
