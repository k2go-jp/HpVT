#define SCS_SOURCECODE_FILEID	"6FECFEC"

/* ============================================================================================== */

#include <stdlib.h>
#include <string.h>

#include "scs/1/in.h"
#include "scs/1/stdbool.h"
#include "scs/6/fec/fec.h"
#include "scs/6/fec/types.h"

/* ---------------------------------------------------------------------------------------------- */

#define FEC_PARITY_MASK8	255
#define FEC_PARITY_MASK16	65535
#define FEC_PARITY_MASK24	16777215

#if __WORDSIZE == 64
typedef uint64_t FECUnit;
#define FEC_PARITY_UNIT 	sizeof(uint64_t)
#define FEC_PARITY_MASK32	(4294967295U)
#define FEC_PARITY_MASK40	(1099511627775UL)
#define FEC_PARITY_MASK48	(281474976710655UL)
#define FEC_PARITY_MASK56	(72057594037927935UL)
#else
typedef uint32_t FECUnit;
#define FEC_PARITY_UNIT 	sizeof(uint32_t)
#define FEC_PARITY_MASK32	0
#define FEC_PARITY_MASK40	0
#define FEC_PARITY_MASK48	0
#define FEC_PARITY_MASK56	0
#endif

/* ---------------------------------------------------------------------------------------------- */

#define _FEC_SET(xxx_ptr, xxx_length, xxx_offset, xxx_size, xxx_type, xxx_value, xxx_retval) \
		if (xxx_size <= xxx_length) {\
			*((xxx_type *) xxx_ptr) = (xxx_type) xxx_value;\
			xxx_ptr += xxx_size;\
			xxx_length -= xxx_size;\
			xxx_offset += xxx_size;\
		}\
		else {\
			return xxx_retval;\
		}

#define _FEC_GET(xxx_ptr, xxx_length, xxx_offset, xxx_size, xxx_type, xxx_value, xxx_retval) \
		if (xxx_size <= xxx_length) {\
			if (xxx_ptr != NULL) {\
				xxx_value = (xxx_type) *((xxx_type *) xxx_ptr);\
				xxx_ptr += xxx_size;\
			}\
			xxx_length -= xxx_size;\
			xxx_offset += xxx_size;\
		}\
		else {\
			return xxx_retval;\
		}

/* ---------------------------------------------------------------------------------------------- */

static FECSetId _FECGenerateSetId(void) {
	static uint16_t tmp_sn = 0;

	FECSetId tmp_retval;

	tmp_retval = ++tmp_sn;

	if (FEC_SETID_MAXVAL <= tmp_sn) {
		tmp_sn = FEC_SETID_MINVAL;
	}

	return tmp_retval;
}

static bool _FECIsSet(FECHeader * __restrict hdr1, FECHeader * __restrict hdr2) {

	if (hdr1->gid != hdr2->gid) {
		return false;
	}

	if (hdr1->sid != hdr2->sid) {
		return false;
	}

	if (hdr1->parity_len != hdr2->parity_len) {
		return false;
	}

	if (hdr1->src_cnt != hdr2->src_cnt) {
		return false;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static bool _FECBuildPacket(FECHeader * __const __restrict hdr, FECData * __const __restrict data, FECData * __restrict out) {
	size_t tmp_size;
	uint8_t * tmp_data;
	uint8_t * tmp_ptr;
	size_t tmp_length;
	size_t tmp_offset;

	tmp_size = data->size + FEC_HEADER_SIZE;

	if (FEC_PKTSIZE_MAX < tmp_size) {
		return false;
	}

	if ((tmp_data = malloc(tmp_size)) == NULL) {
		return false;
	}

	tmp_ptr = tmp_data;
	tmp_length = tmp_size;
	tmp_offset = 0;

	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, hdr->code, false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, hdr->flags, false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, //
			sizeof(uint16_t), uint16_t, htons((uint16_t ) data->size), false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint16_t), uint16_t, htons(hdr->gid), false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint16_t), uint16_t, htons(hdr->sid), false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, //
			sizeof(uint16_t), uint16_t, htons(hdr->parity_len), false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, hdr->src_cnt, false);
	_FEC_SET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, 0x00, false);

	if (tmp_length == data->size) {
		memcpy(tmp_ptr, data->ptr, data->size);
		tmp_ptr += data->size;
		tmp_length -= data->size;
		tmp_offset += data->size;
	}
	else {
		return false;
	}

	out->ptr = tmp_data;
	out->size = tmp_offset;

	return true;
}

static bool _FECParsePacket(FECData * __const __restrict data, FECHeader * __restrict out_hdr, FECData * __restrict out_data) {
	FECHeader tmp_hdr;
	uint16_t tmp_payload_size;
	uint8_t * tmp_payload_ptr;
	uint8_t * tmp_ptr;
	size_t tmp_length;
	size_t tmp_offset;
	uint8_t tmp_pad8;

	memset(&tmp_hdr, 0, sizeof(tmp_hdr));
	tmp_payload_size = 0;
	tmp_ptr = data->ptr;
	tmp_length = data->size;

	tmp_offset = 0;

	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, tmp_hdr.code, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, tmp_hdr.flags, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint16_t), uint16_t, tmp_payload_size, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint16_t), uint16_t, tmp_hdr.gid, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint16_t), uint16_t, tmp_hdr.sid, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, //
			sizeof(uint16_t), uint16_t, tmp_hdr.parity_len, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, tmp_hdr.src_cnt, false);
	_FEC_GET(tmp_ptr, tmp_length, tmp_offset, sizeof(uint8_t), uint8_t, tmp_pad8, false);

	if (tmp_hdr.code != FEC_HEADER_CODE) {
		return false;
	}

	if (FECFlagsValidate(tmp_hdr.flags) == false) {
		return false;
	}

	tmp_payload_size = htons(tmp_payload_size);
	tmp_hdr.gid = htons(tmp_hdr.gid);
	tmp_hdr.sid = htons(tmp_hdr.sid);
	tmp_hdr.parity_len = htons(tmp_hdr.parity_len);

	//if (tmp_hdr.src_cnt < FEC_DATANUM_MIN) {
	//	return false;
	//}

	if (tmp_length < tmp_payload_size) {
		return false;
	}

	if (out_data != NULL) {
		if ((tmp_payload_ptr = malloc(tmp_payload_size)) != NULL) {
			memcpy(tmp_payload_ptr, tmp_ptr, tmp_payload_size);
		}
		else {
			return false;
		}

		out_data->ptr = tmp_payload_ptr;
		out_data->size = tmp_payload_size;
	}

	*out_hdr = tmp_hdr;

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static void _FECCalculate(size_t cnt, FECUnit ** __restrict ptrs, size_t * sizes, uint8_t * __restrict out_ptr, size_t out_size) {
	static const FECUnit masks[8] = { //
			0,
			FEC_PARITY_MASK8,
			FEC_PARITY_MASK16,
			FEC_PARITY_MASK24,
			FEC_PARITY_MASK32,
			FEC_PARITY_MASK40,
			FEC_PARITY_MASK48,
			FEC_PARITY_MASK56 };

	FECUnit * tmp_ptr;
	size_t tmp_offset;
	int32_t tmp_diff;
	FECUnit i;
	FECUnit j;

	tmp_ptr = (FECUnit *) out_ptr;
	tmp_offset = 0;
	tmp_diff = 0;
	i = 0;
	j = 0;

	while (tmp_offset < out_size) {
		FECUnit xor = 0;
		// printf("off=%d, max=%d, i=%d\n", off, size_to_compute, i);
		// *ppb = 0; *ppb = (*ppb ^ in[j][i]) // It is heavy to use substitutions with a pointer and xor calcuration.
		while (j < cnt) {
			// printf("j=%d, number_of_in=%d\n", j, number_of_in);
			tmp_diff = sizes[j] - tmp_offset;
			// Improve a peformance to access input arrays. (see a part of a RC_EGL_gen_parity_for_3packets function, '*ppb++ = (*fpb++ ^ *spb++ ^ *tpb++);'.)
			if (0 < tmp_diff) {
				if (tmp_diff < FEC_PARITY_UNIT) {
					// printf("xor:%d ^ (%d & %d) = %d\n", xor, in[j][i], masks[diff], (xor ^ (in[j][i] & masks[diff])));
					xor = (xor ^ (ptrs[j][i] & masks[tmp_diff]));
				}
				else {
					// printf("xor:%d ^ %d = %d\n", xor, in[j][i], (xor ^ in[j][i]));
					xor = (xor ^ ptrs[j][i]);
				}
			}
			else {
				// printf("xor:%d ^ %d = %d\n", xor, 0, (xor ^ 0));
				xor = (xor ^ 0);
			}
			j++;
		}
		*tmp_ptr++ = xor;
		tmp_offset += FEC_PARITY_UNIT;
		i++;
		j = 0;
	}

}

static bool _FECGenerateParityData(FECData * __restrict data_ptrs, uint32_t data_cnt, uint8_t * __restrict out_ptr, size_t * out_size, size_t * out_parity_len) {
	size_t tmp_out_size;
	FECUnit * tmp_ptrs[FEC_DATANUM_MAX];
	size_t tmp_sizes[FEC_DATANUM_MAX];
	size_t tmp_parity_len;
	size_t tmp_size;
	size_t i;

	if (data_ptrs == NULL) {
		return false;
	}

	if (data_cnt < 1) {
		return false;
	}

	if (out_ptr == NULL) {
		return false;
	}

	if (out_size == NULL) {
		return false;
	}

	if ((tmp_out_size = *out_size) < 1) {
		return false;
	}

	memset(tmp_ptrs, 0, sizeof(tmp_ptrs));
	memset(&tmp_sizes, 0, sizeof(tmp_sizes));
	tmp_parity_len = 0;
	tmp_size = 0;

	for (i = 0; i < data_cnt; i++) {
		if (data_ptrs[i].size < 1) {
			return ((size_t) -1);
		}

		tmp_ptrs[i] = (FECUnit *) data_ptrs[i].ptr;
		tmp_sizes[i] = data_ptrs[i].size;

		tmp_parity_len ^= data_ptrs[i].size;

		if (tmp_size < data_ptrs[i].size) {
			tmp_size = data_ptrs[i].size;
		}
	}

	if (tmp_out_size < tmp_size) {
		return false;
	}

	_FECCalculate(data_cnt, tmp_ptrs, tmp_sizes, out_ptr, tmp_out_size);

	*out_size = tmp_size;
	*out_parity_len = tmp_parity_len;

	return tmp_size;
}

static bool _FECRecoveryData(FECData * __restrict data_ptrs, uint32_t data_cnt, //
		size_t parity_size, size_t parity_len, //
		uint8_t * __restrict out_ptr, size_t out_size) {
	FECUnit * tmp_ptrs[FEC_DATANUM_MAX];
	size_t tmp_sizes[FEC_DATANUM_MAX];
	size_t i;

	if (data_ptrs == NULL) {
		return false;
	}

	if (data_cnt < 1) {
		return false;
	}

	if (out_ptr == NULL) {
		return false;
	}

	if (out_size < 1) {
		return false;
	}

	memset(tmp_ptrs, 0, sizeof(tmp_ptrs));
	memset(&tmp_sizes, 0, sizeof(tmp_sizes));

	for (i = 0; i < data_cnt; i++) {
		if (data_ptrs[i].size < 1) {
			return false;
		}

		tmp_ptrs[i] = (FECUnit *) data_ptrs[i].ptr;
		tmp_sizes[i] = data_ptrs[i].size;
	}

	_FECCalculate(data_cnt, tmp_ptrs, tmp_sizes, out_ptr, out_size);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

static void _FECfree(FECData * ptrs, size_t cnt) {
	size_t i;

	for (i = 0; i < cnt; i++) {
		free(ptrs[i].ptr);
		ptrs[i].size = 0;
	}

}

bool FECEncode(FECGroupId id, FECData * __restrict data_ptrs, size_t data_cnt, FECData * __restrict out_ptrs, size_t * __restrict out_cnt) {
	size_t tmp_out_cnt;
	uint8_t tmp_buffer[FEC_PKTSIZE_MAX];
	uint8_t * tmp_ptr;
	size_t tmp_size;
	size_t tmp_cnt;
	size_t tmp_parity_size;
	size_t tmp_parity_len;
	FECHeader tmp_hdr;
	FECData tmp_data;
	size_t i;

	if (FECGroupIdValidate(id) == false) {
		return false;
	}

	if (data_ptrs == NULL) {
		return false;
	}

	if (data_cnt < FEC_DATANUM_MIN) {
		return false;
	}

	if (out_ptrs == NULL) {
		return false;
	}

	if (out_cnt == NULL) {
		return false;
	}

	if ((tmp_out_cnt = *out_cnt) <= data_cnt) {
		return false;
	}

	tmp_ptr = tmp_buffer;
	tmp_size = sizeof(tmp_buffer);

	if (_FECGenerateParityData(data_ptrs, data_cnt, tmp_ptr, &tmp_size, &tmp_parity_len)) {
		tmp_parity_size = tmp_size;
	}
	else {
		return false;
	}

	tmp_cnt = 0;

	memset(&tmp_hdr, 0, sizeof(tmp_hdr));
	tmp_hdr.code = FEC_HEADER_CODE;
	tmp_hdr.flags = FEC_FLAGS_NONE;
	tmp_hdr.gid = id;
	tmp_hdr.sid = _FECGenerateSetId();
	tmp_hdr.parity_len = tmp_parity_len;
	tmp_hdr.src_cnt = data_cnt;

	for (i = 0; i < data_cnt; i++) {
		if (_FECBuildPacket(&tmp_hdr, &data_ptrs[i], &out_ptrs[i])) {
			tmp_cnt++;
		}
		else {
			_FECfree(out_ptrs, tmp_cnt);
			return false;
		}
	}

	//memset(&tmp_hdr, 0, sizeof(tmp_hdr));
	//tmp_hdr.code = FEC_HEADER_CODE;
	tmp_hdr.flags |= FEC_FLAGS_PARITY;
	//tmp_hdr.gid = FECGenerateGroupId();
	//tmp_hdr.sid = FECGenerateSetId();
	//tmp_hdr.parity_len = tmp_parity_len;
	//tmp_hdr.src_cnt = data_cnt;

	memset(&tmp_data, 0, sizeof(tmp_data));
	tmp_data.ptr = tmp_buffer;
	tmp_data.size = tmp_parity_size;

	if (_FECBuildPacket(&tmp_hdr, &tmp_data, &out_ptrs[i])) {
		tmp_cnt++;
	}
	else {
		return false;
	}

	*out_cnt = tmp_cnt;

	return true;
}

bool FECDecode(FECData * __restrict data_ptrs, size_t data_cnt, FECData * __restrict out_ptrs, size_t * __restrict out_cnt, FECGroupId * out_id, FECDecodeResult * out_result) {
	size_t tmp_out_cnt;
	size_t tmp_cnt;
	FECHeader tmp_hdrs[FEC_DATANUM_MAX];
	FECData tmp_datas[FEC_DATANUM_MAX];
	FECHeader * tmp_hdr;
	FECData * tmp_parity;
	size_t i;
	FECDecodeResult tmp_result;

	if (data_ptrs == NULL) {
		return false;
	}

	//if (data_cnt < FEC_DATANUM_MIN) {
	//	return false;
	//}

	if (out_ptrs == NULL) {
		return false;
	}

	if (out_cnt == NULL) {
		return false;
	}

	if ((tmp_out_cnt = *out_cnt) < data_cnt) {
		return false;
	}

	if (out_id == NULL) {
		return false;
	}

	tmp_cnt = 0;
	memset(&tmp_hdrs, 0, sizeof(tmp_hdrs));
	memset(&tmp_datas, 0, sizeof(tmp_datas));
	tmp_hdr = NULL;
	tmp_parity = NULL;
	memset(&tmp_result,0,sizeof(tmp_result));

	for (i = 0; i < data_cnt; i++) {
		if (_FECParsePacket(&data_ptrs[i], &tmp_hdrs[i], &tmp_datas[i])) {
			tmp_cnt++;
		}
		else {
			_FECfree(tmp_datas, tmp_cnt);
			return false;
		}

		if ((tmp_hdrs[i].flags & FEC_FLAGS_PARITY)) {
			tmp_result.parity_cnt++;
			if (tmp_parity == NULL) {
				tmp_parity = &tmp_datas[i];
			}
			else {
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}
		}

		if (tmp_hdr != NULL) {
			if (_FECIsSet(tmp_hdr, &tmp_hdr[i]) == false) {
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}
		}
		else {
			tmp_hdr = &tmp_hdrs[i];
		}
	}

	if (tmp_hdr->src_cnt == tmp_cnt) {
		if (tmp_parity != NULL) {
			uint8_t tmp_buffer[FEC_PKTSIZE_MAX];
			uint8_t * tmp_ptr;
			size_t tmp_size;

			tmp_size = tmp_hdr->parity_len;

			for (i = 0; i < data_cnt; i++) {
				if (!(tmp_hdrs[i].flags & FEC_FLAGS_PARITY)) {
					tmp_size ^= tmp_datas[i].size;
				}
			}

			if (tmp_parity->size < tmp_size) {
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}

			memset(tmp_buffer, 0, sizeof(tmp_buffer));

			if (_FECRecoveryData(tmp_datas, tmp_cnt, tmp_parity->size, tmp_hdr->parity_len, tmp_buffer, sizeof(tmp_buffer)) == false) {
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}

			if (tmp_parity->size < tmp_size) {
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}

			if ((tmp_ptr = malloc(tmp_size)) != NULL) {
				memcpy(tmp_ptr, tmp_buffer, tmp_size);
			}
			else {
				free(tmp_ptr);
				_FECfree(tmp_datas, tmp_cnt);
				return false;
			}

			tmp_datas[tmp_cnt].ptr = tmp_ptr;
			tmp_datas[tmp_cnt].size = tmp_size;
			tmp_cnt++;
			tmp_result.recovery_success_cnt++;
		}
		else {
			// No parity
		}
	}

	if (tmp_out_cnt < tmp_cnt) {
		_FECfree(tmp_datas, tmp_cnt);
		return false;
	}

	tmp_out_cnt = 0;

	for (i = 0; i < tmp_cnt; i++) {
		if (tmp_hdr[i].flags & FEC_FLAGS_PARITY) {
			free(tmp_datas[i].ptr);
			tmp_datas[i].ptr = NULL;
			tmp_datas[i].size = 0;
		}
		else {
			out_ptrs[tmp_out_cnt++] = tmp_datas[i];
		}
	}

	*out_cnt = tmp_out_cnt;
	*out_id = tmp_hdr->gid;

	if (tmp_out_cnt < tmp_hdr->src_cnt)
	{
		tmp_result.recovery_failure_cnt = tmp_hdr->src_cnt - tmp_out_cnt;
	}

	*out_result = tmp_result;

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool FECWrapHeader(FECGroupId id, FECData * __const __restrict data, FECData * __restrict out) {
	FECHeader tmp_hdr;

	memset(&tmp_hdr, 0, sizeof(tmp_hdr));
	tmp_hdr.code = FEC_HEADER_CODE;
	tmp_hdr.flags = FEC_FLAGS_NONE;
	tmp_hdr.gid = id;
	tmp_hdr.sid = _FECGenerateSetId();
	tmp_hdr.parity_len = 0;
	tmp_hdr.src_cnt = 1;

	if (_FECBuildPacket(&tmp_hdr, data, out) == false) {
		return false;
	}

	return true;
}

bool FECStripHeader(FECData * __const __restrict data, FECHeader * __restrict out_hdr, FECData * __restrict out_data) {
	return _FECParsePacket(data, out_hdr, out_data);
}

/* ---------------------------------------------------------------------------------------------- */

bool FECGetHeader(uint8_t * const __restrict ptr, size_t size, FECHeader * __restrict out) {
	FECData tmp_data;
	FECHeader tmp_hdr;

	memset(&tmp_data, 0, sizeof(tmp_data));
	tmp_data.ptr = ptr;
	tmp_data.size = size;

	if (_FECParsePacket(&tmp_data, &tmp_hdr, NULL)) {
		memcpy(out, &tmp_hdr, sizeof(FECHeader));
	}
	else {
		return false;
	}

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

bool FECIsFEC(uint8_t * const __restrict ptr, size_t size, FECGroupId * __restrict out) {
	FECData tmp_data;
	FECHeader tmp_hdr;

	memset(&tmp_data, 0, sizeof(tmp_data));
	tmp_data.ptr = ptr;
	tmp_data.size = size;

	if (_FECParsePacket(&tmp_data, &tmp_hdr, NULL)) {
		*out = tmp_hdr.gid;
	}
	else {
		return false;
	}

	return true;
}

bool FECIsParity(FECHeader * hdr) {

	if (hdr == NULL) {
		return false;
	}

	return (hdr->flags & FEC_FLAGS_PARITY ? true : false);
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
