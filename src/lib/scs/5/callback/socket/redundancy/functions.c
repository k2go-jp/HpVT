#define SCS_SOURCECODE_FILEID	"5CBKSKTREDFNC"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defines.h"
#include "scs/5/callback/socket/redundancy/functions.h"
#include "scs/5/callback/socket/redundancy/result.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

SCSRedundancyCallbackResult SCSConnectionCallbackFunction(
		SCSRedundancyCallbackConfig * __restrict config,
		__const SCSRedundancyCallbackStatus * __restrict status) {
	SCSRedundancyCallbackResult tmp_result;

	//SCS_PRINT_DEBUG("%s(%d,%zd.%zd)", __func__, status->times, status->interval.tv_sec, status->interval.tv_nsec);

	SCSRedundancyCallbackResultInitialize(&tmp_result);

	switch (status->times) {
		case 1: {
			SCSTimespecSet(tmp_result.interval, 0, (100 * 1000 * 1000)); // 100 msec
			tmp_result.cancel = false;
			break;
		}
		case 2: {
			SCSTimespecSet(tmp_result.interval, 0, (200 * 1000 * 1000)); // 200 msec
			tmp_result.cancel = false;
			break;
		}
		case 3: {
			SCSTimespecSet(tmp_result.interval, 0, (300 * 1000 * 1000)); // 300 msec
			tmp_result.cancel = false;
			break;
		}
		case 4: {
			SCSTimespecSet(tmp_result.interval, 0, (500 * 1000 * 1000)); // 500 msec
			tmp_result.cancel = false;
			break;
		}
		case 5: {
			SCSTimespecSet(tmp_result.interval, 0, (800 * 1000 * 1000)); // 800 msec
			tmp_result.cancel = false;
			break;
		}
		case 6: {
			SCSTimespecSet(tmp_result.interval, 1, (300 * 1000 * 1000)); // 1300 msec
			tmp_result.cancel = false;
			break;
		}
		case 7: {
			SCSTimespecSet(tmp_result.interval, 2, (100 * 1000 * 1000)); // 2100 msec
			tmp_result.cancel = false;
			break;
		}
		case 8: {
			SCSTimespecSet(tmp_result.interval, 3, (400 * 1000 * 1000)); // 3400 msec
			tmp_result.cancel = false;
			break;
		}
		case 9: {
			SCSTimespecSet(tmp_result.interval, 5, (500 * 1000 * 1000)); // 5500 msec
			tmp_result.cancel = false;
			break;
		}
		default: {
			tmp_result.cancel = true;
			break;
		}
	}

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

SCSRedundancyCallbackResult SCSCloseCallbackFunction(
		SCSRedundancyCallbackConfig * __restrict config,
		__const SCSRedundancyCallbackStatus * __restrict status) {
	SCSRedundancyCallbackResult tmp_result;

	//SCS_PRINT_DEBUG("%s(%d,%zd.%zd)", __func__, status->times, status->interval.tv_sec, status->interval.tv_nsec);

	SCSRedundancyCallbackResultInitialize(&tmp_result);

	switch (status->times) {
		case 1: {
			SCSTimespecSet(tmp_result.interval, 0, (100 * 1000 * 1000)); // 100 msec
			tmp_result.cancel = false;
			break;
		}
		case 2: {
			SCSTimespecSet(tmp_result.interval, 0, (200 * 1000 * 1000)); // 1000 msec
			tmp_result.cancel = false;
			break;
		}
		case 3: {
			SCSTimespecSet(tmp_result.interval, 0, (300 * 1000 * 1000)); // 3000 msec
			tmp_result.cancel = false;
			break;
		}
		default: {
			tmp_result.cancel = true;
			break;
		}
	}

	return tmp_result;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
