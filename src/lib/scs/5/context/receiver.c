#define SCS_SOURCECODE_FILEID	"5RVR"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef WIN32
#else
#include <sys/epoll.h>
#endif /* WIN32 */

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/context/context.h"
#include "scs/5/defines.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

static inline void __SCSReceiverWork(SCSContext * context) {
	int tmp_sd;
	struct epoll_event tmp_events[SCS_CHANNEL_MAXNUM];

	tmp_sd = SCSContextGetPollingSocketDescriptor(context);

	while (SCSWorkerIsStarted(&context->worker.receiver)) {
		int tmp_num;
		int i;

		memset(tmp_events, 0, sizeof(tmp_events));

		tmp_num = epoll_wait(tmp_sd, tmp_events, SCS_CHANNEL_MAXNUM, SCS_RECEIVER_TIMEOUT);
		if (tmp_num < 1) {
			if (tmp_num == 0) {
				continue;
			}
			else {
				if (errno == EINTR) {
					continue;
				}

				if (SCSWorkerIsStarted(&context->worker.receiver)) {
					scs_errno tmp_errno;
					char tmp_message[(SCS_EMSGSIZE_MAX + 1)];

					SCSGetLastError(tmp_errno);
					SCSGetErrorMessage(tmp_errno, tmp_message, SCS_EMSGSIZE_MAX);
					SCS_LOG(ALERT, SYSTEM, 00000, "%s <<%d>>", tmp_message, tmp_errno);

					SCSWorkerStop(&context->worker.receiver);
				}

				return;
			}
		}

		for (i = 0; i < tmp_num; i++) {
			int tmp_sd;
			SCSSocket * tmp_socket;

			if (tmp_events[i].events == EPOLLIN) {

				tmp_sd = tmp_events[i].data.fd;

				if ((tmp_socket = SCSSocketTableGet2(&context->sockets, tmp_sd)) == NULL) {
					SCS_LOG(WARN, SYSTEM, 79811, "<<%d>>", tmp_sd);
					continue;
				}

				SCSSocketReceive(tmp_socket, tmp_sd, &context->traiffc);
				SCSSocketFree(tmp_socket);

			}
			else {

				/*
				 switch (tmp_events[i].events) {
				 case EPOLLIN: {
				 break;
				 }
				 case EPOLLHUP: {
				 SCS_LOG(WARN, SYSTEM, 00000, "EPOLLHUP");
				 //TODO close channel
				 continue;
				 }
				 case EPOLLERR: {
				 SCS_LOG(WARN, SYSTEM, 00000, "EPOLLERR");
				 //TODO close channel
				 continue;
				 }
				 default: {
				 //TODO close channel
				 continue;
				 }
				 }
				 */

				tmp_sd = tmp_events[i].data.fd;

				if (SCSContextRemoveReceivableSocketDescriptor(context, tmp_sd) == false) {
					SCS_LOG(WARN, SOCKET, 99999, "");
				}

				if ((tmp_socket = SCSSocketTableGet2(&context->sockets, tmp_sd)) != NULL) {
				 	SCSSocketBroken(tmp_socket);
				 	SCSSocketFree(tmp_socket);
				}
				else {
					SCS_LOG(WARN, SYSTEM, 79811, "<<%d>>", tmp_sd);
				}
			}
		}
	}

}
void * _SCSReceiverWork(void * argument) {
	SCSContext * tmp_self;

	SCS_LOG(NOTICE, SYSTEM, 10001, "<<%s>>", __func__);

	if ((tmp_self = (SCSContext *) argument) == NULL) {
		SCS_LOG(ALERT, SYSTEM, 99999, "");
		return NULL;
	}

	SCSWorkerStarted(&tmp_self->worker.receiver);

	__SCSReceiverWork(tmp_self);

	SCSWorkerStopped(&tmp_self->worker.receiver);

	SCS_LOG(NOTICE, SYSTEM, 10002, "<<%s>>", __func__);

	return NULL;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
