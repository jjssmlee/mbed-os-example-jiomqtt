#include "mbed.h"
#include "jio_utils/jio_utils.h"
#include "netsocket/nsapi_types.h"

#define MQTT_SERVER_IP_ADDR		"39.105.204.1"
#define MQTT_SERVER_PORT		"1883"

osSemaphoreId_t sem_conn;

void jiot_client_connecte_complete_cb(void *context,
		jiot_client_MQTT_token_t token)
{
	printf("conenct callback: connect success\n");
	osSemaphoreRelease(sem_conn);
}

void jiot_client_connecte_failed_cb(void *context, const char *cause,
		int code)
{
	printf("jio connect failed\n\n");
}

void jiot_client_message_deliveryComplete(void *context,
		jiot_client_MQTT_token_t token)
{
	printf("jio pub cb success, token=%d\n\n", token);
}

void jiot_client_message_deliveryFailed(void *context, const char *cause,
		jiot_client_MQTT_token_t token)
{
	printf("jio pub cb failed, token=%d, cause=%s\n\n", token, cause);
}

void jiot_client_subscribeComplete(void *context,
		jiot_client_MQTT_token_t token, jiot_client_MQTT_qos_t qos)
{
	printf("jio sub cb success, token=%d, qos=%d\n\n", token, qos);
}
void jiot_client_subscribeManyComplete(void *context,
		jiot_client_MQTT_token_t token, int *qos)
{
	printf("jio sub many cb success, token=%d, qos=%d\n\n", token, qos);
}
void jiot_client_subscribeFailed(void *context, const char *cause, int code,
		jiot_client_MQTT_token_t token)
{
	printf("jio sub cb fail, token=%d, code=%d\n\n", token, code);
}

void jiot_client_disconnectionComplete(void *context)
{
	printf("jio disconn cb success\n\n");
}

void jiot_client_disconnectionFailed(void *context, const char *cause, int code)
{
	printf("jio disconn cb fail, code=%d, cause=%s\n\n", code, cause);
}

void jiot_client_messageArrived(void *context, char *topic, int topicLen,
		jiot_client_MQTT_message_t *message)
{
	char *result = (char *)message->payload;

	printf("jio message arrive cb, topic=%s,\n", topic);
	printf("payload: %s\n\n", result);
}

void jiot_client_unsubscribeComplete(void *context,
		jiot_client_MQTT_token_t token)
{
	printf("jio unsub success cb, token=%s\n\n", token);
}

void jiot_client_unsubscribeFailed(void *context, const char *cause, int code,
		jiot_client_MQTT_token_t token)
{
	printf("jio unsub failed cb, token=%s, code=%d, cause=%s\n\n", token, code,
			cause);
}

jiot_client_MQTT_Hndl_t jiot_mqtt_client_create(void)
{
	jiot_client_MQTT_Hndl_t client_handle = NULL;
	jiot_client_MQTT_createOpts_t options;
	int result = JIOT_MQTT_OPERATION_SUCCESS;

	options.clientId = (char *)malloc(sizeof(char) * 12);
	memcpy(options.clientId, "mqtt_client", 12);
	options.qos = JIOT_QOS_NONE;
	options.retained = 0;
	options.username = NULL;
	options.password = NULL;
	options.host = (char *)malloc(sizeof(char) * 16);
	memcpy(options.host, MQTT_SERVER_IP_ADDR, 16);
	options.port = (char *)malloc(sizeof(char) * 5);
	memcpy(options.port, MQTT_SERVER_PORT, 5);
	options.keepAlive = 0;

	result = jiot_client_MQTT_create(&client_handle, options);

	return client_handle;
}

int jiot_mqtt_client_set_callbacks(jiot_client_MQTT_Hndl_t handle)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;
	jiot_client_MQTT_callbackOpts_t callbacks;

	callbacks.connectionComplete = jiot_client_connecte_complete_cb;
	callbacks.connectionFailed = jiot_client_connecte_failed_cb;
	callbacks.deliveryComplete = jiot_client_message_deliveryComplete;
	callbacks.deliveryFailed = jiot_client_message_deliveryFailed;
	callbacks.subscribeComplete = jiot_client_subscribeComplete;
	callbacks.subscribeFailed = jiot_client_subscribeFailed;
	callbacks.subscribeManyComplete = jiot_client_subscribeManyComplete;
	callbacks.disconnectComplete = jiot_client_disconnectionComplete;
	callbacks.disconnectFailed = jiot_client_disconnectionFailed;
	callbacks.messageArrived = jiot_client_messageArrived;
	callbacks.unsubscribeComplete = jiot_client_unsubscribeComplete;
	callbacks.unsubscribeFailed = jiot_client_unsubscribeFailed;

	jiot_client_MQTT_setcallbacks(handle, handle, callbacks);

	return result;
}

int network_connect(void)
{
	int ret = 0;
	WiFiInterface *wifi;

	wifi = WiFiInterface::get_default_instance();
	if (!wifi) {
		printf("ERROR: No WiFiInterface found.\n");
		return (-1);
	}

	printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
	int result = wifi->connect(MBED_CONF_APP_WIFI_SSID,
			MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
	if (result != 0) {
		printf("\nConnection error: %d\n", result);
		return (-1);
	}

	printf("Network connect success!\n");
	printf("MAC: %s\n", wifi->get_mac_address());
	printf("IP: %s\n", wifi->get_ip_address());
	printf("Netmask: %s\n", wifi->get_netmask());
	printf("Gateway: %s\n", wifi->get_gateway());
	printf("RSSI: %d\n\n", wifi->get_rssi());

	return ret;
}

int main()
{
	jiot_client_MQTT_Hndl_t client_handle = NULL;
	int result = JIOT_MQTT_OPERATION_SUCCESS;
	int con_state;
	int ret = 0;

	/* Do network connection, you can implement it by yourself, we use WiFi network */
	ret = network_connect();
	if (ret) {
		printf("connect network error\n");
		return ret;
	}

	client_handle = jiot_mqtt_client_create();
	if (!client_handle) {
		printf("mqtt client create error\n");
		return 0;
	}

	result = jiot_mqtt_client_set_callbacks(client_handle);
	if (result != JIOT_MQTT_OPERATION_SUCCESS) {
		printf("mqtt client set callback error\n");
		return 0;
	}

	result = jiot_client_MQTT_connect(client_handle);
	if (result != JIOT_MQTT_OPERATION_SUCCESS) {
		printf("mqtt client connect error\n");
		return 0;
	}

	osSemaphoreAttr_t attr;
	char cb_buff[512] = {0};
	attr.name = "connect sem";
	attr.attr_bits = 0;
	attr.cb_mem = (void *)cb_buff;
	attr.cb_size = 512;

	sem_conn = osSemaphoreNew(1, 0, &attr);
	if (!sem_conn)
		printf("semphore is null\n");
	osStatus_t status = osSemaphoreAcquire(sem_conn, 10000);

	con_state = jiot_client_MQTT_isConnected(client_handle);
	if (con_state)
		printf("Jio mqtt connected success\n");

	/* Do mqtt subscribe */
	wait(5);
	result = jiot_client_MQTT_subscribeWithQos(client_handle, "topic_mqtt",
			JIOT_QOS_NONE);
	if (result < 0) {
		printf("jio sub failed, result=%d, qos=%d\n", result, JIOT_QOS_NONE);
	} else {
		printf("jio sub success, token=%d, qos=%d\n", result, JIOT_QOS_NONE);
	}

	/* Do mqtt publish, this publish topic and content will received in callback */
	wait(2);
	char context[20] = "jio_mqtt_conetx";
	result = jiot_client_MQTT_publishWithQos(client_handle, "topic_mqtt",
			context, 16, JIOT_QOS_NONE);
	if (result < 0) {
		printf("jio pub failed, result=%d\n", result);
	}
	printf("jio pub success, token=%d\n", result);
	wait(10);

	/* Test finish, disconnect connection*/
	result = jiot_client_MQTT_disconnect(client_handle);
	if (result < 0)
		printf("disconnect failed, result=%d\n", result);
	printf("disconnect jio mqtt success\n");

	jiot_client_MQTT_destroy(&client_handle);

	wait(10);
	while(true)
		wait(5);

	return 0;
}
