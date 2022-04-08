// Copyright 2022 ROBOTIS CO., LTD.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Dynamixel2Arduino.h>

#define DXL_BUS Serial1
#define USB Serial
#define DXL_PACKET_BUFFER_LENGTH  1024

uint8_t packet_buffer[DXL_PACKET_BUFFER_LENGTH];
unsigned long led_update_time = 0;

Dynamixel2Arduino dxl(DXL_BUS);

void setup() {
  // put your setup code here, to run once:
  pinMode(LED_BUILTIN, OUTPUT);
  // Use UART port of DYNAMIXEL Shield to debug.
  USB.begin(57600);
  
  // Set Port baudrate to 57600bps. This has to match with DYNAMIXEL baudrate.
  dxl.begin(USB.baud());
  // Set Port Protocol Version. This has to match with DYNAMIXEL protocol version.
}

void loop() {
  // put your main code here, to run repeatedly:
  dataTransceiver();

  if(USB.baud() != dxl.getPortBaud()) {
    dxl.begin(USB.baud());
  }
}

void dataTransceiver()
{
  int length = 0;
  int i = 0;

  // USB -> DXL
  length = USB.available();
  if( length > 0 )
  {
    for(i = 0; i < length; i++)
    {
      DXL_BUS.write(USB.read());
    }
    ledStatus();
  }

  // DXL -> USB
  length = DXL_BUS.available();
  if( length > 0 )
  {
    if( length > DXL_PACKET_BUFFER_LENGTH )
    {
      length = DXL_PACKET_BUFFER_LENGTH;
    }
    for(i = 0; i < length; i++)
    {
      packet_buffer[i] = DXL_BUS.read();
    }
    USB.write(packet_buffer, length);
    ledStatus();
  }
}

void ledStatus()
{
  if((millis() - led_update_time) > 200 )
  {
    digitalWrite(LED_BUILTIN, 1);
    led_update_time = millis();
  } else {
    digitalWrite(LED_BUILTIN, 0);
  }
}