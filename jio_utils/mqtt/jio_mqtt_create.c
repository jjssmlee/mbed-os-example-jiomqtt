#include "cmsis_os2.h"
#include "jio_mqtt_handle.h"

/* Jio MQTT API */
int jiot_client_MQTT_create (jiot_client_MQTT_Hndl_t *handle,
								jiot_client_MQTT_createOpts_t options)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;
	mqtt_client_t *mqtt_client = NULL;
	struct mqtt_connect_client_info_t *client_info = NULL;
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;

	if (!handle || !options.clientId)
		goto error;

	client_handle = (_jiot_client_MQTT_Hndl_t *)malloc(sizeof(_jiot_client_MQTT_Hndl_t));
	if (!client_handle)
		goto error;

	mqtt_client = (mqtt_client_t *)malloc(sizeof(mqtt_client_t));
	if (!mqtt_client)
		goto error;
	memset(mqtt_client, 0, sizeof(mqtt_client_t));

	client_info = (struct mqtt_connect_client_info_t *)
					malloc(sizeof(struct mqtt_connect_client_info_t));
	if (!client_info)
		goto error;

	client_info->client_id = (char *)malloc(
								strlen(options.clientId) * sizeof(char));
	if (!client_info->client_id)
		goto error;
	memcpy(client_info->client_id, options.clientId, strlen(options.clientId));

	if (options.username) {
		client_info->client_user = (char *)malloc(
									strlen(options.username) * sizeof(char));
		if (!client_info->client_user)
			goto error;
		memcpy(client_info->client_user, options.username, strlen(options.username));
	} else
		client_info->client_user = NULL;

	if (options.password) {
		client_info->client_pass = (char *)malloc(
									strlen(options.password) * sizeof(char));
		if (!client_info->client_pass)
			goto error;
		memcpy(client_info->client_pass, options.password, strlen(options.password));
	} else
		client_info->client_pass = NULL;

	if (options.host) {
		client_handle->host = (char *)malloc(
									strlen(options.host) * sizeof(char) + 1);
		if (!client_handle->host)
			goto error;
		memcpy(client_handle->host, options.host, strlen(options.host));
		client_handle->host[strlen(options.host)] = '\0';
	} else
		client_handle->host = NULL;

	if (options.port) {
		client_handle->port = (char *)malloc(
									strlen(options.port) * sizeof(char) + 1);
		if (!client_handle->port)
			goto error;
		memcpy(client_handle->port, options.port, strlen(options.port));
		client_handle->port[strlen(options.port)] = '\0';
	} else
		client_handle->port = NULL;

	if (options.certPassword) {
		client_handle->certPassword = (char *)malloc(
									strlen(options.certPassword) * sizeof(char));
		if (!client_handle->certPassword)
			goto error;
		memcpy(client_handle->certPassword, options.certPassword, strlen(options.certPassword));
	} else
		client_handle->certPassword = NULL;

	client_info->keep_alive = options.keepAlive;

	client_handle->client = mqtt_client;
	client_handle->client_info = client_info;
	client_handle->persist_opts = NULL;
	client_handle->client_callback = NULL;
	client_handle->qos = options.qos;
	client_handle->retained = options.retained;
	client_handle->useSSL = options.useSSL;
	client_handle->token = 0;

	*handle = (void *)client_handle;
	goto exit;

error:
	if (client_info) {
		if (client_info->client_id)
			free(client_info->client_id);
		if (client_info->client_user)
			free(client_info->client_user);
		if (client_info->client_pass)
			free(client_info->client_pass);
		free(client_info);
	}
	if (mqtt_client)
		free(mqtt_client);

	if (client_handle) {
		if (client_handle->host)
			free(client_handle->host);
		if (client_handle->port)
			free(client_handle->port);
		if (client_handle->certPassword)
			free(client_handle->certPassword);
		client_handle->client = NULL;
		client_handle->client_info = NULL;
		client_handle->host = NULL;
		client_handle->port = NULL;
		client_handle->certPassword = NULL;
		free(client_handle);
	}
	result = JIOT_MQTT_OPERATION_FAILURE;
exit:
	return result;
}

int jiot_client_MQTT_createWithPersistence (
								jiot_client_MQTT_Hndl_t *handle,
								jiot_client_MQTT_createOpts_t options,
								jiot_client_MQTT_persistOpts_t persistOpts)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;

	return result;
}

void jiot_client_MQTT_setcallbacks (
								jiot_client_MQTT_Hndl_t handle,
								void *context,
								jiot_client_MQTT_callbackOpts_t callbacks)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	jiot_client_MQTT_callbackOpts_t *client_callback = NULL;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle)
		goto error;

	client_handle->context = context;
	client_callback = client_handle->client_callback;
	if (!client_callback) {
		client_callback = (jiot_client_MQTT_callbackOpts_t *)malloc(
							sizeof(jiot_client_MQTT_callbackOpts_t));
		client_handle->client_callback = client_callback;
	}
	if (!client_callback)
		goto error;
	/*delivery complete callback*/
	if (callbacks.deliveryComplete)
		client_callback->deliveryComplete = callbacks.deliveryComplete;
	else
		client_callback->deliveryComplete = NULL;
	/*delivery failed callback*/
	if (callbacks.deliveryFailed)
		client_callback->deliveryFailed = callbacks.deliveryFailed;
	else
		client_callback->deliveryFailed = NULL;
	/*connect complete callback*/
	if (callbacks.connectionComplete)
		client_callback->connectionComplete = callbacks.connectionComplete;
	else
		client_callback->connectionComplete = NULL;
	/*connect failed callback*/
	if (callbacks.connectionFailed)
		client_callback->connectionFailed = callbacks.connectionFailed;
	else
		client_callback->connectionFailed = NULL;
	/*atuto reconnect complete callback*/
	if (callbacks.autoReconnectComplete)
		client_callback->autoReconnectComplete = callbacks.autoReconnectComplete;
	else
		client_callback->autoReconnectComplete = NULL;
	/*disconnect complete callback*/
	if (callbacks.disconnectComplete)
		client_callback->disconnectComplete = callbacks.disconnectComplete;
	else
		client_callback->disconnectComplete = NULL;
	/*disconnect failed callback*/
	if (callbacks.disconnectFailed)
		client_callback->disconnectFailed = callbacks.disconnectFailed;
	else
		client_callback->disconnectFailed = NULL;
	/*subsrcribe complete callback*/
	if (callbacks.subscribeComplete)
		client_callback->subscribeComplete = callbacks.subscribeComplete;
	else
		client_callback->subscribeComplete = NULL;
	/*subsrcribe many complete callback*/
	if (callbacks.subscribeManyComplete)
		client_callback->subscribeManyComplete = callbacks.subscribeManyComplete;
	else
		client_callback->subscribeManyComplete = NULL;
	/*subsrcribe failed callback*/
	if (callbacks.subscribeFailed)
		client_callback->subscribeFailed = callbacks.subscribeFailed;
	else
		client_callback->subscribeFailed = NULL;
	/*message arrived callback*/
	if (callbacks.messageArrived)
		client_callback->messageArrived = callbacks.messageArrived;
	else
		client_callback->messageArrived = NULL;
	/*unsubscribe complete callback*/
	if (callbacks.unsubscribeComplete)
		client_callback->unsubscribeComplete = callbacks.unsubscribeComplete;
	else
		client_callback->unsubscribeComplete = NULL;
	/*unsubscribe failed callback*/
	if (callbacks.unsubscribeFailed)
		client_callback->unsubscribeFailed = callbacks.unsubscribeFailed;
	else
		client_callback->unsubscribeFailed = NULL;

	goto exit;

error:
	if (client_callback)
		free(client_callback);
	client_handle->client_callback = NULL;
exit:
	return;
}

int jiot_client_MQTT_getPendingTokens (
								jiot_client_MQTT_Hndl_t handle,
								jiot_client_MQTT_token_t **tokens)
{
	int result = JIOT_MQTT_OPERATION_SUCCESS;

	return result;
}

void jiot_client_MQTT_destroy (jiot_client_MQTT_Hndl_t *handle)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	mqtt_client_t *mqtt_client = NULL;
	struct mqtt_connect_client_info_t *client_info = NULL;

	client_handle = (_jiot_client_MQTT_Hndl_t *)(*handle);
	if (!client_handle)
		goto exit;

	client_info = client_handle->client_info;
	mqtt_client = client_handle->client;
	if (client_info) {
		if (client_info->client_id)
			free(client_info->client_id);
		if (client_info->client_user)
			free(client_info->client_user);
		if (client_info->client_pass)
			free(client_info->client_pass);
		free(client_info);
	}
	if (mqtt_client)
		free(mqtt_client);
	if (client_handle->host)
		free(client_handle->host);
	if (client_handle->port)
		free(client_handle->port);
	if (client_handle->certPassword)
		free(client_handle->certPassword);
	free(client_handle);

exit:
	*handle= NULL;
	return;
}
