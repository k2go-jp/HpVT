#define SCS_SOURCECODE_FILEID	"0API"

/* ============================================================================================== */

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "scs/api.h"
#include "scs/defines.h"
#include "scs/internal.h"

#include "scs/1/comatibility.h"
#include "scs/1/errno.h"
#include "scs/2/lock.h"
#include "scs/2/logger.h"
#include "scs/3/utils.h"
#include "scs/4/gc.h"
#include "scs/5/channel/channel.h"
#include "scs/5/context/action.h"
#include "scs/5/defaults.h"
#include "scs/5/context/context.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

SCSLogger _scs_global_logger;
SCSGarbageCollector _scs_global_gc;
scs_errno _scs_global_errno = SCS_ERRNO_NONE;

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSStart(SCSSettings * settings) {

	SCS_NOTIFY("Ver.%s (Build %s)", SCS_VERSION, SCS_BUILD);

	SCSLoggerInitialize(&_scs_global_logger);
	SCSGarbageCollectorInitialize(&_scs_global_gc);

	if (SCSGarbageCollectorStart(&_scs_global_gc) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (_SCSStartUp(settings) == false) {
		return false;
	}

	return true;
}
void WINAPI SCSStop(void) {

	//TODO Finalize logger
	//TODO Finalize gc

	_SCSCleanUp();

}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSSetLogLevel(SCSLogType type, SCSLogLevel level) {

	if (SCSLogLevelValidate(level) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", level);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSLogTypeValidate(type) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", level);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	SCSLoggerSetLevel(&_scs_global_logger, type, level);

	return true;
}
bool WINAPI SCSOpenLogFile(const char * path) {
	size_t tmp_length;

	if (path == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_length = strlen(path)) < 1) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%s>>", path);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (PATH_MAX < tmp_length) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%s>>", path);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSLoggerOpenFiles(&_scs_global_logger, path) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%s>>", path);
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	return true;
}

void WINAPI SCSDumpLogger(void) {
	SCSLoggerDump(&_scs_global_logger, "");
}

/* ---------------------------------------------------------------------------------------------- */

void WINAPI SCSGetSettings(SCSSettings * settings) {

	if (settings == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

}

void WINAPI SCSDumpSettings(SCSSettings * settings) {

	if (settings == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	SCSSettingsDump(settings, "");

}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSAddCallback(SCSCallbackType type, void * __restrict ptr, size_t size,
		SCSCallbackId * __restrict out) {
	SCSContext * tmp_context;

	if (!SCSCallbackTypeValidate(type)) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%s(%d)>>", //
				SCSCallbackTypeToString(type), type);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (size < 1) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", size);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if (SCSCallbackListAdd(&tmp_context->callbacks, type, ptr, size, out) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSContextFree(tmp_context);

	return true;
}
bool WINAPI SCSRemoveCallback(SCSCallbackType type, SCSCallbackId id) {
	SCSContext * tmp_context;

	if (!SCSCallbackTypeValidate(type)) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%s(%d)>>", //
				SCSCallbackTypeToString(type), type);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (!SCSCallbackIdValidate(id)) {
		SCS_LOG(WARN, SYSTEM, 80001, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if (SCSCallbackListRemove(&tmp_context->callbacks, id) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSContextFree(tmp_context);

	return true;
}

/* ---------------------------------------------------------------------------------------------- */

SCSSocketId WINAPI SCSCreateSocket(SCSSocketType type, SCSProtocolType protocol) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;
	SCSSocketId tmp_id;

	if (SCSSocketTypeValidate(type) == false) {
		SCS_LOG(ALERT, SYSTEM, 80002, "<<%d>>", type);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return SCS_SKTID_INVVAL;
	}

	if (SCSProtocolTypeValidate(protocol) == false) {
		SCS_LOG(ALERT, SYSTEM, 80002, "<<%d>>", protocol);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return SCS_SKTID_INVVAL;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return SCS_SKTID_INVVAL;
	}

	if ((tmp_socket = SCSSocketCreate(SCS_SKTTYPE_DATAGRAM, SCS_PROTOCOLTYPE_UDP)) != NULL) {
		SCSSocketHold(tmp_socket);
	}
	else {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return SCS_SKTID_INVVAL;
	}

	if (SCSSocketStandBy(tmp_socket, &tmp_context->settings.socket) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketDestroy(tmp_socket);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return SCS_SKTID_INVVAL;
	}

	if (SCSSocketTableAdd(&tmp_context->sockets, tmp_socket) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketDestroy(tmp_socket);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return SCS_SKTID_INVVAL;
	}

	tmp_id = SCSSocketGetId(tmp_socket);

	if (SCSSocketIdValidate(tmp_id) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "<<%d>>", tmp_id);
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		//SCSSocketDestroy(tmp_socket);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return SCS_SKTID_INVVAL;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return tmp_id;
}
void WINAPI SCSDestroySocket(SCSSocketId id) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return;
	}

	SCSSocketShutdown(tmp_socket, SCS_SKTDRCTN_BOTH);

	if (SCSCloserWakeUp(tmp_socket, 1000) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSBind(SCSSocketId id, scs_sockaddr addr) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketBind(tmp_socket, &addr) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketTableUpdate(&tmp_context->sockets, id) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return true;
}
bool WINAPI SCSConnect(SCSSocketId id, scs_sockaddr addr) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;
	scs_socket_desc tmp_sd;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketConnect(tmp_socket, &addr) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if ((tmp_sd = SCSSocketGetSocketDescriptor(tmp_socket)) == SCS_SKTDESC_INVVAL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSContextAddReceivableSocketDescriptor(tmp_context, tmp_sd) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return true;
}
bool WINAPI SCSListen(SCSSocketId id, scs_sockaddr addr) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketListen(tmp_socket, &addr) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketTableUpdate(&tmp_context->sockets, id) == false) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return true;
}
bool WINAPI SCSAccept(SCSSocketId id, scs_sockaddr * addr) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;
	scs_socket_desc tmp_sd;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (addr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSSocketAccept(tmp_socket, addr) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if ((tmp_sd = SCSSocketGetSocketDescriptor(tmp_socket)) == SCS_SKTDESC_INVVAL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSContextAddReceivableSocketDescriptor(tmp_context, tmp_sd) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return true;
}
void WINAPI SCSShutdown(SCSSocketId id, SCSSocketDirection option) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	if (SCSSocketDirectionValidate(option) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", option);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return;
	}

	SCSSocketShutdown(tmp_socket, option);

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

}

/* ---------------------------------------------------------------------------------------------- */

void WINAPI SCSClear(SCSSocketId id, SCSSocketDirection option) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	if (SCSSocketDirectionValidate(option) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", option);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return;
	}

	SCSSocketClear(tmp_socket, option);

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSGetSocketInfo(SCSSocketId id, SCSSocketInfo * out) {
	bool tmp_retval;
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (out == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return false;
	}

	tmp_retval = SCSSocketGetInfo(tmp_socket, out);

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSSetSocketOption(SCSSocketId id, SCSSocketOptionName name, void * ptr, size_t size) {
	bool tmp_retval;
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSSocketOptionNameValidate(name) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", name);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (size < 1) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%zu>>", size);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return false;
	}

	switch (name) {
		case SCS_SKTOPTNAME_FEEDBACK_INTERVAL: {
			scs_timespec tmp_value;

			if (size == sizeof(scs_time)) {
				SCSTimespecSetMsec(tmp_value, *((scs_time * ) ptr));
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
				break;
			}

			tmp_retval = SCSSocketSetFeedbackInterval(tmp_socket, &tmp_value);

			break;
		}
		case SCS_SKTOPTNAME_FEEDBACK_DISABLE: {
			scs_timespec tmp_value;

			SCSTimespecInitialize(tmp_value);
			SCSTimespecSetInfinity(tmp_value);

			tmp_retval = SCSSocketSetFeedbackInterval(tmp_socket, &tmp_value);

			break;
		}
		case SCS_SKTOPTNAME_BANDWIDTH: {
			uint64_t * tmp_value;

			if (size == sizeof(uint64_t)) {
				tmp_value = (uint64_t *) ptr;
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
				break;
			}

			tmp_retval = SCSSocketSetBandwidth(tmp_socket, tmp_value);

			break;
		}
		case SCS_SKTOPTNAME_PATHMTUD_ENABLE: {
			tmp_retval = SCSSocketEnablePathMTUDiscovery(tmp_socket);
			break;
		}
		case SCS_SKTOPTNAME_PATHMTUD_DISABLE: {
			tmp_retval = SCSSocketDisablePathMTUDiscovery(tmp_socket);
			break;
		}
		case SCS_SKTOPTNAME_HASTYDATA: {
			tmp_retval = SCSSocketSetHastyData(tmp_socket, ptr, size);
			break;
		}
		case SCS_SKTOPTNAME_HASTYDATASIZE: {
			tmp_retval = false;
			break;
		}
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return tmp_retval;
}
bool WINAPI SCSGetSocketOption(SCSSocketId id, SCSSocketOptionName name, void * ptr, size_t size) {
	bool tmp_retval;
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSSocketOptionNameValidate(name) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", name);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (size < 1) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%zu>>", size);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return false;
	}

	switch (name) {
		case SCS_SKTOPTNAME_FEEDBACK_INTERVAL: {

			if (size == sizeof(scs_timespec)) {
				*((scs_timespec *) ptr) = SCSSocketGetFeedbackInterval(tmp_socket);
				tmp_retval = true;
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
				break;
			}

			break;
		}
		case SCS_SKTOPTNAME_FEEDBACK_DISABLE: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTOPTNAME_BANDWIDTH: {

			if (size == sizeof(uint64_t)) {
				uint64_t tmp_value;

				if ((tmp_value = SCSSocketGetBandwidth(tmp_socket)) != ((uint64_t) -1)) {
					*((uint64_t *) ptr) = tmp_value;
					tmp_retval = true;
				}
				else {
					tmp_retval = false;
				}
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
				break;
			}

			break;
		}
		case SCS_SKTOPTNAME_PATHMTUD_ENABLE: {
			break;
		}
		case SCS_SKTOPTNAME_PATHMTUD_DISABLE: {
			break;
		}
		case SCS_SKTOPTNAME_HASTYDATA: {
			tmp_retval = SCSSocketGetHastyData(tmp_socket, ptr, size);
			break;
		}
		case SCS_SKTOPTNAME_HASTYDATASIZE: {

			if (size == sizeof(size_t)) {
				*((size_t *) ptr) = SCSSocketGetHastyDataSize(tmp_socket);
				tmp_retval = true;
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
				break;
			}

			break;
		}
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI SCSSetSocketCallback(SCSSocketId id, SCSSocketCallbackType type, void * ptr,
		size_t size) {
	bool tmp_retval;
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSSocketCallbackTypeValidate(type) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", type);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (size < 1) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%zu>>", size);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return false;
	}

	switch (type) {
		case SCS_SKTCBTYPE_CLOSING_CALLBACK: {
			tmp_retval = false;
			//TODO No implement.
			break;
		}
		case SCS_SKTCBTYPE_CONNECTION_CALLBACK: {
			tmp_retval = false;
			break;
		}
		case SCS_SKTCBTYPE_FEEDBACK_CALLBACK: {
			if (size == sizeof(SCSFeedbackCallbackConfig)) {
				tmp_retval = SCSSocketSetFeedbackCallback(tmp_socket, ptr);
			}
			else {
				SCS_LOG(WARN, SYSTEM, 80002, "");
				SCSSetLastError(SCS_ERRNO_EINVALARGS);
				tmp_retval = false;
			}

			break;
		}
		case SCS_SKTCBTYPE_REDUNDANCY_CALLBACK: {
			break;
		}
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return tmp_retval;
}

/* ---------------------------------------------------------------------------------------------- */

bool WINAPI DGSSend(SCSSocketId id, void * __restrict ptr, size_t length, size_t offset,
		SCSSocketOption option, int redundancy, SCSPacketSeqno * __restrict seqno) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;
	SCSPointer tmp_ptr;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCS_PACKET_MAXSIZE < length) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%zu/%zu>>", length, SCS_PACKET_MAXSIZE);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (length < offset) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d/%d>>", offset, length);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSSocketOptionValidate(option) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", option);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCS_REDUNDANCY_MAXTIMES < redundancy) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d/%d>>", redundancy, SCS_REDUNDANCY_MAXTIMES);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 80004, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSContextFree(tmp_context);
		return false;
	}

	if (tmp_socket->type != SCS_SKTTYPE_DATAGRAM) {
		SCS_LOG(WARN, SYSTEM, 80002, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSDatagramSocketCheckSendable(&tmp_socket->datagram) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALSTAT);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSPointerInitialize(tmp_ptr, ptr, length, offset);

	if (!SCSPointerCanWrite(tmp_ptr, 1)) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALSTAT);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSDatagramSocketCheckPayloadLength(&tmp_socket->datagram, length) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EINVALSOCK);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSDatagramSocketEnqueue(&tmp_socket->datagram, tmp_ptr, option, redundancy, seqno) == false) {
		for (;;) {
			if (SCSDatagramSocketWiatToSendable(&tmp_socket->datagram) == false) {
				SCS_LOG(WARN, SYSTEM, 99999, "");
				SCSSetLastError(SCS_ERRNO_EINVALSOCK);
				SCSSocketFree(tmp_socket);
				SCSContextFree(tmp_context);
				return false;
			}

			if (SCSDatagramSocketEnqueue( //
					&tmp_socket->datagram, tmp_ptr, option, redundancy, seqno)) {
				break;
			}
		}
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	return true;
}
bool WINAPI DGSRecv(SCSSocketId id, void * __restrict ptr, size_t length,
		size_t * __restrict offset, SCSSocketOption option, SCSPacketSeqno * __restrict seqno) {
	SCSContext * tmp_context;
	SCSSocket * tmp_socket;
	SCSPointer tmp_ptr;

	if (SCSSocketIdValidate(id) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", id);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (ptr == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (offset == NULL) {
		SCS_LOG(WARN, SYSTEM, 80001, "");
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (length < *offset) {
		SCS_LOG(WARN, SYSTEM, 80001, "<<%zu/%zu>>", *offset, length);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if (SCSSocketOptionValidate(option) == false) {
		SCS_LOG(WARN, SYSTEM, 80002, "<<%d>>", option);
		SCSSetLastError(SCS_ERRNO_EINVALARGS);
		return false;
	}

	if ((tmp_context = _SCSGetContext()) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		return false;
	}

	if ((tmp_socket = SCSSocketTableGet1(&tmp_context->sockets, id)) == NULL) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSContextFree(tmp_context);
		return false;
	}

	if (tmp_socket->type != SCS_SKTTYPE_DATAGRAM) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSDatagramSocketCheckReceivable(&tmp_socket->datagram) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSPointerInitialize(tmp_ptr, ptr, length, *offset);

	if (!SCSPointerCanWrite(tmp_ptr, 1)) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	if (SCSDatagramSocketDequeue(&tmp_socket->datagram, &tmp_ptr, option, seqno) == false) {
		SCS_LOG(WARN, SYSTEM, 99999, "");
		SCSSetLastError(SCS_ERRNO_EILLEGAL);
		SCSSocketFree(tmp_socket);
		SCSContextFree(tmp_context);
		return false;
	}

	SCSSocketFree(tmp_socket);
	SCSContextFree(tmp_context);

	if (offset != NULL) {
		*offset = SCSPointerGetOffset(tmp_ptr);
	}

	return true;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
