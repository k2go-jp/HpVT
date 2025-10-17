#define FILE_NUMBER "Q213"

#include <algorithm>

#include "HPVTPacketSetQueue.h"

HPVTPacketSetQueue::HPVTPacketSetQueue() {

	if (pthread_mutex_init(&queue_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	i_frame_count = 0;
	p_frame_count = 0;
	dropped_i_frame_count = 0;
	dropped_p_frame_count = 0;
}
HPVTPacketSetQueue::~HPVTPacketSetQueue() {

	if (pthread_mutex_destroy(&queue_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
int HPVTPacketSetQueue::enqueue(HPVTPacketSet * packetSet) {

	HPVT_Lock(queue_locked);

	if (isFull()) {
		HPVT_Unlock(queue_locked);
		if (packetSet->getFrameSeqno() % 256 == 1) {
			HPVT_logging(LOG_LEVEL_NOTICE, "PacketSet queue is full");
		}
		return -2;
	}

	HPVT_PACKET_SET_ENTRY tmp_st;

	tmp_st.packet_set = packetSet;
	tmp_st.deleted = false;
	packet_set_queue.push_back(tmp_st);

	if (packetSet->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
		i_frame_count++;
	}
	else {
		p_frame_count++;
	}

	//

	HPVT_Unlock(queue_locked);
	return 0;
}
HPVTPacketSet *HPVTPacketSetQueue::dequeue(void) {

	HPVT_Lock(queue_locked);

	if (isEmpty()) {
		HPVT_Unlock(queue_locked);
		return NULL;
	}

	HPVTPacketSet *data;

	data = (HPVTPacketSet *) packet_set_queue.front().packet_set;

	if (data->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
		i_frame_count--;
	}
	else {
		p_frame_count--;
	}

	packet_set_queue.erase(packet_set_queue.begin());

	HPVT_Unlock(queue_locked);
	return data;
}
void HPVTPacketSetQueue::clear(void) {

	HPVT_Lock(queue_locked);

	while (!isEmpty()) {

		delete packet_set_queue.front().packet_set;
		packet_set_queue.erase(packet_set_queue.begin());

	}
	i_frame_count = 0;
	p_frame_count = 0;
	HPVT_Unlock(queue_locked);
}
boolean HPVTPacketSetQueue::isEmpty(void) {

	return packet_set_queue.empty();
}
boolean HPVTPacketSetQueue::isFull(void) {

	if (packet_set_queue.size() == HPVT_FRAME_QUEUE_SIZE) {
		return true;
	}
	else {
		return false;
	}
}
boolean HPVTPacketSetQueue::isOverLimitIFrames(void) {

	if (i_frame_count > BUFFERING_I_FRAME_MAXIMUM) {
		return true;
	}
	else {
		return false;
	}
}
boolean HPVTPacketSetQueue::isOverMaxFrames(int fps) {

	if ((i_frame_count + p_frame_count) > (BUFFERING_TOTAL_FRAME_MAXIMUM * fps)) {
		return true;
	}
	else {
		return false;
	}
}
static boolean HPVT_is_deleted_seqno(HPVT_PACKET_SET_ENTRY item) {

	return item.deleted;

}
boolean HPVTPacketSetQueue::cutOldFrames() {

	int tmp_i_frame_total = 0;
	int tmp_skip_i_count = 0;
	int tmp_skip_p_count = 0;
	int queue_length;

	HPVT_Lock(queue_locked);

	queue_length = packet_set_queue.size();

	for (int i = 0; i < queue_length; i++) {

		if (packet_set_queue.at(i).packet_set->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
			tmp_i_frame_total++;

			if ((i_frame_count - tmp_i_frame_total) < 0) {
				break;
			}

			tmp_skip_i_count++;
		}
		else {
			tmp_skip_p_count++;
		}

		packet_set_queue.at(i).deleted = true;
		delete packet_set_queue.at(i).packet_set;
	}

	packet_set_queue.erase(remove_if(packet_set_queue.begin(), packet_set_queue.end(), HPVT_is_deleted_seqno), packet_set_queue.end());

	dropped_i_frame_count += tmp_skip_i_count;
	dropped_p_frame_count += tmp_skip_p_count;
	i_frame_count -= tmp_skip_i_count;
	p_frame_count -= tmp_skip_p_count;

	if (tmp_skip_i_count > 0) {
		HPVT_logging(LOG_LEVEL_WARNING, "%d I Frames are eliminated because the packet queue is over limit", tmp_skip_i_count);
	}

	if (tmp_skip_p_count > 0) {
		HPVT_logging(LOG_LEVEL_WARNING, "%d P Frames are eliminated because the packet queue is over limit", tmp_skip_p_count);
	}

	HPVT_Unlock(queue_locked);

	return true;
}
boolean HPVTPacketSetQueue::cutPFrames(int num) {

	if(num == 0)
	{
		return true;
	}

	int tmp_i_frame_total = 0;
	int tmp_skip_i_count = 0;
	int tmp_skip_p_count = 0;
	int queue_length;

	HPVT_Lock(queue_locked);

	queue_length = packet_set_queue.size();

	for (int i = 0; i < queue_length; i++) {

		if (packet_set_queue.at(i).packet_set->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {

			if (tmp_skip_p_count >= num) {
				break;
			}

			continue;

		}

		tmp_skip_p_count++;

		packet_set_queue.at(i).deleted = true;
		delete packet_set_queue.at(i).packet_set;
	}

	packet_set_queue.erase(remove_if(packet_set_queue.begin(), packet_set_queue.end(), HPVT_is_deleted_seqno), packet_set_queue.end());
	dropped_p_frame_count += tmp_skip_p_count;
	p_frame_count -= tmp_skip_p_count;

	if (tmp_skip_p_count > 0) {
		HPVT_logging(LOG_LEVEL_WARNING, "%d P Frames are eliminated because the packet queue is over maximum", tmp_skip_p_count);
	}

	HPVT_Unlock(queue_locked);

	return true;
}

