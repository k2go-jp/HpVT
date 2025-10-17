#ifndef NETWORK_QUEUE_HPVTVIDEOFRAMESEQUENCEMANAGER_H_
#define NETWORK_QUEUE_HPVTVIDEOFRAMESEQUENCEMANAGER_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

class HPVTVideoFrameSequenceManager {

	pthread_mutex_t frame_locked;
	pthread_cond_t frame_cond;

	HPVT_Queue_FRAME_SEQNO processing_seqno;

public:
	HPVTVideoFrameSequenceManager();
	virtual ~HPVTVideoFrameSequenceManager();

	boolean condition_wait();
	boolean signal();
	boolean lock();
	boolean unlock();

	int compare(HPVT_Queue_FRAME_SEQNO);
	void incrementSeqno();
	void reset();

	HPVT_Queue_FRAME_SEQNO getProcessingSeqno() const {
		return processing_seqno;
	}

	void setProcessingSeqno(HPVT_Queue_FRAME_SEQNO seqno) {
		processing_seqno = seqno;
	}
};

#endif /* NETWORK_QUEUE_HPVTVIDEOFRAMESEQUENCEMANAGER_H_ */
