
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
void mupeModbusNvsInit(void){
	esp_err_t err = nvs_flash_init();
	if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		err = nvs_flash_init();
	}
}


uint8_t  intervallGet() {
	nvs_handle_t my_handle;
	uint8_t intervall=0;
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

size_t modbusNvsGetStrSize(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id,"%llu",modbus->id);
	size_t strSize=0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_blob(my_handle, id, NULL, &strSize);
	nvs_close(my_handle);
	return strSize;
}

ModbusNvs* modbusNvsGet(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id,"%llu",modbus->id);
	size_t strSize=0;
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_get_blob(my_handle, id, NULL, &strSize);
	nvs_get_blob(my_handle, id, modbus, &strSize);
	nvs_close(my_handle);
	return modbus;
}
void modbusNvsSet(ModbusNvs *modbus) {
	nvs_handle_t my_handle;
	char id[20];
	sprintf(id,"%llu",modbus->id);
	nvs_open(NAMESPACE_NAME, NVS_READWRITE, &my_handle);
	nvs_set_blob(my_handle, id, modbus,sizeof(modbus));
	nvs_commit(my_handle);
	nvs_close(my_handle);
}
