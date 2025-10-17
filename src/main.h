#ifndef MAIN_H_
#define MAIN_H_

#include "config/config_s.h"

#define PROCESS_NAME			"hpvt"
#define PROCESS_BUILD           "20250617"

#define ARGS_HELP_KEY_S			"-h"
#define ARGS_HELP_KEY_L			"--help"
#define ARGS_VERSION_KEY_S		"-v"
#define ARGS_VERSION_KEY_L		"--version"

#define ARGS_CONFIG_KEY_S		"-c"
#define ARGS_CONFIG_KEY_L		"--config"

int HPVT_main_client(HPVT_Config *config);
int HPVT_main_server(HPVT_Config *config);

#endif /* MAIN_H_ */
