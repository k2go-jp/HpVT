#ifndef NETWORK_QUEUE_HPVTVIDEOFRAMEBUILDER_H_
#define NETWORK_QUEUE_HPVTVIDEOFRAMEBUILDER_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTVideoFrame.h"

class HPVTVideoFrameBuilder {

	 int frame_index;
	 HPVTVideoFrame *frame_table[2];
	 HPVTVideoFrame *current_frame;

	 pthread_mutex_t frame_locked;

public:

	void changeFrame();
	boolean update(HPVTPacket *);

	HPVTVideoFrameBuilder();
	virtual ~HPVTVideoFrameBuilder();

	HPVTVideoFrame* getCurrentFrame() const {
		return current_frame;
	}
};

#endif /* NETWORK_QUEUE_HPVTVIDEOFRAMEBUILDER_H_ */
