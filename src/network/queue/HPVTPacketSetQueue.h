#ifndef QUEUE_HPVTPACKETSETQUEUE_H_
#define QUEUE_HPVTPACKETSETQUEUE_H_

#include <vector>

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTPacketSet.h"

typedef struct packet_set_structure {
	HPVTPacketSet *packet_set;
	boolean deleted;
} HPVT_PACKET_SET_ENTRY;

class HPVTPacketSetQueue {

	pthread_mutex_t queue_locked;
	std::vector<HPVT_PACKET_SET_ENTRY> packet_set_queue;
	uint16_t i_frame_count;
	uint16_t p_frame_count;
	uint32_t dropped_i_frame_count;
	uint32_t dropped_p_frame_count;

public:

	HPVTPacketSetQueue();
	~HPVTPacketSetQueue();

	int enqueue(HPVTPacketSet *);
	HPVTPacketSet * dequeue(void);
	boolean isEmpty();
	boolean isFull();
	boolean isOverLimitIFrames();
	boolean isOverMaxFrames(int fps);
	void clear();

	boolean cutOldFrames();
	boolean cutPFrames(int);

	uint16_t getIFrameCount() const {
		return i_frame_count;
	}

	uint16_t getPFrameCount() const {
		return p_frame_count;
	}

	uint32_t getDroppedIFrameCount() const {
		return dropped_i_frame_count;
	}

	uint32_t getDroppedPFrameCount() const {
		return dropped_p_frame_count;
	}
};

#endif /* QUEUE_HPVTPACKETSETQUEUE_H_ */
