#ifndef JIO_MQTT_UTILS_H
#define JIO_MQTT_UTILS_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <lwip/apps/mqtt.h>
#include <lwip/ip4_addr.h>
#include <lwip/inet.h>

/* Jio MQTT typedef and structure definition */
#define JIOT_MQTTVERSION_DEFAULT			0
#define JIOT_MQTTVERSION_3_1 				3
#define JIOT_MQTTVERSION_3_1_1 				4

#define JIOT_MQTT_PERSISTENCE_NONE			0
#define JIOT_MQTT_PERSISTENCE_DEFAULT		1

typedef void* jiot_client_MQTT_Hndl_t;
typedef int jiot_client_MQTT_token_t;

typedef enum jiot_client_MQTT_qos_t {
	JIOT_QOS_NONE = 0,
	JIOT_QOS_DEFAULT = 1,
	JIOT_QOS_TWO = 2
} jiot_client_MQTT_qos_t;

/* Persistence Options */
typedef struct {
	int enablePersistence;
	int type;
	void *context;
	int maxBufferedMessages;
} jiot_client_MQTT_persistOpts_t;

/* Create Options */
typedef struct {
	char *clientId;
	jiot_client_MQTT_qos_t qos;
	int retained;
	char *username;
	char *password;
	char *host;
	char *port;
	int keepAlive;
	int useSSL;
	char *certPassword;
} jiot_client_MQTT_createOpts_t;

/* LWT Options */
typedef struct {
	const char *topicName;
	const char *message;
	int retained;
	jiot_client_MQTT_qos_t qos;
} jiot_client_MQTT_willOpts_t;

/* SSL Options */
typedef struct {
	const char *trustStore;
	const char *keyStore;
	const char *privateKey;
	const char *privateKeyPassword;
	const char *enabledCipherSuites;
	int enableServerCertAuth;
} jiot_client_MQTT_sslOpts_t;

/* Connect Options */
typedef struct {
	int keepAliveInterval;
	int cleanSession;
	int maxInflight;
	int connectTimeout;
	int retryInterval;
	int mqttVersion;
	int autoReconnect;
	int minRetryInterval;
	int maxRetryInterval;
	jiot_client_MQTT_willOpts_t *willOptions;
	jiot_client_MQTT_sslOpts_t *sslOptions;
} jiot_client_MQTT_connectOpts_t;

/* MQTT Message Structure */
typedef struct {
	int payloadlen;
	void* payload;
	jiot_client_MQTT_qos_t qos;
	int retained;
	int dup;
	int msgid;
} jiot_client_MQTT_message_t;

/* Call back Options */
typedef void jiot_client_MQTT_message_deliveryComplete (
				void *context,
				jiot_client_MQTT_token_t token);
typedef void jiot_client_MQTT_message_deliveryFailed (
				void *context,
				const char *cause,
				jiot_client_MQTT_token_t token);
typedef void jiot_client_MQTT_connectionFailed (
				void *context,
				const char *cause,
				int code);
typedef void jiot_client_MQTT_connectionComplete (
				void *context,
				jiot_client_MQTT_token_t token);
typedef void jiot_client_MQTT_autoReconnectComplete (void *context);
typedef void jiot_client_MQTT_disconnectionComplete (void *context);
typedef void jiot_client_MQTT_disconnectionFailed (
				void *context,
				const char *cause,
				int code);
typedef void jiot_client_MQTT_subscribeComplete (
				void *context,
				jiot_client_MQTT_token_t token,
				jiot_client_MQTT_qos_t qos);
typedef void jiot_client_MQTT_subscribeManyComplete (
				void *context,
				jiot_client_MQTT_token_t token,
				int *qos);
typedef void jiot_client_MQTT_subscribeFailed (
				void *context,
				const char *cause,
				int code,
				jiot_client_MQTT_token_t token);
typedef void jiot_client_MQTT_messageArrived (
				void *context,
				char *topic,
				int topicLen,
				jiot_client_MQTT_message_t *message);
typedef void jiot_client_MQTT_unsubscribeComplete (
				void *context,
				jiot_client_MQTT_token_t token);
typedef void jiot_client_MQTT_unsubscribeFailed (
				void *context,
				const char *cause,
				int code,
				jiot_client_MQTT_token_t token);

typedef struct {
	jiot_client_MQTT_message_deliveryComplete *deliveryComplete;
	jiot_client_MQTT_message_deliveryFailed *deliveryFailed;
	jiot_client_MQTT_connectionComplete *connectionComplete;
	jiot_client_MQTT_connectionFailed *connectionFailed;
	jiot_client_MQTT_autoReconnectComplete *autoReconnectComplete;
	jiot_client_MQTT_disconnectionComplete *disconnectComplete;
	jiot_client_MQTT_disconnectionFailed *disconnectFailed;
	jiot_client_MQTT_subscribeComplete *subscribeComplete;
	jiot_client_MQTT_subscribeManyComplete *subscribeManyComplete;
	jiot_client_MQTT_subscribeFailed *subscribeFailed;
	jiot_client_MQTT_messageArrived *messageArrived;
	jiot_client_MQTT_unsubscribeComplete *unsubscribeComplete;
	jiot_client_MQTT_unsubscribeFailed *unsubscribeFailed;
} jiot_client_MQTT_callbackOpts_t;

/* Error codes */
#define JIOT_MQTT_OPERATION_SUCCESS					0
#define JIOT_MQTT_OPERATION_FAILURE					-1
#define JIOT_MQTT_OPERATION_DISCONNECTED			-3
#define JIOT_MQTT_OPERATION_MAX_MESSAGES_INFLIGHT	-4
#define JIOT_MQTT_OPERATION_BAD_UTF8_STRING			-5
#define JIOT_MQTT_OPERATION_NULL_PARAMETER			-6
#define JIOT_MQTT_OPERATION_TOPICNAME_TRUNCATED		-7
#define JIOT_MQTT_OPERATION_BAD_QOS					-9
#define JIOT_MQTT_OPERATION_NO_MORE_MSGIDS			-10
#define JIOT_MQTT_REQUEST_OPERATION_INCOMPLETE		-11
#define JIOT_MQTT_OPERATION_MAX_BUFFERED_MESSAGES	-12
#define JIOT_MQTT_OPERATION_SSL_NOT_SUPPORTED		-13
#define JIOT_MQTT_OPERATION_BAD_PROTOCOL			-14

/* Jio MQTT API*/
int jiot_client_MQTT_create (jiot_client_MQTT_Hndl_t *handle,
							jiot_client_MQTT_createOpts_t options);
int jiot_client_MQTT_createWithPersistence (
							jiot_client_MQTT_Hndl_t *handle,
							jiot_client_MQTT_createOpts_t options,
							jiot_client_MQTT_persistOpts_t persistOpts);
void jiot_client_MQTT_setcallbacks (
							jiot_client_MQTT_Hndl_t handle,
							void *context,
							jiot_client_MQTT_callbackOpts_t callbacks);
int jiot_client_MQTT_connect (jiot_client_MQTT_Hndl_t handle);
int jiot_client_MQTT_connectWithOptions (
							jiot_client_MQTT_Hndl_t handle,
							jiot_client_MQTT_connectOpts_t *options);
int jiot_client_MQTT_isConnected (jiot_client_MQTT_Hndl_t handle);
jiot_client_MQTT_token_t jiot_client_MQTT_publish (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic,
							void *payload,
							int payloadlen);
jiot_client_MQTT_token_t jiot_client_MQTT_publishWithQos (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic,
							void *payload,
							int payloadlen,
							jiot_client_MQTT_qos_t qos);
jiot_client_MQTT_token_t jiot_client_MQTT_publishWithQosAndRetained (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic,
							void *payload,
							int payloadlen,
							jiot_client_MQTT_qos_t qos,
							int retained);
jiot_client_MQTT_token_t jiot_client_MQTT_subscribe (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic);
jiot_client_MQTT_token_t jiot_client_MQTT_subscribeWithQos (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic,
							jiot_client_MQTT_qos_t qos);
jiot_client_MQTT_token_t jiot_client_MQTT_subscribeMany (
							jiot_client_MQTT_Hndl_t handle,
							int count,
							char *const *topic);
jiot_client_MQTT_token_t jiot_client_MQTT_subscribeManyWithQos (
							jiot_client_MQTT_Hndl_t handle,
							int count,
							char *const *topic,
							jiot_client_MQTT_qos_t *qos);
jiot_client_MQTT_token_t jiot_client_MQTT_unsubscribe (
							jiot_client_MQTT_Hndl_t handle,
							const char *topic);
jiot_client_MQTT_token_t jiot_client_MQTT_unsubscribeMany (
							jiot_client_MQTT_Hndl_t handle,
							int length,
							char *const *topics);
int jiot_client_MQTT_getPendingTokens (
							jiot_client_MQTT_Hndl_t handle,
							jiot_client_MQTT_token_t **tokens);
int jiot_client_MQTT_disconnect (jiot_client_MQTT_Hndl_t handle);
void jiot_client_MQTT_destroy (jiot_client_MQTT_Hndl_t *handle);

#endif /* JIO_MQTT_H */