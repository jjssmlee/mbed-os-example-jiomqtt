#ifndef JIO_MQTT_HANDLE_H
#define JIO_MQTT_HANDLE_H

#include "jio_mqtt_utils.h"

#define JIOT_MQTT_DISCONNECT_CB_SIZE	256
#define JIOT_MQTT_DISCONNECT_STACK_SIZE	1024

#define JIO_MQTT_CONNECTED_TRUE			1
#define JIO_MQTT_CONNECTED_FALSE		0

typedef struct {
	char *topic;
	int topiclen;
	int payload_index;
	jiot_client_MQTT_message_t message;
} jiot_client_MQTT_data_t;

typedef struct {
	jiot_client_MQTT_token_t token;
	jiot_client_MQTT_qos_t qos;
	u16_t pkt_id;
} jiot_client_MQTT_Request_t;

/* Client handle*/
typedef struct {
	mqtt_client_t *client;
	struct mqtt_connect_client_info_t *client_info;
	jiot_client_MQTT_persistOpts_t *persist_opts;
	jiot_client_MQTT_callbackOpts_t *client_callback;
	jiot_client_MQTT_qos_t qos;
	void *context;
	int retained;
	char *host;
	char *port;
	int useSSL;
	char *certPassword;
	jiot_client_MQTT_Request_t request;
	jiot_client_MQTT_token_t token;
} _jiot_client_MQTT_Hndl_t;

/* Internal functions */
static inline jiot_client_MQTT_token_t jiot_client_mqtt_generate_token(
										_jiot_client_MQTT_Hndl_t *handle)
{
	handle->token ++;
	if (handle->token == 0)
		handle->token ++;
	if (handle->token < 0)
		handle->token = 1;

	return handle->token;
}

#endif
