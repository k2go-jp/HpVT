#ifndef COMMON_S_H_
#define COMMON_S_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

typedef bool boolean;

#define HPVT_Lock(xxx_locked) \
	if(pthread_mutex_lock(&(xxx_locked)) != 0){ \
	HPVT_logging(LOG_LEVEL_ERROR, "Lock Error!");\
	printf("%s\n",strerror(errno));\
	};

#define HPVT_Unlock(xxx_locked) \
	if(pthread_mutex_unlock(&(xxx_locked)) != 0){\
	HPVT_logging(LOG_LEVEL_ERROR, "Unlock Error!");\
	printf("%s\n",strerror(errno));\
	};

#define HPVT_VERSION_MAJOR					2
#define HPVT_VERSION_MINOR					4
#define HPVT_VERSION_REVISION				0

#define HPVT_LOG_FACILITY_DEFAULT			LOG_FACILITY_LOCAL7
#define HPVT_LOG_LEVEL_DEFAULT				LOG_LEVEL_NOTICE

#define HPVT_PATH_MAX_LENGTH					(1024)
#define HPVT_PATH_BUFFER_SIZE					(HPVT_PATH_MAX_LENGTH + 1)

#define HPVT_CONFIG_FILE_DEFAULT_PATH					"./hpvt.conf"

#define HPVT_FRAME_LENGTH_MAXIMUM          (1048576)
#define HPVT_FRAME_QUEUE_SIZE  	           (128)

typedef uint16_t HPVT_Queue_FRAME_SEQNO;
#define HPVT_Queue_FRAME_SEQNO_MINIMUM     (1)
#define HPVT_Queue_FRAME_SEQNO_MAXIMUM     (60000)
#define HPVT_Queue_FRAME_SEQNO_INVALID     (0)

typedef uint8_t HPVT_Queue_FRAME_TYPE;
#define HPVT_Queue_FRAME_TYPE_I            (0x01)
#define HPVT_Queue_FRAME_TYPE_P            (0x02)
#define HPVT_Queue_FRAME_TYPE_NONE         (0x00)

#define BUFFERING_I_FRAME_MAXIMUM               (3)
#define BUFFERING_TOTAL_FRAME_MAXIMUM           (5)

typedef uint16_t HPVT_Queue_PACKET_SEQNO;
#define HPVT_Queue_PACKET_SEQNO_MIN        (1)
#define HPVT_Queue_PACKET_SEQNO_MAX        (2047)
#define HPVT_Queue_PACKET_SEQNO_INVALID    (0)

#define HPVT_PACKET_SET_PACKET_ARRAY_SIZE	    (2048)

#define HPVT_PACKET_RECEIVE_QUEUE_SIZE	        (10000)

#define HPVT_PACKET_IP_UDP_HEADER_LENGTH      (28)
#define HPVT_PACKET_HEADER_LENGTH             (29)
#define HPVT_PACKET_SCS_HEADER_LENGTH_MAXIMUM  (22)
#define HPVT_PACKET_PAYLOAD_LENGTH_DEFAULT    (1200)
#define HPVT_PACKET_MTU_MAXIMUM               (1500)

#define HPVT_PACKET_PADDING_ON                (0x01)
#define HPVT_PACKET_PADDING_OFF               (0x00)

#define HPVT_PACKET_TYPE_NORMAL               (1)
#define HPVT_PACKET_TYPE_PATH_MTU_DISCOVERY   (2)
#define HPVT_PACKET_TYPE_INTERNAL_INFO        (3)

#define HPVT_TELEMETRY_KEY_CODE_FRAME_GENERATED_TIME        (0x81)
#define HPVT_TELEMETRY_KEY_CODE_FUNCTION_FLAGS           	(0x82)
#define HPVT_TELEMETRY_KEY_CODE_ADAPTIVE_CONTROL_STATUS     (0x84)
#define HPVT_TELEMETRY_FUNCTION_FLAG_VIDEO_ADAPTIVE_ON		(0x10)

#endif /* COMMON_S_H_ */
