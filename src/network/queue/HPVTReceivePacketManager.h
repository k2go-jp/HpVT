#ifndef NETWORK_QUEUE_HPVTRECEIVEPACKETMANAGER_H_
#define NETWORK_QUEUE_HPVTRECEIVEPACKETMANAGER_H_

#include <vector>

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTPacket.h"

typedef struct stock_packet_structure {
	HPVTPacket *packet;
	boolean deleted;
	HPVT_Queue_FRAME_SEQNO seqno;
	boolean is_fec;
	boolean is_parity;

} HPVT_STOCK_PACKET_ENTRY;

class HPVTReceivePacketManager {

	std::vector<HPVT_STOCK_PACKET_ENTRY> packet_list;
	pthread_mutex_t list_locked;
	boolean full_list_flag;

public:
	HPVTReceivePacketManager();
	~HPVTReceivePacketManager();

	boolean lock();
	boolean unlock();

	boolean isEmpty();
	void clear();

	boolean add(HPVTPacket *, HPVT_Queue_FRAME_SEQNO, boolean, boolean);
	int getConsumptionRate();
	int getPacketListLength();
	HPVT_Queue_FRAME_SEQNO getOldestIFrameSeqno();
	HPVT_Queue_FRAME_SEQNO getFirstFrameSeqno(HPVT_Queue_FRAME_SEQNO);
	void eraseDeletedEntries();

	std::vector<HPVT_STOCK_PACKET_ENTRY>& getPacketList(){
		return packet_list;
	}

	boolean isFullListFlag() const {
		return full_list_flag;
	}

	void setFullListFlag(boolean fullListFlag) {
		full_list_flag = fullListFlag;
	}
};

#endif /* NETWORK_QUEUE_HPVTRECEIVEPACKETMANAGER_H_ */
