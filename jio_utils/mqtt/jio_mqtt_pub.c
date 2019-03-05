#include "jio_mqtt_handle.h"

static void jiot_client_mqtt_pub_request_cb(void *arg, err_t result)
{
	_jiot_client_MQTT_Hndl_t *client_handle = (_jiot_client_MQTT_Hndl_t *)arg;
	jiot_client_MQTT_token_t token;

	if (!client_handle || !client_handle->client_callback) {
		return;
	}

	token = client_handle->request.token;
	if (result == ERR_OK && client_handle->client_callback->deliveryComplete) {
		client_handle->client_callback->deliveryComplete(client_handle->context, token);
	} else if (result != ERR_OK
			&& client_handle->client_callback->deliveryFailed) {
		client_handle->client_callback->deliveryFailed(client_handle->context,
				lwip_strerr(result), token);
	}
}


jiot_client_MQTT_token_t jiot_client_MQTT_publish (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic,
								void *payload,
								int payloadlen)
{
	return jiot_client_MQTT_publishWithQosAndRetained(handle, topic, payload,
			payloadlen, JIOT_QOS_DEFAULT, 0);
}

jiot_client_MQTT_token_t jiot_client_MQTT_publishWithQos (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic,
								void *payload,
								int payloadlen,
								jiot_client_MQTT_qos_t qos)
{
	return jiot_client_MQTT_publishWithQosAndRetained(handle, topic, payload,
			payloadlen, qos, 0);
}

jiot_client_MQTT_token_t jiot_client_MQTT_publishWithQosAndRetained (
								jiot_client_MQTT_Hndl_t handle,
								const char *topic,
								void *payload,
								int payloadlen,
								jiot_client_MQTT_qos_t qos,
								int retained)
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

	if (qos != JIOT_QOS_NONE) {
		token = jiot_client_mqtt_generate_token(client_handle);
	} else {
		token = 0;
	}
	client_handle->request.token = token;

	ret = mqtt_publish(client_handle->client, topic, payload, payloadlen, qos,
			retained, jiot_client_mqtt_pub_request_cb, handle);
	if (ret == ERR_OK) {
		return token;
	}

	return JIOT_MQTT_OPERATION_FAILURE;
}
