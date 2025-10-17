#ifndef SCS_6_FEC_H_
#define SCS_6_FEC_H_ 1

/* ============================================================================================== */

#include <stdint.h>

#include "scs/1/comatibility.h"
#include "scs/1/stdbool.h"
#include "scs/6/fec/defines.h"
#include "scs/6/fec/types.h"

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECEncode(FECGroupId id, FECData * __restrict data_ptrs, size_t data_cnt, FECData * __restrict out_ptrs, size_t * __restrict out_cnt);

EXTERN bool FECDecode(FECData * __restrict data_ptrs, size_t data_cnt, FECData * __restrict out_ptrs, size_t * __restrict out_cnt, FECGroupId * out_id, FECDecodeResult * out_result);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECWrapHeader(FECGroupId id, FECData * __const __restrict data, FECData * __restrict out);

EXTERN bool FECStripHeader(FECData * __const __restrict data, FECHeader * __restrict out_hdr, FECData * __restrict out_data);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECGetHeader(uint8_t * const __restrict ptr, size_t size, FECHeader * __restrict out);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool FECIsFEC(uint8_t * const __restrict ptr, size_t size, FECGroupId * __restrict out);

EXTERN bool FECIsParity(FECHeader * hdr);

/* ============================================================================================== */

#endif /* FEC_H_ */
