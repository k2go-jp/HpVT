#define FILE_NUMBER "Q224"

#include "HPVTVideoFrameSequenceManager.h"

#include "queue.h"

HPVTVideoFrameSequenceManager::HPVTVideoFrameSequenceManager() {

	processing_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;


	if (pthread_mutex_init(&frame_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_cond_init(&frame_cond, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
HPVTVideoFrameSequenceManager::~HPVTVideoFrameSequenceManager() {

	if (pthread_mutex_destroy(&frame_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (pthread_cond_destroy(&frame_cond)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
boolean HPVTVideoFrameSequenceManager::condition_wait() {

	if (pthread_cond_wait(&frame_cond, &frame_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	return true;
}
boolean HPVTVideoFrameSequenceManager::signal() {

	if (pthread_cond_signal(&frame_cond)) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	return true;
}
boolean HPVTVideoFrameSequenceManager::lock() {
	HPVT_Lock(frame_locked);
	return true;
}
boolean HPVTVideoFrameSequenceManager::unlock() {
	HPVT_Unlock(frame_locked);
	return true;
}
int HPVTVideoFrameSequenceManager::compare(HPVT_Queue_FRAME_SEQNO seqno) {

	return HPVT_compare_frame_sequence_number(processing_seqno, seqno);
}
void HPVTVideoFrameSequenceManager::incrementSeqno() {

	if (processing_seqno < HPVT_Queue_FRAME_SEQNO_MAXIMUM) {
		processing_seqno++;
	}
	else {
		processing_seqno = HPVT_Queue_FRAME_SEQNO_MINIMUM;
	}
}
void HPVTVideoFrameSequenceManager::reset() {
	processing_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	HPVT_logging(LOG_LEVEL_NOTICE, "DEC initialized seqno");
}
