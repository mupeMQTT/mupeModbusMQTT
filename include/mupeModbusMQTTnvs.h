
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

#ifndef MUPEMODBUSNVS_H
#define MUPEMODBUSNVS_H
#include "esp_system.h"

typedef struct ModbusNvs{
	uint64_t id;
  char parameterName[20];
  char hostname[20];
  uint8_t unitId;
  uint16_t adress;
  uint8_t size;
  uint16_t portNr;
} ModbusNvs;

void mupeModbusNvsInit(void);


uint8_t intervallGet();
void intervallSet(uint8_t intervall);
size_t modbusNvsGetStrSize(ModbusNvs *modbus) ;
ModbusNvs* modbusNvsGet(ModbusNvs *modbus);
void modbusNvsSet(ModbusNvs *modbus);

#endif
