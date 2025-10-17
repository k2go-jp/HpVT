#define SCS_SOURCECODE_FILEID	"5CBKUTL"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/5/callback/system/error/config.h"
#include "scs/5/callback/system/error/status.h"
#include "scs/5/callback/system/socket/config.h"
#include "scs/5/callback/system/socket/status.h"
#include "scs/5/callback/system/system/config.h"
#include "scs/5/callback/system/system/status.h"
#include "scs/5/callback/system/types.h"
#include "scs/5/context/context.h"
#include "scs/5/socket/socket.h"
#include "scs/debug.h"
#include "scs/internal.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSCallbackConfigFinalize(SCSCallbackType type, void * ptr, size_t size) {

	switch (type) {
		case SCS_CALLBACKTYPE_ERROR: {
			SCSErrorCallbackConfigFinalize((SCSErrorCallbackConfig *) ptr);
			break;
		}
		case SCS_CALLBACKTYPE_SOCKET: {
			SCSSocketCallbackConfigFinalize((SCSSocketCallbackConfig *) ptr);
			break;
		}
		case SCS_CALLBACKTYPE_SYSTEM: {
			SCSSystemCallbackConfigFinalize((SCSSystemCallbackConfig *) ptr);
			break;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

bool SCSCallbackConfigValidate(SCSCallbackType type, void * ptr, size_t size) {

	switch (type) {
		case SCS_CALLBACKTYPE_ERROR: {

			if (sizeof(SCSErrorCallbackConfig) != size) {
				return false;
			}

			if (SCSErrorCallbackConfigValidate((SCSErrorCallbackConfig *) ptr) == false) {
				return false;
			}

			return true;
		}
		case SCS_CALLBACKTYPE_SOCKET: {

			if (sizeof(SCSSocketCallbackConfig) != size) {
				return false;
			}

			if (SCSSocketCallbackConfigValidate((SCSSocketCallbackConfig *) ptr) == false) {
				return false;
			}

			return true;
		}
		case SCS_CALLBACKTYPE_SYSTEM: {

			if (sizeof(SCSSystemCallbackConfig) != size) {
				return false;
			}

			if (SCSSystemCallbackConfigValidate((SCSSystemCallbackConfig *) ptr) == false) {
				return false;
			}

			return true;
		}
		default: {
			return false;
		}
	}

}

/* ---------------------------------------------------------------------------------------------- */

static inline bool _SCSCallErrorCallbackFunction(SCSErrorCallbackConfig * __restrict config,
		SCSCallbackId id, SCSContext * __restrict context, scs_time * __restrict interval) {
	SCSErrorCallbackStatus tmp_status;

	SCSErrorCallbackStatusInitialize(&tmp_status);
	tmp_status.id = id;
	SCSLoggerGetErrorInfo(&_scs_global_logger, &tmp_status.error);

	if (SCSErrorCallbackConfigValidate(config)) {
		config->func(config, &tmp_status);
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", id);
		return false;
	}

	*interval = config->interval;

	return true;
}
static inline bool _SCSCallSocketCallbackFunction(SCSSocketCallbackConfig * __restrict config,
		SCSCallbackId id, SCSContext * __restrict context, scs_time * __restrict interval) {
	SCSSocket * tmp_socket;
	SCSSocketCallbackStatus tmp_status;

	if (SCSSocketCallbackConfigValidate(config) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", id);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&context->sockets, config->id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 79811, "<<%d>>", config->id);
		return false;
	}

	SCSSocketCallbackStatusInitialize(&tmp_status);
	tmp_status.id = id;
	SCSLoggerGetErrorInfo(&_scs_global_logger, &tmp_status.system.error);
	SCSNetworkTrafficCounterGet(&context->traiffc, &tmp_status.system.traffic);
	SCSSocketGetNetworkTrafficInfo(tmp_socket, &tmp_status.socket.traffic);
	SCSSocketGetFeedbackInfo(tmp_socket, &tmp_status.socket.feedback.basic);
	SCSSocketGetExtendedFeedbackInfo(tmp_socket, //
			&tmp_status.socket.feedback.extension.ptr, &tmp_status.socket.feedback.extension.size);
	tmp_status.socket.plr = SCSSocketGetPLR(tmp_socket);
	tmp_status.socket.rtt = SCSSocketGetRTT(tmp_socket);
	tmp_status.socket.mtu = SCSSocketGetMTU(tmp_socket);
	tmp_status.socket.pmtu = SCSSocketGetPathMTU(tmp_socket);
	SCSSocketGetBufferStatus(tmp_socket, &tmp_status.socket.buffer);

	config->func(config, &tmp_status);

	*interval = config->interval;

	SCSSocketFree(tmp_socket);

	return true;
}
static inline bool _SCSCallSystemCallbackFunction(SCSSystemCallbackConfig * __restrict config,
		SCSCallbackId id, SCSContext * __restrict context, scs_time * __restrict interval) {
	SCSSystemCallbackStatus tmp_status;

	SCSSystemCallbackStatusInitialize(&tmp_status);
	tmp_status.id = id;
	SCSLoggerGetErrorInfo(&_scs_global_logger, &tmp_status.error);
	SCSNetworkTrafficCounterGet(&context->traiffc, &tmp_status.traffic);

	if (SCSSystemCallbackConfigValidate(config)) {
		config->func(config, &tmp_status);
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%zu>>", id);
		return false;
	}

	*interval = config->interval;

	return true;
}

bool SCSCallCallbackFunction(SCSCallbackType type, void * __restrict ptr, size_t size,
		SCSCallbackId id, scs_time * __restrict interval) {
	bool tmp_result;
	SCSContext * tmp_context;

//	if (!SCSCallbackTypeValidate(type)) {
//		SCS_LOG(WARN, SYSTEM, 99997,  "<<%d>>", type);
//		return false;
//	}

//	if (ptr == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

//	if (size < 1 {
//		SCS_LOG(WARN, SYSTEM, 99997,  "<<%zu>>", size);
//		return false;
//	}

//	if (!SCSCallbackIdValidate(id)) {
//		SCS_LOG(WARN, SYSTEM, 99997,  "<<%d>>", id);
//		return false;
//	}

//	if (interval == NULL) {
//		SCS_LOG(WARN, SYSTEM, 99998, "");
//		return false;
//	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		return false;
	}

	switch (type) {
		case SCS_CALLBACKTYPE_ERROR: {
			tmp_result = _SCSCallErrorCallbackFunction( //
					(SCSErrorCallbackConfig *) ptr, id, tmp_context, interval);
			break;
		}
		case SCS_CALLBACKTYPE_SOCKET: {
			tmp_result = _SCSCallSocketCallbackFunction( //
					(SCSSocketCallbackConfig *) ptr, id, tmp_context, interval);
			break;
		}
		case SCS_CALLBACKTYPE_SYSTEM: {
			tmp_result = _SCSCallSystemCallbackFunction( //
					(SCSSystemCallbackConfig *) ptr, id, tmp_context, interval);
			break;
		}
		default: {
			SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", type);
			tmp_result = false;
			break;
		}
	}

	SCSContextFree(tmp_context);

	return tmp_result;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
