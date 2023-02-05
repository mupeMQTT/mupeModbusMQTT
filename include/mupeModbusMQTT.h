#ifndef MUPEMODBUS_H
#define MUPEMODBUS_H
/* BSD Socket API Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"




uint64_t mupeModbusReadU64(char *hostname, uint8_t unitIdentifier,uint16_t dataAddress,int port);

uint32_t mupeModbusReadU32(char *hostname, uint8_t unitIdentifier,uint16_t dataAddress,int port);

uint8_t* mupeModbusRead(char *hostname, uint8_t unitIdentifier,uint16_t dataAddress,uint16_t messageLength,int port) ;

void mupeModbusInit( );


#endif


