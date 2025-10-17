#define FILE_NUMBER "M001"

#include <signal.h>

#include "config/config.h"
#include "network/socket.h"
#include "network/context.h"
#include "main.h"
#include "scs/api.h"

typedef struct HPVT_Arguments_s HPVT_Arguments;
struct HPVT_Arguments_s {
	char * config_path;
};

HPVT_Context *g_context;
SCSSocketId g_scssock = SCS_SKTID_INVVAL;
SCSCallbackId g_callback_id = SCS_CALLBACKID_INVVAL;

static void HPVT_signal_handler() {
	HPVT_logging(LOG_LEVEL_NOTICE, "received signal")

	if (g_context != NULL) {
		g_context->termination = true;
	}

	syslog(LOG_LEVEL_NOTICE, "Exiting ...");
}
static int HPVT_set_sigaction(void) {

	int retval;
	struct sigaction sa;
	sa.sa_handler = (__sighandler_t) HPVT_signal_handler;
	sa.sa_flags = SA_RESTART;

	retval = sigemptyset(&sa.sa_mask);
	if (retval == -1) {
		return -1;
	}

	retval = sigaction(SIGINT, &sa, 0);
	if (retval == -1) {
		return -1;
	}

	retval = sigaction(SIGTERM, &sa, 0);
	if (retval == -1) {
		return -1;
	}

	retval = sigaction(SIGTSTP, &sa, 0);
	if (retval == -1) {
		return -1;
	}

	return 0;
}
static void HPVT_reset_sigaction(void) {
	signal(SIGINT, SIG_DFL);
}
static inline void _HPVT_Logger_setup(HPVT_Config * config,	HPVT_Arguments * arguments) {
	HPVT_Logger_teardown();
	usleep(500000);
	HPVT_Logger_setup(PROCESS_NAME, config->log.facility, config->log.level);
}
static inline void _HPVT_Logger_teardown(void) {
	HPVT_Logger_teardown();
}
static void _HPVT_Arguments_initialize(HPVT_Arguments * object) {
	memset(object, 0, sizeof(HPVT_Arguments));
}
static void __HPVT_Arguments_parse(HPVT_Arguments * object, int argc, const char * argv[], int * index, char ** out) {
	int tmp_index;
	const char * tmp_argv;
	char * tmp_string;
	int i;

	tmp_index = *index;

	if (argc <= ++tmp_index) {
		return;
	}

	tmp_argv = argv[tmp_index];

	if (tmp_argv[0] == '-') {
		return;
	}

	if ((tmp_string = strdup(tmp_argv)) == NULL) {
		return;
	}

	for (i = (strlen(tmp_string) - 1); i <= 0; i--) {
		if (tmp_string[i] == '/') {
			tmp_string[i] = '\0';
		} else {
			break;
		}
	}

	*index = tmp_index;
	*out = tmp_string;
}
static void _HPVT_Arguments_parse(HPVT_Arguments * object, int argc, const char * argv[]) {

	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], ARGS_CONFIG_KEY_S) == 0 || strcmp(argv[i], ARGS_CONFIG_KEY_L) == 0) {
			__HPVT_Arguments_parse(object, argc, argv, &i, &object->config_path);
			continue;
		}

		printf("Argument error <<%s>>\n", argv[i]);
		syslog(LOG_LEVEL_NOTICE, "Argument error <<%s>>\n", argv[i]);
		exit(EXIT_FAILURE);
	}
}
static void _HPVT_Arguments_display(HPVT_Arguments * object) {
	syslog(LOG_LEVEL_NOTICE, "### Arguments\n");
	syslog(LOG_LEVEL_NOTICE, "# Config File : %s\n", object->config_path != NULL ? object->config_path : "none");
	syslog(LOG_LEVEL_NOTICE, "#");
}
static boolean _HPVT_loadConfig(HPVT_Config * config, HPVT_Arguments * arguments, int particular) {

	const char * tmp_path;
	if ((tmp_path = arguments->config_path) != NULL) {
		tmp_path = arguments->config_path;
	} else {
		tmp_path = HPVT_CONFIG_FILE_DEFAULT_PATH;
	}

	if (particular == HPVT_Config_LOAD_REGULAR) {
		printf("Load config ... <<%s>>\n", tmp_path);
	}

	syslog(LOG_LEVEL_NOTICE, "Load config ... <<%s>> mode=%d", tmp_path, particular);

	if (HPVT_Config_load(config, tmp_path, particular) == false) {
		printf("Could not load a config file <<%s>>\n", tmp_path);
		syslog(LOG_LEVEL_ERROR, "Could not load a config file <<%s>>",
				tmp_path);
		return false;
	}

	if (HPVT_Config_fix(config, g_context) == false) {
		return false;
	}

	if (particular == HPVT_Config_LOAD_REGULAR) {

		HPVT_Config_logging(config);

		if (HPVT_Config_verify(config, g_context) == false) {
			printf("HPVT_Config_verify failed\n");
			return false;
		}
	}

	return true;
}
static void _HPVT_displayVersion() {

	char edition[] = "";
	printf("%s Ver.%d.%d.%d(build %s)\n", PROCESS_NAME, HPVT_VERSION_MAJOR,
			HPVT_VERSION_MINOR, HPVT_VERSION_REVISION, PROCESS_BUILD);
}
static void _HPVT_displayUsage() {
	printf("Usage: %s -h\n", PROCESS_NAME);
	printf("       %s -v\n", PROCESS_NAME);
	printf("       %s [-c <config-file-path>]\n", PROCESS_NAME);
	printf("\n");
}
static boolean _HPVT_isUsage(int argc, const char *argv[]) {
	int i;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], ARGS_HELP_KEY_S) == 0 || strcmp(argv[i], ARGS_HELP_KEY_L) == 0) {
			return true;
		}
	}

	return false;
}
static boolean _HPVT_isVersion(int argc, const char *argv[]) {
	int i;

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], ARGS_VERSION_KEY_S) == 0 || strcmp(argv[i], ARGS_VERSION_KEY_L) == 0) {
			return true;
		}
	}

	return false;
}
static boolean HPVT_resolve_absolute_path(char *path, char **resolved_path) {

	char temp_buffer[4096];
	memset(temp_buffer, 0, sizeof(temp_buffer));

	if (realpath(path, temp_buffer) == NULL) {
		*resolved_path = NULL;
		return false;
	} else {
		int length;
		length = strlen(temp_buffer);

		if (length < HPVT_PATH_BUFFER_SIZE) {
			*resolved_path = (char *) calloc(1, length + 1);
			memcpy(*resolved_path, temp_buffer, length);
		} else {
			*resolved_path = NULL;
			return false;
		}
	}

	return true;
}
int main(int argc, const char **argv) {

	HPVT_set_sigaction();

	char *tmp_ptr;
	tmp_ptr = (char *) strrchr(argv[0], '/');
	if (tmp_ptr == NULL) {
		tmp_ptr = (char *) argv[0];
	} else {
		tmp_ptr = tmp_ptr + 1;
	}

	HPVT_Arguments tmp_arguments;
	HPVT_Config *tmp_config_for_logging;
	HPVT_Config *tmp_config;
	HPVT_Config *tmp_override_config = NULL;

	if (_HPVT_isUsage(argc, argv) == true) {
		_HPVT_displayUsage();
		return EXIT_SUCCESS;
	}

	if (_HPVT_isVersion(argc, argv) == true) {
		_HPVT_displayVersion();
		return EXIT_SUCCESS;
	}

	_HPVT_Arguments_initialize(&tmp_arguments);
	_HPVT_Arguments_parse(&tmp_arguments, argc, argv);
	_HPVT_Arguments_display(&tmp_arguments);

	g_context = (HPVT_Context*) malloc(sizeof(HPVT_Context));
	HPVT_Context_initialize(g_context);

	tmp_config_for_logging = HPVT_Config_create();

	if (_HPVT_loadConfig(tmp_config_for_logging, &tmp_arguments,
			HPVT_Config_LOAD_LOGGING) == false) {
		return EXIT_FAILURE;
	}

	HPVT_Logger_setup(PROCESS_NAME, tmp_config_for_logging->log.facility,
			tmp_config_for_logging->log.level);

	HPVT_Config_delete(tmp_config_for_logging);

	printf("Started %s v.%d.%d.%d(build %s)\n", PROCESS_NAME, HPVT_VERSION_MAJOR,
			HPVT_VERSION_MINOR, HPVT_VERSION_REVISION, PROCESS_BUILD);
	syslog(LOG_LEVEL_NOTICE, "Started %s v.%d.%d.%d(build %s)\n", PROCESS_NAME,
			HPVT_VERSION_MAJOR, HPVT_VERSION_MINOR, HPVT_VERSION_REVISION, PROCESS_BUILD);
	setvbuf(stdout, 0, _IONBF, 0);

	tmp_config = HPVT_Config_create();

	if (_HPVT_loadConfig(tmp_config, &tmp_arguments, HPVT_Config_LOAD_REGULAR) == false) {
		return EXIT_FAILURE;
	}

	_HPVT_Logger_setup(tmp_config, &tmp_arguments);

	if (HPVT_resolve_absolute_path(tmp_config->config_file, &g_context->config_path) == false) {
		HPVT_logging(LOG_LEVEL_ERROR, "Could not find config file(%s)", tmp_config->config_file);
		return EXIT_FAILURE;
	}

	HPVT_Context_setup(g_context, tmp_config);

	HPVT_SCS_initialize (NULL);

	switch (tmp_config->connection.type) {
		case HPVT_Config_TYPE_TRANSMITTER:
			HPVT_main_server(tmp_config);
			break;
		case HPVT_Config_TYPE_RECEIVER:
			HPVT_main_client(tmp_config);
			break;
		default:
			HPVT_log_UNEXPECTED_ERROR();
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "exited the main loop");

	HPVT_Config_dispose(tmp_config);

	syslog(LOG_LEVEL_NOTICE, "Terminated %s v.%d.%d.%d(build %s)\n", PROCESS_NAME,
			HPVT_VERSION_MAJOR, HPVT_VERSION_MINOR, HPVT_VERSION_REVISION, PROCESS_BUILD);

	HPVT_Context_destroy(g_context);

	_HPVT_Logger_teardown();

	HPVT_reset_sigaction();

	fprintf(stdout, "\nExited\n");

	return EXIT_SUCCESS;
}
