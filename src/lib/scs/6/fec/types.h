#ifndef SCS_6_FEC_TYPES_H_
#define SCS_6_FEC_TYPES_H_ 1

/* ============================================================================================== */

#include <stdint.h>

#include "scs/1/stdbool.h"

/* ---------------------------------------------------------------------------------------------- */

typedef uint16_t FECGroupId;

#define FEC_GROUPID_INVVAL		UINT16_MAX
#define FEC_GROUPID_MINVAL		0
#define FEC_GROUPID_MAXVAL		(UINT16_MAX - 1)

#define FECGroupIdValidate(xxx_value) \
		(xxx_value != FEC_GROUPID_INVVAL ? true : false)

/* ---------------------------------------------------------------------------------------------- */

typedef uint16_t FECSetId;

#define FEC_SETID_INVVAL		UINT16_MAX
#define FEC_SETID_MINVAL		0
#define FEC_SETID_MAXVAL		(UINT16_MAX - 1)

#define FECSetIdValidate(xxx_value) \
		(xxx_value != FEC_SETID_INVVAL ? true : false)

/* ---------------------------------------------------------------------------------------------- */

typedef uint8_t FECFlags;

#define FEC_FLAGS_NONE		0x00	// 0000 0000
#define FEC_FLAGS_PARITY	0x01	// 0000 0001

extern bool FECFlagsValidate(FECFlags value);

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECHeader {
	uint8_t code;
	FECFlags flags;
	FECGroupId gid;
	FECSetId sid;
	size_t parity_len;
	int src_cnt;
} FECHeader;

#define FEC_HEADER_CODE		0x55

/* ---------------------------------------------------------------------------------------------- */

typedef struct FECData {
	uint8_t * ptr;
	size_t size;
} FECData;

/* ---------------------------------------------------------------------------------------------- */

typedef int FECEncodeLevel;

#define FEC_ENCODELEVEL_MIN		2
#define FEC_ENCODELEVEL_MAX		10

#define FECEncodeLevelValidate(xxx_value) \
		((FEC_ENCODELEVEL_MIN <= xxx_value && xxx_value <= FEC_ENCODELEVEL_MAX) ? true : false)


/* ---------------------------------------------------------------------------------------------- */

typedef struct FECDecodeResult {
	int parity_cnt;
	int recovery_success_cnt;
	int recovery_failure_cnt;
} FECDecodeResult;

/* ============================================================================================== */

#endif /* SCS_6_FEC_TYPES_H_ */
