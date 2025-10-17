#define SCS_SOURCECODE_FILEID	"5DGMSTG"

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "scs/1/errno.h"
#include "scs/1/inttypes.h"
#include "scs/1/stdbool.h"
#include "scs/2/logger.h"
#include "scs/5/defaults.h"
#include "scs/5/defines.h"
#include "scs/5/feature/plr.h"
#include "scs/5/callback/socket/feedback/config.h"
#include "scs/5/callback/socket/redundancy/config.h"
#include "scs/5/callback/socket/redundancy/functions.h"
#include "scs/5/packet/watcher.h"
#include "scs/5/packet/types.h"
#include "scs/5/socket/datagram/settings.h"
#include "scs/debug.h"

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketSettingsInitialize(SCSDatagramSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	memset(self, 0, sizeof(SCSDatagramSocketSettings));

	self->connect.timeout = SCS_DEFAULT_SOCKET_CONNECT_TIMEOUT;
	SCSRedundancyCallbackConfigInitialize(&self->connect.redundancy);
	SCSRedundancyCallbackConfigSet(&self->connect.redundancy,
			SCS_DEFAULT_SOCKET_CONNECT_REDUNDANCY_TIMES,
			SCS_DEFAULT_SOCKET_CONNECT_REDUNDANCY_FUNCTION);

	self->send.timeout = SCS_DEFAULT_SOCKET_SEND_TIMEOUT;
	SCSRedundancyCallbackConfigInitialize(&self->send.redundancy);
	SCSRedundancyCallbackConfigSet(&self->send.redundancy, //
			SCS_DEFAULT_SOCKET_SEND_REDUNDANCY_TIMES,//
			SCS_DEFAULT_SOCKET_SEND_REDUNDANCY_FUNCTION);
	self->send.queue.length = SCS_DEFAULT_SOCKET_SEND_QUEUELENGTH;

	self->receive.timeout = SCS_DEFAULT_SOCKET_RECEIVE_TIMEOUT;
	self->receive.queue.length = SCS_DEFAULT_SOCKET_RECEIVE_QUEUELENGTH;

	SCSFeedbackCallbackConfigInitialize(&self->notification.callback);
	SCSTimespecInitialize(self->notification.timeout);
	SCSTimespecSet(self->notification.timeout, SCS_DEFAULT_SOCKET_NOTIFICATION_TIMEOUT, 0);
	SCSTimespecInitialize(self->notification.interval);
	SCSTimespecSet(self->notification.interval, //
			(SCS_DEFAULT_SOCKET_NOTIFICATION_INTERVAL / 1000),//
			((SCS_DEFAULT_SOCKET_NOTIFICATION_INTERVAL % 1000) * 1000 * 1000));

	self->verification.enabled = SCS_DEFAULT_SOCKET_VERIFICATION_ENABLED;
	self->verification.method = SCS_DEFAULT_SOCKET_VERIFICATION_METHOD;

	self->pmtudisc.enabled = SCS_DEFAULT_SOCKET_PMTUDISCOVERY_ENABLED;
	self->pmtudisc.interval.seconds = SCS_DEFAULT_SOCKET_PMTUDISCOVERY_INTERVALSECONDS;
	self->pmtudisc.interval.packets = SCS_DEFAULT_SOCKET_PMTUDISCOVERY_INTERVALPACKETS;
	self->pmtudisc.interval.bytes = SCS_DEFAULT_SOCKET_PMTUDISCOVERY_INTERVALBYTES;

	self->rttmeas.enabled = SCS_DEFAULT_SOCKET_RTTMEASUREMENT_ENABLED;
	SCSPacketWatcherSettingsInitialize(&self->rttmeas.watcher);
	self->rttmeas.watcher.capacity = SCS_DEFAULT_SOCKET_RTTMEASUREMENT_MAXPACKETS;
	self->rttmeas.watcher.id.minimum = SCS_PKTRTTMID_MINVAL;
	self->rttmeas.watcher.id.maximum = SCS_PKTRTTMID_MAXVAL;

	self->plrmeas.enabled = SCS_DEFAULT_SOCKET_PLRMEASUREMENT_ENABLED;
	self->plrmeas.samples = SCS_DEFAULT_SOCKET_PLRMEASUREMENT_SAMPLES;

	self->jittermeas.enabled = SCS_DEFAULT_SOCKET_JITTERMEASUREMENT_ENABLED;
	self->jittermeas.samples = SCS_DEFAULT_SOCKET_JITTERMEASUREMENT_SAMPLES;

	self->close.delaytime = SCS_DEFAULT_SOCKET_CLOSE_DELAYTIME;
	SCSRedundancyCallbackConfigInitialize(&self->close.redundancy);
	SCSRedundancyCallbackConfigSet(&self->close.redundancy, //
			SCS_DEFAULT_SOCKET_CLOSE_REDUNDANCY_TIMES,//
			SCS_DEFAULT_SOCKET_CLOSE_REDUNDANCY_FUNCTION);

}
void SCSDatagramSocketSettingsFinalize(SCSDatagramSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	self->connect.timeout = 0;
	self->connect.redundancy.times = -1;
	SCSRedundancyCallbackConfigFinalize(&self->connect.redundancy);

	self->send.timeout = 0;
	self->send.redundancy.times = -1;
	SCSRedundancyCallbackConfigFinalize(&self->send.redundancy);

	self->receive.timeout = 0;
	//self->receive.queue.length = 0;

	SCSFeedbackCallbackConfigFinalize(&self->notification.callback);
	SCSTimespecFinalize(self->notification.timeout);
	SCSTimespecFinalize(self->notification.interval);

	//self->verification.enabled = false;
	self->verification.method = SCS_PKTVRFMETHOD_INVVAL;

	//self->pmtudiscovery.enabled = false;
	//self->pmtudiscovery.interval.seconds = 0;
	//self->pmtudiscovery.interval.packets = 0;
	//self->pmtudiscovery.interval.bytes = 0;

	//self->rttmeasurement.enabled = false;
	SCSPacketWatcherSettingsFinalize(&self->rttmeas.watcher);

	//self->plrmeas.enabled = false;
	//self->plrmeas.samples = 0;

	//self->jittermeas.enabled = false;
	//self->jittermeas.samples = 0;

	//self->close.delaytime = 0;
	SCSRedundancyCallbackConfigFinalize(&self->close.redundancy);

	memset(self, 0, sizeof(SCSDatagramSocketSettings));

}

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketSettingsAdapt(SCSDatagramSocketSettings * self) {

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	/* Connect */

	//self->connect.timeout
	//self->connect.redundancy
	/* Send */

	//self->send.timeout
	//self->send.redundancy
	if (self->send.queue.length < SCS_PKTQUEUE_MINSIZE) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%s=%d(%d)>>", "send.queue.length",
				SCS_DEFAULT_SOCKET_SEND_QUEUELENGTH, self->send.queue.length);
		self->send.queue.length = SCS_PKTQUEUE_MINSIZE;
	}

	/* Receive */

	//self->receive.timeout
	if (self->receive.queue.length < SCS_PKTQUEUE_MINSIZE) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%s=%d(%d)>>", "receive.queue.length",
				SCS_DEFAULT_SOCKET_RECEIVE_QUEUELENGTH, self->receive.queue.length);
		self->receive.queue.length = SCS_PKTQUEUE_MINSIZE;
	}

	/* Notification */

	//TODO Check self->notification.callback
	//TODO Check self->notification.timeout
	//TODO Check self->notification.interval

	/* Verification */

	//self->verification.enabled;
	if (!SCSPacketVerificationMethodValidate(self->verification.method)) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%s=%d(%d)>>", "verification.method",
				SCS_DEFAULT_SOCKET_VERIFICATION_METHOD, self->verification.method);
		self->verification.method = SCS_DEFAULT_SOCKET_VERIFICATION_METHOD;
	}

	/* PMTU-Discovery */

	//self->pmtudiscovery.enabled
	//self->pmtudiscovery.interval.seconds
	//self->pmtudiscovery.interval.packets
	//self->pmtudiscovery.interval.bytes
	//self->rttmeasurement.enabled
	/* RTT Measurement */

	//self->rttmeasurement.enabled = false;
	SCSPacketWatcherSettingsAdapt(&self->rttmeas.watcher);

	/* PLR Measurement */

	//self->plrmeas.enabled
	if (self->plrmeas.samples < SCS_PLRMEASURER_MINENTRIES) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%s=%d(%d)>>", "plr-meas.samples",
				SCS_PLRMEASURER_MINENTRIES, self->plrmeas.samples);
		self->plrmeas.samples = SCS_PLRMEASURER_MINENTRIES;
	}

	if (SCS_PLRMEASURER_MAXENTRIES < self->plrmeas.samples) {
		SCS_LOG(WARN, SYSTEM, 79995, "<<%s=%d(%d)>>", "plr-meas.samples",
				SCS_PLRMEASURER_MAXENTRIES, self->plrmeas.samples);
		self->plrmeas.samples = SCS_PLRMEASURER_MAXENTRIES;
	}

	/* Jitter Measurement */

	//self->jittermeas.enabled
	//self->jittermeas.samples

	/* Close */

	//self->close.delaytime
	//self->close.redundancy

}

bool SCSDatagramSocketSettingsValidate(SCSDatagramSocketSettings * self) {
	bool tmp_result;

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return false;
	}

	tmp_result = true;

	/* Connect */

	//self->connect.timeout
	//self->connect.redundancy
	/* Send */

	//self->send.timeout
	//self->send.redundancy
	if (self->send.queue.length < SCS_PKTQUEUE_MINSIZE || //
			SCS_PKTQUEUE_MAXSIZE < self->send.queue.length) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "send.queue.length");
		tmp_result = false;
	}

	/* Receive */

	//self->receive.timeout
	if (self->receive.queue.length < SCS_PKTQUEUE_MINSIZE || //
			SCS_PKTQUEUE_MAXSIZE < self->receive.queue.length) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "receive.queue.length");
		tmp_result = false;
	}

	/* Notification */

	if (!SCSFeedbackCallbackConfigValidate(&self->notification.callback)) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "notification.callback");
		tmp_result = false;
	}

	if (!SCSTimespecIsSet(self->notification.timeout)) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "notification.timeout");
		tmp_result = false;
	}

	if (!SCSTimespecIsSet(self->notification.interval)) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "notification.interval");
		tmp_result = false;
	}

	/* Verification */

	//self->verification.enabled
	if (!SCSPacketVerificationMethodValidate(self->verification.method)) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "verification.method");
		tmp_result = false;
	}

	/* PMTU-Discovery */

	if (self->pmtudisc.enabled == true && //
			self->pmtudisc.interval.seconds < 1 && //
			self->pmtudisc.interval.packets < 1 && //
			self->pmtudisc.interval.bytes < 1) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "pmtu-discovery.interval");
		tmp_result = false;
	}

	/* RTT-Measurement */

	if (SCSPacketWatcherSettingsValidate(&self->rttmeas.watcher) == false) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "rtt-measurement.watcher");
		tmp_result = false;
	}

	/* PLR Measurement */

	//self->plrmeas.enabled
	if (self->plrmeas.samples < 1) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "plr-measurement.samples");
		tmp_result = false;
	}

	/* Jitter Measurement */

	//self->jittermeas.enabled
	if (self->jittermeas.samples < 1) {
		SCS_LOG(WARN, SYSTEM, 79996, "<<%s>>", "jitter-measurement.samples");
		tmp_result = false;
	}

	/* Close */
	//self->close.delaytime
	//self->close.redundancy

	return tmp_result;
}

/* ---------------------------------------------------------------------------------------------- */

void SCSDatagramSocketSettingsDump(SCSDatagramSocketSettings * __restrict self,
		__const char * __restrict prefix) {
	const char tmp_caption[] = "socket.";
	char tmp_prefix[UINT8_MAX];
	char tmp_buffer[UINT8_MAX];

	if (self == NULL) {
		SCS_LOG(WARN, SYSTEM, 99998, "");
		return;
	}

	if (prefix == NULL) {
		prefix = "";
	}

	//snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s", prefix, tmp_caption);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "connect.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "timeout", "%zd [sec]",
			self->connect.timeout);
	SCSRedundancyCallbackConfigDump(&self->connect.redundancy, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "send.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "timeout", "%zd [sec]", self->send.timeout);
	SCSRedundancyCallbackConfigDump(&self->send.redundancy, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "receive.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "timeout", "%zd [sec]",
			self->receive.timeout);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "queue.size", "%zu",
			self->receive.queue.length);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "notification.");
	SCSFeedbackCallbackConfigDump(&self->notification.callback, tmp_prefix);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "timeout", "%zd [sec]", //
			SCSTimespecGetSec(self->notification.timeout));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "interval", "%zd [sec]", //
			SCSTimespecGetSec(self->notification.interval));

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "verification.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "enabled", "%s",
			(self->verification.enabled == true ? "Yes" : "No"));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "method", "%s(%d)", //
			SCSPacketVerificationMethodToString(self->verification.method),//
			self->verification.method);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "pmtu-discovery.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "enabled", "%s",
			(self->pmtudisc.enabled == true ? "Yes" : "No"));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "interval.seconds", "%zd [seconds]",
			self->pmtudisc.interval.seconds);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "interval.seconds", "%"PRIu64" [packets]",
			self->pmtudisc.interval.packets);
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "interval.seconds", "%"PRIu64" [bytes]",
			self->pmtudisc.interval.bytes);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "rtt-measurement.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "enabled", "%s",
			(self->rttmeas.enabled == true ? "Yes" : "No"));
	SCSPacketWatcherSettingsDump(&self->rttmeas.watcher, tmp_prefix);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "plr-measurement.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "enabled", "%s",
			(self->jittermeas.enabled == true ? "Yes" : "No"));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "samples", "%zd [packets]",
			self->plrmeas.samples);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "jitter-measurement.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "enabled", "%s",
			(self->jittermeas.enabled == true ? "Yes" : "No"));
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "samples", "%zd [packets]",
			self->jittermeas.samples);

	snprintf(tmp_prefix, sizeof(tmp_prefix), "%s%s%s", prefix, tmp_caption, "close.");
	SCS_DUMP(tmp_buffer, sizeof(tmp_buffer), tmp_prefix, "delaytime", "%zd [sec]",
			self->close.delaytime);
	SCSRedundancyCallbackConfigDump(&self->send.redundancy, tmp_prefix);

}

/* ============================================================================================== */

#undef SCS_SOURCECODE_FILEID
