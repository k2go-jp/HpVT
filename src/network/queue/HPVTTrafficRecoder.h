#ifndef NETWORK_QUEUE_HPVTTRAFFICRECODER_H_
#define NETWORK_QUEUE_HPVTTRAFFICRECODER_H_

#include "../../common_s.h"
#include "../../logger/logger.h"

#define TRAFFIC_RECODER_ARRAY_SIZE    600

class HPVTTrafficRecoder {

	int feedback_recv_plr_index;
	uint32_t feedback_recv_plr[TRAFFIC_RECODER_ARRAY_SIZE];

	int feedback_recv_throughput_index;
	uint64_t feedback_recv_throughput_array[TRAFFIC_RECODER_ARRAY_SIZE];

	int send_throughput_index;
	uint64_t send_throughput_array[TRAFFIC_RECODER_ARRAY_SIZE];

public:
	HPVTTrafficRecoder();
	~HPVTTrafficRecoder();

	void addLatestPLR(uint32_t);
	void addLatestRecvThroughput(uint64_t);
	void addLatestSendThroughput(uint32_t);

	uint32_t getAveragePLR(int);
	uint64_t getAverageRecvThroughput(int);
	uint32_t getAverageSendThroughput(int);

	uint32_t getLatestPLR(void);
	uint64_t getLatestRecvThroughput(void);
	uint64_t getLatestSendThroughput(void);

	void clearPLR();
	void clearThroughput();
};

#endif /* NETWORK_QUEUE_HPVTTRAFFICRECODER_H_ */
