#define FILE_NUMBER "Q222"

#include <algorithm>

#include "HPVTVideoFrameQueue.h"

HPVTVideoFrameQueue::HPVTVideoFrameQueue() {
	if (pthread_mutex_init(&queue_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	full_list_flag = false;
	i_frame_count = 0;
	p_frame_count = 0;
}
HPVTVideoFrameQueue::~HPVTVideoFrameQueue() {

	if (pthread_mutex_destroy(&queue_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	full_list_flag = false;
	i_frame_count = 0;
	p_frame_count = 0;
}
int HPVTVideoFrameQueue::enqueue(HPVTVideoFrame * frame) {

	if (frame->getLength() > HPVT_FRAME_LENGTH_MAXIMUM) {
		HPVT_logging(LOG_LEVEL_ERROR, "!");
		return -1;
	}

	if (isFull()) {

		if (full_list_flag == false) {
			HPVT_logging(LOG_LEVEL_INFO, "Frame queue is full");
		}
		full_list_flag = true;

		return -2;
	}

	HPVT_Lock(queue_locked);

	HPVT_VIDEO_FRAME_ENTRY tmp_st;

	tmp_st.frame = frame;
	tmp_st.deleted = false;
	video_frame_queue.push_back(tmp_st);
	if (frame->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
		i_frame_count++;
	}
	else {
		p_frame_count++;
	}
	HPVT_Unlock(queue_locked);

	full_list_flag = false;

	return 0;
}
HPVTVideoFrame *HPVTVideoFrameQueue::dequeue(void) {

	if (isEmpty()) {

		HPVT_logging(LOG_LEVEL_INFO, "Frame queue is empty");
		return NULL;
	}

	HPVT_Lock(queue_locked);

	HPVTVideoFrame *data;

	data = video_frame_queue.front().frame;

	if (data->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
		i_frame_count--;
	}
	else {
		p_frame_count--;
	}

	video_frame_queue.erase(video_frame_queue.begin());

	HPVT_Unlock(queue_locked);

	full_list_flag = false;

	return data;
}
boolean HPVTVideoFrameQueue::isEmpty(void) {

	return video_frame_queue.empty();

}
boolean HPVTVideoFrameQueue::isFull(void) {

	if (video_frame_queue.size() == HPVT_FRAME_QUEUE_SIZE) {
		return true;
	}
	else {
		return false;
	}

}
static boolean HPVT_is_deleted_seqno(HPVT_VIDEO_FRAME_ENTRY item) {

	return item.deleted;
}
int HPVTVideoFrameQueue::cut(int num) {

	int skip_count = 0;
	HPVT_Lock(queue_locked);

	for (int i = 0; i < video_frame_queue.size(); i++) {

		if (video_frame_queue[i].frame->getFrameType() == HPVT_Queue_FRAME_TYPE_P) {

			video_frame_queue[i].deleted = true;
			delete video_frame_queue[i].frame;
			skip_count++;
		}
	}

	video_frame_queue.erase(remove_if(video_frame_queue.begin(), video_frame_queue.end(), HPVT_is_deleted_seqno), video_frame_queue.end());

	HPVT_Unlock(queue_locked);

	return skip_count;
}
int HPVTVideoFrameQueue::getFrameCount() {
	return video_frame_queue.size();
}
