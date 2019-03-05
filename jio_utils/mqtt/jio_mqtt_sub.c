#include "cmsis_os2.h"
#include "jio_mqtt_handle.h"

jiot_client_MQTT_data_t *mqtt_data;

/*Jio MQTT callbacks*/
void jiot_client_mqtt_incoming_publish_cb(
				void *arg, const char *topic, u32_t tot_len)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	jiot_client_MQTT_callbackOpts_t *client_callback = NULL;

	client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
	if (!client_handle)
		goto exit;

	client_callback = client_handle->client_callback;
	if (!client_callback)
		goto exit;

	if (tot_len <= 0)
		goto error;

	mqtt_data = (jiot_client_MQTT_data_t *)malloc(
					sizeof(jiot_client_MQTT_data_t));
	if (!mqtt_data)
		goto exit;

	mqtt_data->topic = (char *)malloc(sizeof(char *) * (strlen(topic) + 1));
	if (!mqtt_data->topic)
		goto error;
	mqtt_data->topiclen = strlen(topic);
	memcpy(mqtt_data->topic, topic, mqtt_data->topiclen);
	mqtt_data->topic[mqtt_data->topiclen] = 0;
	mqtt_data->payload_index = 0;

	mqtt_data->message.payload = (void *)malloc(tot_len + 1);
	if (!mqtt_data->message.payload)
		goto error;
	//mqtt_data->message.payload[tot_len] = 0;
	mqtt_data->message.payloadlen = tot_len;
	goto exit;

error:
	if (mqtt_data->message.payload)
		free(mqtt_data->message.payload);
	if (mqtt_data->topic)
		free(mqtt_data->topic);

	if (mqtt_data)
		free(mqtt_data);
	mqtt_data = NULL;

exit:
	return;
}

void jiot_client_data_timeout_cb (void *arg)
{
	bool *data_status = (bool *)arg;

	if (!data_status)
		return;
	*data_status = false;
	return;
}

void jiot_client_mqtt_incoming_data_cb(
				void *arg, const u8_t *data, u16_t len, u8_t flags)
{
	static osTimerId_t timer_id = NULL;
	bool data_status = true;

	if (!arg || !mqtt_data)
		goto error;
	if (!data || len <= 0)
		goto error;

	if (timer_id && (osTimerIsRunning (timer_id))) {
		osTimerStop (timer_id);
		osTimerDelete (timer_id);
	}
	timer_id = NULL;

	memcpy(mqtt_data->message.payload + mqtt_data->payload_index,
			data, len);
	((char *)(mqtt_data->message.payload))[mqtt_data->payload_index + len] = 0;
	mqtt_data->payload_index += len;

	if (flags != MQTT_DATA_FLAG_LAST) {
		osTimerAttr_t attr;
		attr.name = "data_timer";
		attr.cb_mem = (void *)malloc(128);
		attr.cb_size = 128;
		timer_id = osTimerNew (jiot_client_data_timeout_cb, osTimerOnce,
								(void *)(&data_status), &attr);
		if (!timer_id)
			goto error;
		uint32_t ticks = 1000;
		osTimerStart (timer_id, ticks);
	} else {
		_jiot_client_MQTT_Hndl_t *client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
		if (!client_handle)
			goto exit;
		jiot_client_MQTT_callbackOpts_t *client_callback = client_handle->client_callback;
		if (!client_callback)
			goto exit;
		if (!client_callback->messageArrived) {
			printf("No message arrived callback\n");
			goto error;
		}

		client_callback->messageArrived(arg, mqtt_data->topic, mqtt_data->topiclen,
										&(mqtt_data->message));
	}

	if (!data_status)
		goto error;

	return;
error:
	if (osTimerIsRunning (timer_id)) {
		osTimerStop (timer_id);
		osTimerDelete (timer_id);
		timer_id = NULL;
	}
exit:
	if (mqtt_data->message.payload)
		free(mqtt_data->message.payload);
	if (mqtt_data->topic)
		free(mqtt_data->topic);

	if (mqtt_data)
		free(mqtt_data);
	mqtt_data = NULL;

	return;
}

static void jiot_client_mqtt_sub_request_cb(void *arg, err_t result)
{
	_jiot_client_MQTT_Hndl_t *client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
	jiot_client_MQTT_token_t token;
	jiot_client_MQTT_qos_t qos;

	if (!client_handle || !client_handle->client_callback) {
		return;
	}

	token = client_handle->request.token;
	qos = client_handle->request.qos;

	if (result == ERR_OK && client_handle->client_callback->subscribeComplete) {
		client_handle->client_callback->subscribeComplete(client_handle->context,
				token, qos);
	} else if (result != ERR_OK
			&& client_handle->client_callback->subscribeFailed) {
		client_handle->client_callback->subscribeFailed(client_handle->context,
				lwip_strerr(result), result, token);
	}
}

static void jiot_client_mqtt_unsub_request_cb(void *arg, err_t result)
{
	_jiot_client_MQTT_Hndl_t *client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
	jiot_client_MQTT_token_t token;

	if (!client_handle || !client_handle->client_callback) {
		return;
	}

	token = client_handle->request.token;
	if (result == ERR_OK
			&& client_handle->client_callback->unsubscribeComplete) {
		client_handle->client_callback->unsubscribeComplete(client_handle->context,
				token);
	} else if (result != ERR_OK
			&& client_handle->client_callback->unsubscribeFailed) {
		client_handle->client_callback->unsubscribeFailed(client_handle->context,
				lwip_strerr(result), result, token);
	}
}

jiot_client_MQTT_token_t jiot_client_MQTT_subscribe (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic)
{
	return jiot_client_MQTT_subscribeWithQos(handle, topic, JIOT_QOS_DEFAULT);
}

jiot_client_MQTT_token_t jiot_client_MQTT_subscribeWithQos (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic,
								jiot_client_MQTT_qos_t qos)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	jiot_client_MQTT_token_t token;
	err_t ret;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle) {
		return JIOT_MQTT_OPERATION_FAILURE;
	}

	if (!topic) {
		return JIOT_MQTT_OPERATION_FAILURE;
	}

	if (qos > JIOT_QOS_TWO || qos < JIOT_QOS_NONE) {
		return JIOT_MQTT_OPERATION_BAD_QOS;
	}

	token = jiot_client_mqtt_generate_token(client_handle);
	client_handle->request.token = token;
	client_handle->request.qos = qos;

	ret = mqtt_subscribe(client_handle->client, topic, qos,
			jiot_client_mqtt_sub_request_cb, handle);

	if (ret == ERR_OK) {
		return token;
	}

	return JIOT_MQTT_OPERATION_FAILURE;
}

jiot_client_MQTT_token_t jiot_client_MQTT_subscribeMany (
								jiot_client_MQTT_Hndl_t handle,
								int count,
								char *const *topic)
{
	jiot_client_MQTT_token_t token;
	return token;
}

jiot_client_MQTT_token_t jiot_client_MQTT_subscribeManyWithQos (
								jiot_client_MQTT_Hndl_t handle,
								int count,
								char *const *topic,
								jiot_client_MQTT_qos_t *qos)
{
	jiot_client_MQTT_token_t token;
	return token;
}

jiot_client_MQTT_token_t jiot_client_MQTT_unsubscribe (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic)
{
	_jiot_client_MQTT_Hndl_t *client_handle = NULL;
	jiot_client_MQTT_token_t token;
	err_t ret;

	client_handle = (_jiot_client_MQTT_Hndl_t *)handle;
	if (!client_handle) {
		return JIOT_MQTT_OPERATION_FAILURE;
	}

	if (!topic) {
		return JIOT_MQTT_OPERATION_FAILURE;
	}

	token = jiot_client_mqtt_generate_token(client_handle);
	client_handle->request.token = token;

	ret = mqtt_unsubscribe(client_handle->client, topic,
			jiot_client_mqtt_unsub_request_cb, handle);
	if (ret == ERR_OK) {
		return token;
	}

	return JIOT_MQTT_OPERATION_FAILURE;
}

jiot_client_MQTT_token_t jiot_client_MQTT_unsubscribeMany (
								jiot_client_MQTT_Hndl_t handle,
								int length,
								char *const *topics)
{
	jiot_client_MQTT_token_t token;
	return token;
}
