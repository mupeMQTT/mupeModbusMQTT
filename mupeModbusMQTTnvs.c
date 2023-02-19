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

#include "nvs_flash.h"
#include "mupeModbusMQTTnvs.h"

#define NAMESPACE_NAME "Modbuscfg"
void mupeModbusNvsInit(void) {
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
}

uint8_t intervallGet() {
	nvs_handle_t my_handle;
	uint8_t intervall = 1;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_u8(my_handle, "intervall", &intervall);

	nvs_close(my_handle);
	return intervall;
}
void intervallSet(uint8_t intervall) {
	nvs_handle_t my_handle;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_u8(my_handle, "intervall", intervall);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

void mqttTopicSet(char * mqttTopic){
	nvs_handle_t my_handle;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_str(my_handle, "mqttTopic", mqttTopic);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

size_t mqttTopicGetSize(){
	nvs_handle_t my_handle;
	size_t strSize = 0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_str(my_handle, "mqttTopic", NULL, &strSize);
	nvs_commit(my_handle);
	nvs_close(my_handle);
	return strSize;
}

void mqttTopicGet(char *mqttTopic) {
	nvs_handle_t my_handle;
	size_t strSize = 0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_str(my_handle, "mqttTopic", NULL, &strSize);
	nvs_get_str(my_handle,"mqttTopic", mqttTopic, &strSize);
	nvs_close(my_handle);

}

size_t modbusNvsGetSize(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id, "%llu", modbus->id);
	size_t strSize = 0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_blob(my_handle, id, NULL, &strSize);
	nvs_close(my_handle);
	return strSize;
}

ModbusNvs* modbusNvsGet(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id, "%llu", modbus->id);
	size_t strSize = 0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_blob(my_handle, id, NULL, &strSize);
	nvs_get_blob(my_handle, id, modbus, &strSize);
	nvs_close(my_handle);
	return modbus;
}

void modbusNvsSet(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id, "%llu", modbus->id);
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_blob(my_handle, id, modbus, sizeof(ModbusNvs));
	nvs_commit(my_handle);
	nvs_close(my_handle);
}

void modbusNvsDel(char *id) {
	nvs_handle_t my_handle;

	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_erase_key(my_handle, id);
	nvs_commit(my_handle);
	nvs_close(my_handle);
}
nvs_iterator_t it = NULL;
void modbusNvsGetNext(ModbusNvs *modbus) {
	modbus->id = 0;
	esp_err_t res;
	if (it == NULL) {
		res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
				NVS_TYPE_BLOB, &it);
	} else {
		res = nvs_entry_next(&it);
	}
	if (res != ESP_OK) {
		nvs_release_iterator(it);
		it = NULL;
		return;
	}
	nvs_entry_info_t info;
	nvs_entry_info(it, &info);
	modbus->id = atoll(info.key);
	modbusNvsGet(modbus);

}

void sendModbusCfg(httpd_req_t *req) {

	nvs_iterator_t it = NULL;
	char value[82];
	esp_err_t res = nvs_entry_find(NVS_DEFAULT_PART_NAME, NAMESPACE_NAME,
			NVS_TYPE_BLOB, &it);
	while (res == ESP_OK) {
		nvs_entry_info_t info;
		nvs_entry_info(it, &info); // Can omit error check if parameters are guaranteed to be non-NULL
		ModbusNvs modbus;
		modbus.id = atoll(info.key);
		modbusNvsGet(&modbus);
		sprintf(value, "<tr><th>%s</th><th>%s</th><th>%hhu</th>",
				modbus.parameterName, modbus.hostname, modbus.unitId);
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value, "<th>%i</th><th>%i</th><th>%i</th>", modbus.adress,
				modbus.size, modbus.portNr);
		httpd_resp_send_chunk(req, value, strlen(value));
		sprintf(value,
				"<th><button onclick=\"onClickDel(%llu)\">del</button></th></tr>",
				modbus.id);
		httpd_resp_send_chunk(req, value, strlen(value));

		res = nvs_entry_next(&it);
	}
	nvs_release_iterator(it);

}

