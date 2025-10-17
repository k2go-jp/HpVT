#define FILE_NUMBER "Q214"

#include <algorithm>

#include "HPVTPacket.h"
#include "HPVTReceivePacketManager.h"

#include "queue.h"

HPVTReceivePacketManager::HPVTReceivePacketManager() {

	if (pthread_mutex_init(&list_locked, 0)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
	full_list_flag = false;
}
HPVTReceivePacketManager::~HPVTReceivePacketManager() {

	if (pthread_mutex_destroy(&list_locked)) {
		HPVT_log_UNEXPECTED_ERROR();
	}
}
boolean HPVTReceivePacketManager::lock() {
	HPVT_Lock(list_locked);
	return true;
}
boolean HPVTReceivePacketManager::unlock() {
	HPVT_Unlock(list_locked);
	return true;
}
boolean HPVTReceivePacketManager::isEmpty() {

	return packet_list.empty();

}
static boolean _HPVT_is_deleted_seqno(HPVT_STOCK_PACKET_ENTRY item) {

	return item.deleted;
}
void HPVTReceivePacketManager::eraseDeletedEntries() {

	packet_list.erase(remove_if(packet_list.begin(), packet_list.end(), _HPVT_is_deleted_seqno), packet_list.end());
}
void HPVTReceivePacketManager::clear() {
	HPVT_Lock(list_locked);

	for (int i = 0; i < packet_list.size(); i++) {

		delete packet_list[i].packet;
		 packet_list[i].deleted = true;
	}

	eraseDeletedEntries();

	HPVT_Unlock(list_locked);
	full_list_flag = false;
}
int HPVTReceivePacketManager::getPacketListLength() {

	return packet_list.size();
}
boolean HPVTReceivePacketManager::add(HPVTPacket *packet, HPVT_Queue_FRAME_SEQNO seqno, boolean is_fec, boolean is_parity) {

	HPVT_Lock(list_locked);
	if (packet_list.size() >= HPVT_PACKET_RECEIVE_QUEUE_SIZE) {
		HPVT_Unlock(list_locked);

		if (full_list_flag == false) {
			HPVT_logging(LOG_LEVEL_WARNING, "packet queue is full!");
		}
		full_list_flag = true;
		return false;
	}

	full_list_flag = false;
	HPVT_STOCK_PACKET_ENTRY tmp_list_entry;
	tmp_list_entry.packet = packet;
	tmp_list_entry.seqno = seqno;
	tmp_list_entry.is_fec = is_fec;
	tmp_list_entry.is_parity = is_parity;
	tmp_list_entry.deleted = false;

	packet_list.push_back(tmp_list_entry);
	HPVT_Unlock(list_locked);

	return true;
}
int HPVTReceivePacketManager::getConsumptionRate() {

	int tmp_rate;
	HPVT_Lock(list_locked);

	tmp_rate = 100 * packet_list.size() / HPVT_PACKET_RECEIVE_QUEUE_SIZE;

	HPVT_Unlock(list_locked);

	return tmp_rate;
}
HPVT_Queue_FRAME_SEQNO HPVTReceivePacketManager::getOldestIFrameSeqno() {

	HPVT_Lock(list_locked);

	HPVT_Queue_FRAME_SEQNO tmp_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	int list_length;
	list_length = packet_list.size();

	for (int i = 0; i < list_length; i++) {
		HPVTPacket *tmp_stocked_packet = packet_list[i].packet;

		if (tmp_stocked_packet == NULL) {
			HPVT_logging(LOG_LEVEL_ALERT, "!");
			continue;
		}

		if (tmp_stocked_packet->getFrameType() == HPVT_Queue_FRAME_TYPE_I) {
			tmp_seqno = tmp_stocked_packet->getFrameSeqno();
			break;
		}
	}

	HPVT_Unlock(list_locked);

	return tmp_seqno;
}
HPVT_Queue_FRAME_SEQNO HPVTReceivePacketManager::getFirstFrameSeqno(HPVT_Queue_FRAME_SEQNO comparison_seqno) {

	HPVT_Lock(list_locked);

	HPVT_Queue_FRAME_SEQNO tmp_seqno = HPVT_Queue_FRAME_SEQNO_INVALID;
	int list_length;
	list_length = packet_list.size();
	int oldest_result = 0;
	int tmp_result = 0;

	for (int i = 0; i < list_length; i++) {
		HPVTPacket *tmp_stocked_packet = packet_list[i].packet;

		if (tmp_stocked_packet == NULL) {
			HPVT_logging(LOG_LEVEL_ALERT, "!");
			continue;
		}

		tmp_result = HPVT_compare_frame_sequence_number(comparison_seqno, tmp_stocked_packet->getFrameSeqno());

		if (i == 0 || tmp_result < oldest_result) {
			oldest_result = tmp_result;
			tmp_seqno = tmp_stocked_packet->getFrameSeqno();
		}
	}

	HPVT_Unlock(list_locked);

	return tmp_seqno;
}
