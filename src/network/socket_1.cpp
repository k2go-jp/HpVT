#define FILE_NUMBER "N111"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "context.h"
#include "socket.h"

extern HPVT_Context *g_context;

static struct sockaddr_in s_peer;
static struct sockaddr_in tmp_peer_sockaddr;
static socklen_t sockaddr_in_len = sizeof(struct sockaddr_in);

void HPVT_SCS_initialize(char *path) {

	if (SCSStart(NULL) == false) {
		HPVT_log_UNEXPECTED_ERROR();
	}

	if (global_log_level == LOG_LEVEL_DEBUG) {
		SCSSetLogLevel(SCS_LOGTYPE_ALL, LOG_LEVEL_DEBUG);
	}
	else {
		SCSSetLogLevel(SCS_LOGTYPE_ALL, LOG_LEVEL_ERROR);
	}

	SCSOpenLogFile(path);
}
void HPVT_SCS_destroy(void) {

	SCSStop();
}
int HPVT_UDP_create_server_socket(uint16_t port) {

	int v;
	struct sockaddr_in sin = {
			AF_INET };
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(port);
	if (bind(sock, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
		perror("bind");
		return -1;
	}
	v = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*) &v, sizeof(v)) != 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "!");
		return -1;
	}

	v = 1024000;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (char*) &v, sizeof(v)) != 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "!");
		return -1;
	}

	struct timeval recv_tv;
	recv_tv.tv_sec = 5;
	recv_tv.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &recv_tv, sizeof(recv_tv)) != 0) {
		HPVT_logging(LOG_LEVEL_ALERT, "!");
		return -1;
	}

	return sock;

}
SCSSocketId HPVT_SCS_create_server_socket(uint16_t port) {

	while (g_context->connection.reconnection_attempt_on == false) {
		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CLOSED);
		sleep(1);
	}

	HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_DISCONNECTED);

	scs_sockaddr tmp_addr;
	struct sockaddr_in * tmp_sockaddr;
	SCSSocketId tmp_socket;
	SCSProtocolType protocol;

	protocol = SCS_PROTOCOLTYPE_UDP;

	tmp_sockaddr = (struct sockaddr_in *) &tmp_addr;
	tmp_sockaddr->sin_family = AF_INET;
	tmp_sockaddr->sin_addr.s_addr = INADDR_ANY;
	tmp_sockaddr->sin_port = htons(port);
	if ((tmp_socket = SCSCreateSocket(SCS_SKTTYPE_DATAGRAM, protocol)) == SCS_SKTID_INVVAL) {
		printf("Socket error.\n");
		return SCS_SKTID_INVVAL;
	}

	if (HPVT_Context_is_transmitter(g_context) == true) {
		if (HPVT_SCS_set_transmitter_hasty_data(tmp_socket) == false) {
			return SCS_SKTID_INVVAL;
		}
	}

	if (SCSListen(tmp_socket, tmp_addr) == false) {
		printf("Listen error.\n");
		return SCS_SKTID_INVVAL;
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "Listening.");
	printf("Listening.  (port %d)\n", port);

	HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_WAITING);

	if (SCSAccept(tmp_socket, &tmp_addr) == false) {
		printf("Accept error.\n");
		return SCS_SKTID_INVVAL;
	}

	HPVT_logging(LOG_LEVEL_NOTICE, "Accepted.");
	printf("Accepted.\n");

	if (HPVT_Context_is_transmitter(g_context) == false) {
		if (HPVT_SCS_get_transmitter_hasty_data(tmp_socket) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "get piggyback error!");
		}
	}

	scs_time tmp_time;
	tmp_time = g_context->settings.video.feedback_interval;

	if (g_context->settings.video.feedback_interval == 0) {
		if (SCSSetSocketOption(tmp_socket, SCS_SKTOPTNAME_FEEDBACK_DISABLE, &tmp_time, sizeof(tmp_time)) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
			return SCS_SKTID_INVVAL;
		}
	}
	else {
		if (SCSSetSocketOption(tmp_socket, SCS_SKTOPTNAME_FEEDBACK_INTERVAL, &tmp_time, sizeof(tmp_time)) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
			return SCS_SKTID_INVVAL;
		}
	}

	char tmp_buf_remote[32];
	char tmp_buf_local[32];
	int tmp_mtu;
	uint32_t tmp_connection_id;

	if (HPVT_SCS_get_socket_info(tmp_socket, tmp_buf_local, tmp_buf_remote, &tmp_mtu, &tmp_connection_id) == true) {
		printf("Established. (%s<->%s connectionID=%lu,socketID=%lu)\n", tmp_buf_local, tmp_buf_remote, tmp_connection_id, tmp_socket);
	}

	return tmp_socket;
}
int HPVT_UDP_create_client_socket(uint32_t peer_addr, uint16_t peer_port) {
	int sock;

	memset(&s_peer, 0, sizeof(s_peer));
	s_peer.sin_family = AF_INET;
	s_peer.sin_addr.s_addr = peer_addr;
	s_peer.sin_port = htons(peer_port);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		perror("socket");
		return -1;
	}

	return sock;
}
SCSSocketId HPVT_SCS_create_client_socket(uint32_t peer_addr, uint16_t peer_port) {
	SCSSocketId tmp_socket;
	scs_sockaddr tmp_self;
	scs_sockaddr tmp_peer;

	struct {
		struct sockaddr_in * self;
		struct sockaddr_in * peer;
	} tmp_sockaddr;

	SCSProtocolType protocol;
	protocol = SCS_PROTOCOLTYPE_UDP;
	memset(&tmp_self, 0, sizeof(tmp_self));
	tmp_sockaddr.self = (struct sockaddr_in *) &tmp_self;
	tmp_sockaddr.self->sin_family = AF_INET;
	tmp_sockaddr.self->sin_addr.s_addr = INADDR_ANY;
	tmp_sockaddr.self->sin_port = 0;
	memset(&tmp_peer, 0, sizeof(tmp_peer));
	tmp_sockaddr.peer = (struct sockaddr_in *) &tmp_peer;
	tmp_sockaddr.peer->sin_family = AF_INET;
	tmp_sockaddr.peer->sin_addr.s_addr = peer_addr;
	tmp_sockaddr.peer->sin_port = htons(peer_port);

	while (true) {

		if (g_context->connection.reconnection_attempt_on == false) {
			HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CLOSED);
			sleep(1);
			continue;
		}

		if ((tmp_socket = SCSCreateSocket(SCS_SKTTYPE_DATAGRAM, protocol)) == SCS_SKTID_INVVAL) {
			printf("Create error.\n");
			return SCS_SKTID_INVVAL;
		}

		if (HPVT_Context_is_transmitter(g_context) == true) {
			if (HPVT_SCS_set_transmitter_hasty_data(tmp_socket) == false) {
				return SCS_SKTID_INVVAL;
			}
		}

		HPVT_change_connection_state(HPVT_VIDEO_CONNECTION_STATE_CONNECTING);

		if (SCSBind(tmp_socket, tmp_self) == false) {
			printf("Bind error.\n");
			SCSDestroySocket(tmp_socket);
			sleep(1);
			continue;
		}

		HPVT_logging(LOG_LEVEL_NOTICE, "Connecting ..");
		char address_buffer[64];

		if (inet_ntop(AF_INET, &tmp_sockaddr.peer->sin_addr, address_buffer, sizeof(address_buffer)) != NULL) {
			printf("Connecting to %s:%d  ...\n", address_buffer, peer_port);
		}

		if (SCSConnect(tmp_socket, tmp_peer) == false) {
			printf("Connect error.\n");
			SCSDestroySocket(tmp_socket);
			sleep(1);
			continue;
		}

		printf("Connected.\n");
		HPVT_logging(LOG_LEVEL_NOTICE, "Connected.");
		break;

	}

	if (HPVT_Context_is_transmitter(g_context) == false) {
		if (HPVT_SCS_get_transmitter_hasty_data(tmp_socket) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "get piggyback error!");
		}
	}

	scs_time tmp_time;
	tmp_time = g_context->settings.video.feedback_interval;

	if (g_context->settings.video.feedback_interval == 0) {
		if (SCSSetSocketOption(tmp_socket, SCS_SKTOPTNAME_FEEDBACK_DISABLE, &tmp_time, sizeof(tmp_time)) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
			return SCS_SKTID_INVVAL;
		}
	}
	else {
		if (SCSSetSocketOption(tmp_socket, SCS_SKTOPTNAME_FEEDBACK_INTERVAL, &tmp_time, sizeof(tmp_time)) == false) {
			HPVT_logging(LOG_LEVEL_ERROR, "socket option error");
			return SCS_SKTID_INVVAL;
		}
	}

	char tmp_buf_remote[32];
	char tmp_buf_local[32];
	int tmp_mtu;
	uint32_t tmp_connection_id;

	if (HPVT_SCS_get_socket_info(tmp_socket, tmp_buf_local, tmp_buf_remote, &tmp_mtu, &tmp_connection_id) == true) {
		printf("Established. (%s<->%s connectionID=%u,socketID=%d)\n", tmp_buf_local, tmp_buf_remote, tmp_connection_id, tmp_socket);
	}

	return tmp_socket;
}
static void HPVT_UDP_dispose_socket(SCSSocketId sock) {

	close(sock);
	printf("Closed socket.(%d)\n", sock);
	HPVT_logging(LOG_LEVEL_NOTICE, "Closed socket.(%d)\n", sock);
}
static void HPVT_SCS_dispose_socket(SCSSocketId sock) {

	SCSDestroySocket(sock);
	printf("Closed socket.(ID=%d)\n", sock);
	HPVT_logging(LOG_LEVEL_NOTICE, "Closed socket.(%d)\n", sock);
}
void HPVT_dispose_socket(SCSSocketId sock) {

	if (sock == SCS_SKTID_INVVAL) {
		return;
	}

	if (g_context->settings.system.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
		HPVT_UDP_dispose_socket(sock);
	}
	else {
		HPVT_SCS_dispose_socket(sock);
	}
}
static int HPVT_UDP_send(int sock, unsigned char* buffer, unsigned int buffer_len) {

	int ret;

	ret = sendto(sock, buffer, buffer_len, 0, (struct sockaddr*) &s_peer, sizeof(s_peer));

	return ret;
}
static int HPVT_SCS_send(SCSSocketId sock, unsigned char* buffer, unsigned int buffer_len, int scsrednum) {

	size_t tmp_offset = 0;
	if (DGSSend(sock, buffer, buffer_len, tmp_offset,
	SCS_SKTOPTN_NONE, scsrednum, NULL) == false) {
		HPVT_logging(LOG_LEVEL_WARNING, "DGSSend failure");
		return -1;
	}

	return (int) tmp_offset;
}
int HPVT_send(SCSSocketId sock, unsigned char* buffer, unsigned int buffer_len) {

	if(sock == SCS_SKTID_INVVAL){
		HPVT_logging(LOG_LEVEL_WARNING, "socket id is invalid");
		return -1;
	}

	if (g_context->settings.system.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
		return HPVT_UDP_send(sock, buffer, buffer_len);
	}
	else {
		return HPVT_SCS_send(sock, buffer, buffer_len, 0);
	}
}
static int HPVT_UDP_recv(int sock, unsigned char* buffer, unsigned int buffer_len) {

	int ret;

	while (true) {

		ret = recvfrom(sock, buffer, buffer_len, 0, (struct sockaddr*) &tmp_peer_sockaddr, &sockaddr_in_len);

		if (ret > 0) {

			if (g_context->connection.peer.addr == 0) {
				g_context->connection.peer.addr = tmp_peer_sockaddr.sin_addr.s_addr;
				g_context->connection.peer.port = ntohs(tmp_peer_sockaddr.sin_port);
			}
			else {
				if (g_context->connection.peer.addr != tmp_peer_sockaddr.sin_addr.s_addr) {
					return -2;
				}

				if (g_context->connection.peer.port != ntohs(tmp_peer_sockaddr.sin_port)) {
					return -2;
				}
			}
		}

		if (ret == -1 && errno == EAGAIN) {
			g_context->connection.peer.addr = 0;
			g_context->connection.peer.port = 0;
			continue;
		}
		else {
			break;
		}
	}

	return ret;

}
static int HPVT_SCS_recv(SCSSocketId sock, unsigned char* buffer, unsigned int buffer_len) {

	size_t tmp_offset = 0;
	SCSPacketSeqno tmp_seqno;

	if (DGSRecv(sock, buffer, buffer_len, &tmp_offset, SCS_SKTOPTN_NONE, &tmp_seqno) == false) {
		HPVT_logging(LOG_LEVEL_NOTICE, "DGSReceive failure");
		return -1;
	}

	return (int) tmp_offset;
}
int HPVT_recv(SCSSocketId sock, unsigned char* buffer, unsigned int buffer_len) {

	if(sock == SCS_SKTID_INVVAL){
		HPVT_logging(LOG_LEVEL_WARNING, "socket id is invalid");
		return -1;
	}

	if (g_context->settings.system.connection_mode == HPVT_Config_CONNECTION_MODE_CONNECTION_LESS) {
		return HPVT_UDP_recv(sock, buffer, buffer_len);
	}
	else {
		return HPVT_SCS_recv(sock, buffer, buffer_len);
	}
}
