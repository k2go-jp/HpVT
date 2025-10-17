#define FILE_NUMBER "Q223"

#include "HPVTVideoFrameBuilder.h"

HPVTVideoFrameBuilder::HPVTVideoFrameBuilder() {

	frame_index = 0;
	frame_table[0] = new HPVTVideoFrame();
	frame_table[1] = new HPVTVideoFrame();
	current_frame = frame_table[0];

	if (pthread_mutex_init(&frame_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
HPVTVideoFrameBuilder::~HPVTVideoFrameBuilder() {

	delete frame_table[0];
	delete frame_table[1];

	if (pthread_mutex_destroy(&frame_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
void HPVTVideoFrameBuilder::changeFrame() {

	HPVT_Lock(frame_locked);
	frame_index = (frame_index + 1) % 2;
	current_frame->reset();
	current_frame = frame_table[frame_index];
	HPVT_Unlock(frame_locked);
}
boolean HPVTVideoFrameBuilder::update(HPVTPacket* packet) {
	HPVT_Lock(frame_locked);

	boolean ret;
	ret = current_frame->setPacketizedData(packet);

	HPVT_Unlock(frame_locked);
	return ret;
}
