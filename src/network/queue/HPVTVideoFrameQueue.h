#ifndef QUEUE_HPVTVIDEOFRAMEQUEUE_H_
#define QUEUE_HPVTVIDEOFRAMEQUEUE_H_

#include <vector>

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTVideoFrame.h"

typedef struct video_frame_structure {
	HPVTVideoFrame *frame;
	boolean deleted;
} HPVT_VIDEO_FRAME_ENTRY;

class HPVTVideoFrameQueue {

	pthread_mutex_t queue_locked;
	std::vector<HPVT_VIDEO_FRAME_ENTRY> video_frame_queue;
	boolean full_list_flag;
	uint16_t i_frame_count;
	uint16_t p_frame_count;

public:
	HPVTVideoFrameQueue();
	~HPVTVideoFrameQueue();

	int enqueue(HPVTVideoFrame *);
	HPVTVideoFrame *dequeue();
	boolean isEmpty();
	boolean isFull();
	int getFrameCount();
	int cut(int);

	uint16_t getIFrameCount() const {
		return i_frame_count;
	}

	uint16_t getPFrameCount() const {
		return p_frame_count;
	}
};

#endif /* QUEUE_HPVTVIDEOFRAMEQUEUE_H_ */
