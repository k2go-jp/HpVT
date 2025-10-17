#define SCS_SOURCECODE_FILEID	"5SCKUTL"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/stdbool.h"
#include "scs/1/stdlib.h"
#include "scs/2/logger.h"
#include "scs/5/socket/datagram/socket.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

SCSFeedbackInfo * SCSGenerateFeedbackInfo(SCSDatagramSocket * socket) {
	SCSFeedbackInfo * tmp_info;
	scs_timespec tmp_timestamp;
	scs_timespec tmp_rtt;

	if (socket == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return NULL;
	}

	if ((tmp_info = (SCSFeedbackInfo *) malloc(sizeof(SCSFeedbackInfo))) == NULL) {
		SCS_LOG(ALERT, MEMORY, 99999, "<<%zu>>", sizeof(SCSFeedbackInfo));
		return NULL;
	}

	SCSTimespecSetCurrentTime(tmp_timestamp, CLOCK_MONOTONIC);

	SCSFeedbackInfoInitialize(tmp_info);

	tmp_info->timestamp = SCSTimespecConvertToMillisec64(tmp_timestamp);

	SCSNetworkTrafficCounterGet(&socket->traffic, &tmp_info->traffic);
	tmp_rtt = SCSRTTMeasurerGetMin(&socket->rtt.measurer);
	tmp_info->rtt.minimum = SCSTimespecConvertToMillisec32(tmp_rtt);
	tmp_rtt = SCSRTTMeasurerGetMax(&socket->rtt.measurer);
	tmp_info->rtt.maximum = SCSTimespecConvertToMillisec32(tmp_rtt);
	tmp_rtt = SCSRTTMeasurerGetLatest(&socket->rtt.measurer);
	tmp_info->rtt.current = SCSTimespecConvertToMillisec32(tmp_rtt);

	tmp_info->plr.minimum = SCSPLRMeasurerGetMin(&socket->plr.measurer);
	tmp_info->plr.maximum = SCSPLRMeasurerGetMax(&socket->plr.measurer);
	tmp_info->plr.current = SCSPLRMeasurerGetLatest(&socket->plr.measurer);

	tmp_info->mtu = SCSPMTUMeasurerGet(&socket->pmtu.measurer);

	//SCSFeedbackInfoLogging(tmp_info);

	return tmp_info;
}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
