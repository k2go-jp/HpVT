#ifndef SCS_DEBUG_H_
#define SCS_DEBUG_H_ 1

/* ============================================================================================== */

#include <stdio.h>
#include <string.h>

/* ---------------------------------------------------------------------------------------------- */

#ifndef SCS_ENABLE_DEBUG
#define SCS_ENABLE_DEBUG	0
#endif

/* ---------------------------------------------------------------------------------------------- */

#if SCS_ENABLE_DEBUG == 1
#define SCS_PRINT_DEBUG(xxx_format, xxx_args...) \
		printf("[%-16s:%5d]"xxx_format"\n", (strrchr((__FILE__), '/') + 1), __LINE__, ## xxx_args)
#else
#define SCS_PRINT_DEBUG(xxx_format, xxx_args...)
#endif

#if SCS_ENABLE_DEBUG == 1
#define SCS_HEXDUMP(xxx_value, xxx_length, xxx_width) \
		{\
			size_t xxx_i;\
			printf("[%-48s:%5d] %zu bytes\n", (strrchr((__FILE__), '/') + 1), __LINE__, xxx_length);\
			for (xxx_i = 0; xxx_i < xxx_length; xxx_i++) {\
				if (((xxx_i + 1) % xxx_width) != 0) {\
					printf(" %02x", ((uint8_t *) xxx_value)[xxx_i]);\
				}\
				else {\
					printf(" %02x\n", ((uint8_t *) xxx_value)[xxx_i]);\
				}\
			}\
			printf("\n");\
		}
#else
#define SCS_HEXDUMP(xxx_value, xxx_length, xxx_width)
#endif

/* ============================================================================================== */

#endif /* SCS_DEBUG_H_ */
