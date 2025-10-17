#ifndef SCS_5_DEFINES_H_
#define SCS_5_DEFINES_H_ 1

/* ============================================================================================== */

#include <limits.h>

/* ---------------------------------------------------------------------------------------------- */

#define SCS_IFNAMESIZE_MAX				(32)

#define SCS_MEMORY_MINSIZE				(1 * 1000 * 1000) 					// bytes

#define SCS_SOCKET_MAXNUM				INT8_MAX							// sockets

#define SCS_PACKET_MAXPAYLOADSIZE		(64 * 1024)							// bytes
#define SCS_PACKET_MAXSIZE				(SCS_PACKET_MAXPAYLOADSIZE + 256)	// bytes
#define SCS_DESCRIPTOR_MAXVAL			UINT16_MAX							// descriptors
#define SCS_CHANNEL_MAXNUM				SCS_SOCKET_MAXNUM					// channels

#define SCS_RECEIVER_TIMEOUT			(1 * 1000)							// msec
#define SCS_MANAGER_INTERVAL			(50)								// msec

#define SCS_REDUNDANCY_MAXINTERVAL		(60)								// sec
#define SCS_REDUNDANCY_MAXTIMES			(99)								// times

#define SCS_PKTQUEUE_MINSIZE			(100)								// packets
#define SCS_PKTQUEUE_MAXSIZE			(99999)								// packets

#define SCS_SKTQUEUE_MINSIZE			(100)								// packets
#define SCS_SKTQUEUE_MAXSIZE			(99999)								// packets

#define SCS_PKTSEQUENCER_MAXENTRIES		(512)								// entries

#define SCS_CLOSEWAITQUEUE_SIZE			(1024)								// sockets

/* ============================================================================================== */

#endif /* SCS_5_DEFINES_H_ */
