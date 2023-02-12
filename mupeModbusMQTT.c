// Copyright Peter Müller mupe
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to permit
// persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
// NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
// DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
// OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
// USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "mupeModbusMQTT.h"
#include "mupeMdnsNtp.h"
#include "mupeModbusMQTTweb.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <esp_wifi.h>

static const char *TAG = "Modbus";

uint16_t trId = 0;

typedef struct {
	uint16_t transactionIdentifier;
	uint16_t protocolIdentifier;
	uint16_t messageLength;
	uint8_t unitIdentifier;
	uint8_t functionCode;
	uint16_t dataAddress;
	uint16_t numberOfRegisters;
	uint8_t ret[16]; //13
} payloadST;

typedef struct {
	uint16_t transactionIdentifier; //2
	uint16_t protocolIdentifier; //4
	uint16_t messageLength; //6
	uint8_t unitIdentifier; //7
	uint8_t functionCode; //8
	uint8_t numberOfBits; //9
	uint8_t ret[16]; //13

} payloadSTk;

payloadST payload;
payloadSTk payloadRet;

TaskHandle_t xHandle = NULL;

void vTaskModbus() {
	TickType_t xLastWakeTime;
	// Block for ms.
	const TickType_t xFrequency = 100000 / portTICK_PERIOD_MS;
	waitForNTPConnect();
	xLastWakeTime = xTaskGetTickCount();
	xLastWakeTime = xLastWakeTime
			- (getNowMs() / portTICK_PERIOD_MS) % xFrequency;

	while (1) {
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
		ESP_LOGI(TAG, "MS-------------------- %llu", getNowMs());
		ESP_LOGI(TAG, "MS-------------------- %lu", xLastWakeTime);
	}
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	if (xHandle != NULL) {
		vTaskDelete(xHandle);
	}
}

static void connect_handler(void *arg, esp_event_base_t event_base,
		int32_t event_id, void *event_data) {
	if (xHandle == NULL) {
		xTaskCreatePinnedToCore(vTaskModbus, "vTaskModbus", 2096, NULL,
		tskIDLE_PRIORITY, &xHandle, 0);
	}
}

void mupeModbusInit() {
	mupeModbusWebInit();
	ESP_ERROR_CHECK(
			esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
					&connect_handler, &xHandle));

	ESP_ERROR_CHECK(
			esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
					&disconnect_handler, &xHandle));

	xTaskCreatePinnedToCore(vTaskModbus, "TaskModbus", 2096, NULL,
	tskIDLE_PRIORITY, &xHandle, 0);


}

uint64_t mupeModbusReadU64(char *hostname, uint8_t unitIdentifier,
		uint16_t dataAddress, int port) {
	uint8_t *ret;
	ret = mupeModbusRead(hostname, unitIdentifier, dataAddress, 8, port);

	return ret[0] * 0x100000000000000 + ret[1] * 0x1000000000000
			+ ret[2] * 0x10000000000 + ret[3] * 0x100000000 + ret[4] * 0x1000000
			+ ret[5] * 0x10000 + ret[6] * 0x100 + ret[7];
}

uint32_t mupeModbusReadU32(char *hostname, uint8_t unitIdentifier,
		uint16_t dataAddress, int port) {
	uint8_t *ret;
	ret = mupeModbusRead(hostname, unitIdentifier, dataAddress, 4, 502);
	if (ret[0] == 0x80)
		return 0;
	return ret[0] * 0x1000000 + ret[1] * 0x10000 + ret[2] * 0x100 + ret[3];
}

uint8_t* mupeModbusRead(char *hostname, uint8_t unitIdentifier,
		uint16_t dataAddress, uint16_t messageLength, int port) {
	int sock = -1;
	int addr_family = 0;
	int ip_protocol = 0;
	struct sockaddr_in dest_addr;
	messageLength = messageLength + 4;

	struct hostent *he;
	he = gethostbyname(hostname);
	dest_addr.sin_addr.s_addr = *((unsigned long*) he->h_addr);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);

	addr_family = AF_INET;
	ip_protocol = IPPROTO_IP;

	sock = socket(addr_family, SOCK_STREAM, ip_protocol);
	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
	} else {
		ESP_LOGI(TAG, "Socket created, connecting to %s:%d", hostname, port);
	}

	int err = connect(sock, (struct sockaddr*) &dest_addr, sizeof(dest_addr));
	if (err != 0) {
		ESP_LOGE(TAG, "Socket unable to connect: errno %d", errno);
	}

	payload.transactionIdentifier = htons(trId);
	trId++;
	payload.protocolIdentifier = htons(0);
	payload.messageLength = htons(messageLength);
	payload.unitIdentifier = unitIdentifier;
	payload.functionCode = 3;
	payload.dataAddress = htons(dataAddress);
	payload.numberOfRegisters = htons(2);

	err = send(sock, &payload, sizeof(payload), 0);
	if (err < 0) {
		ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);

	}
	vTaskDelay(pdMS_TO_TICKS(1000)); // Delay o bloqueo de la tarea

	int len = recv(sock, &payloadRet, sizeof(payloadRet), 0);
	// Error occurred during receiving
	if (len < 0) {
		ESP_LOGE(TAG, "recv failed: errno %d", errno);
	}
	// Data received
	else {
		ESP_LOGI(TAG, "Received %d bytes from %s:", len, hostname);
	}
	shutdown(sock, 0);
	close(sock);
	vTaskDelay(pdMS_TO_TICKS(100));

	ESP_LOGI(TAG, "ret: %x %x %x %x %x %x %x %x", payloadRet.ret[0],
			payloadRet.ret[1], payloadRet.ret[2], payloadRet.ret[3],
			payloadRet.ret[4], payloadRet.ret[5], payloadRet.ret[6],
			payloadRet.ret[7]);

	return payloadRet.ret;
}
