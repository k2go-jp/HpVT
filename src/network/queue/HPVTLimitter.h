#ifndef QUEUE_HPVTLIMITTER_H_
#define QUEUE_HPVTLIMITTER_H_

#include <sys/time.h>

#include "../../common_s.h"
#include "../../logger/logger.h"

#include "HPVTLimitter.h"

class HPVTLimitter {

	uint32_t sent_size;
	uint32_t total_byte;
	uint32_t target_rate;
	struct timeval begin_time;

public:
	HPVTLimitter(uint32_t, uint32_t);
	~HPVTLimitter();

	uint32_t getWaitTime(uint32_t);
};

#endif /* QUEUE_HPVTLIMITTER_H_ */
