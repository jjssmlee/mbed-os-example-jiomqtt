#include "cmsis_os2.h"
#include "jio_mqtt_handle.h"

static void *disconnect_cb_mem = NULL;
static void *disconnect_stack_mem = NULL;

/*Jio MQTT callbacks*/
extern void jiot_client_mqtt_incoming_publish_cb(
				void *arg, const char *topic, u32_t tot_len);
extern void jiot_client_mqtt_incoming_data_cb(
				void *arg, const u8_t *data, u16_t len, u8_t flags);

static void jiot_client_mqtt_connection_cb(
				mqtt_client_t *client, void *arg,
				mqtt_connection_status_t status)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	jiot_client_MQTT_callbackOpts_t *client_callback = NULL;

	client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
	if (!client_handle)
		goto exit;

	client_callback = client_handle->client_callback;
	if (!client_callback)
		goto exit;

	if (status == MQTT_CONNECT_ACCEPTED) {
		jiot_client_MQTT_token_t token = 0;
		if (client_callback->connectionComplete)
			client_callback->connectionComplete(client_handle->context, token);
		else
			printf("No connection complete callback\n");
	} else {
		if (client_callback->connectionFailed)
			client_callback->connectionFailed(client_handle->context,
											"Connect Failed", status);
		else
			printf("No connection failed callback\n");
	}

exit:
	return;
}

/*Jio MQTT API*/
int jiot_client_MQTT_connect (jiot_client_MQTT_Hndl_t handle)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	ip_addr_t ip_addr;
	u16_t port;
	err_t ret;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle)
		goto error;

	ip_addr.addr = inet_addr(client_handle->host);
	port = atoi(client_handle->port);

	ret = mqtt_client_connect(client_handle->client, &ip_addr,
								port, jiot_client_mqtt_connection_cb,
								handle, client_handle->client_info);
	if (ret != 0)
		goto error;
	mqtt_set_inpub_callback(client_handle->client,
						jiot_client_mqtt_incoming_publish_cb,
                        jiot_client_mqtt_incoming_data_cb,
                        (void *)handle);
	goto exit;

error:
	result = JIOT_MQTT_OPERATION_FAILURE;
exit:
	return result;
}

int jiot_client_MQTT_connectWithOptions (
								jiot_client_MQTT_Hndl_t handle,
								jiot_client_MQTT_connectOpts_t *options)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;

	return result;
}

int jiot_client_MQTT_isConnected (jiot_client_MQTT_Hndl_t handle)
{
	int result = JIO_MQTT_CONNECTED_FALSE;
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	u8_t con_state;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle)
		goto exit;

	con_state = mqtt_client_is_connected(client_handle->client);
	if (con_state)
		result = JIO_MQTT_CONNECTED_TRUE;

exit:
	return result;
}

void jiot_client_disconnect_func(void *ptr)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	mqtt_client_t *mqtt_client = NULL;
	jiot_client_MQTT_callbackOpts_t *client_callback = NULL;

	client_handle = (_jiot_client_MQTT_Hndl_t *)ptr;
	if (!client_handle)
		goto exit;

	client_callback = client_handle->client_callback;
	if (!client_callback)
		goto exit;

	mqtt_client = client_handle->client;
	if (!mqtt_client)
		goto error;

	mqtt_disconnect(mqtt_client);

	if (client_callback->disconnectComplete)
		client_callback->disconnectComplete(ptr);
	else
		printf("No disconnect complete callback\n");
	goto exit;

error:
	if (client_callback->disconnectFailed)
		client_callback->disconnectFailed(ptr, "Disconnect failed\n",
											JIOT_MQTT_OPERATION_FAILURE);
	else
		printf("No disconnect failed callback\n");
exit:
	return;
}

static osThreadAttr_t jiot_client_create_attr(void)
{
	osThreadAttr_t attr;

	if (disconnect_cb_mem) {
		free(disconnect_cb_mem);
		disconnect_cb_mem = NULL;
	} else {
		disconnect_cb_mem = (void *)malloc(JIOT_MQTT_DISCONNECT_CB_SIZE);
		memset(disconnect_cb_mem, 0, JIOT_MQTT_DISCONNECT_CB_SIZE);
	}
	if (disconnect_stack_mem) {
		free(disconnect_stack_mem);
		disconnect_stack_mem = NULL;
	} else {
		disconnect_stack_mem = (void *)malloc(JIOT_MQTT_DISCONNECT_STACK_SIZE);
		memset(disconnect_stack_mem, 0, JIOT_MQTT_DISCONNECT_STACK_SIZE);
	}

	attr.name = "disconnect thread";
	attr.attr_bits = 0;
	attr.cb_mem = (void *)disconnect_cb_mem;
	attr.cb_size = JIOT_MQTT_DISCONNECT_CB_SIZE;
	attr.stack_mem = (void *)disconnect_stack_mem;
	attr.stack_size = JIOT_MQTT_DISCONNECT_STACK_SIZE;
	attr.priority = osPriorityAboveNormal;

	return attr;
}

int jiot_client_MQTT_disconnect (jiot_client_MQTT_Hndl_t handle)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	mqtt_client_t *mqtt_client = NULL;
	osThreadId_t thread_id;
	osThreadAttr_t attr;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle)
		goto error;

	mqtt_client = client_handle->client;
	if (!mqtt_client)
		goto error;

	attr = jiot_client_create_attr();
	thread_id = osThreadNew(jiot_client_disconnect_func, handle, &attr);
	if (!thread_id) {
		printf("create disconnect thread failed\n");
		goto error;
	}
	goto exit;

error:
	result = JIOT_MQTT_OPERATION_FAILURE;
exit:
	return result;
}
