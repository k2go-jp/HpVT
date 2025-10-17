#ifndef SCS_5_FEATURE_TRRAFIC_CONTROLLER_H_
#define SCS_5_FEATURE_TRRAFIC_CONTROLLER_H_ 1

/* ============================================================================================== */

#include <stddef.h>
#include <stdint.h>

#include "scs/1/comatibility.h"
#include "scs/1/stdbool.h"
#include "scs/1/time.h"
#include "scs/1/unistd.h"
#include "scs/2/atomic.h"
#include "scs/2/lock.h"

/* ---------------------------------------------------------------------------------------------- */

typedef uint64_t scs_bit_rate;

#define SCS_BITRATE_MIN		1
#define SCS_BITRATE_MAX		INT64_MAX

/* ---------------------------------------------------------------------------------------------- */

typedef int scs_frame_rate;

#define SCS_FRAMERATE_MIN		1
#define SCS_FRAMERATE_MAX		60

/* ---------------------------------------------------------------------------------------------- */

typedef int SCS_TCRETVAL;

#define SCS_TCRETVAL_NG			(-1)
#define SCS_TCRETVAL_OK			0
#define SCS_TCRETVAL_OVER		1
#define SCS_TCRETVAL_DELAY		2

/* ---------------------------------------------------------------------------------------------- */

typedef struct SCSVideoStreamTrafficController {
	bool available;
	struct {
		scs_bit_rate bit;
		scs_frame_rate frame;
	} rate;
	int divition;
	struct {
		scs_timespec frame;
		scs_timespec quantity;
	} interval;
	struct {
		uint64_t total;
		uint64_t frame;
		uint64_t quantity;
	} bytes;
	struct {
		uint64_t packets;
		struct {
			uint64_t limit;
			uint64_t total;
			uint64_t quantity;
		} bytes;
		struct {
			scs_frame_rate remain;
			scs_frame_rate ideal;
			scs_frame_rate actual;
		} frames;
		struct {
			scs_timespec base;
			scs_timespec dead;
			struct {
				scs_timespec frame;
				scs_timespec quantity;
			} next;
		} timestamp;
	} state;
} SCSVideoStreamTrafficController;

EXTERN void SCSVideoStreamTrafficControllerInitialize(SCSVideoStreamTrafficController * self);
EXTERN void SCSVideoStreamTrafficControllerFinalize(SCSVideoStreamTrafficController * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN SCS_TCRETVAL SCSVideoStreamTrafficControllerUpdate(										//
		SCSVideoStreamTrafficController * __restrict self, 										//
		size_t size,																			//
		scs_timespec * __restrict out);

//TODO convert to nanosleep
#define SCSVideoStreamTrafficControllerWait(xxx_self, xxx_period) \
		usleep((xxx_period).tv_nsec / 1000)

EXTERN SCS_TCRETVAL SCSVideoStreamTrafficControllerNextFrame(									//
		SCSVideoStreamTrafficController * __restrict self,										//
		scs_timespec * __restrict out);

EXTERN void SCSVideoStreamTrafficControllerReset(SCSVideoStreamTrafficController * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN bool SCSVideoStreamTrafficControllerSetBitRate(											//
		SCSVideoStreamTrafficController * self,													//
		scs_bit_rate bps);
EXTERN scs_bit_rate SCSVideoStreamTrafficControllerGetBitRate(SCSVideoStreamTrafficController * self);

EXTERN bool SCSVideoStreamTrafficControllerSetFrameRate(										//
		SCSVideoStreamTrafficController * self,													//
		scs_frame_rate fps);
EXTERN scs_frame_rate SCSVideoStreamTrafficControllerGetFrameRate(SCSVideoStreamTrafficController * self);

EXTERN size_t SCSVideoStreamTrafficControllerGetFrameSize(SCSVideoStreamTrafficController * self);

EXTERN size_t SCSVideoStreamTrafficControllerGetSendableFrames(SCSVideoStreamTrafficController * self);

/* ---------------------------------------------------------------------------------------------- */

EXTERN void SCSVideoStreamTrafficControllerDump(												//
		SCSVideoStreamTrafficController * __restrict self,										//
		__const char * __restrict prefix);

/* ============================================================================================== */

#endif /* SCS_5_FEATURE_TRRAFIC_CONTROLLER_H_ */
