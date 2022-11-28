/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

///////////////////////////////////////////////////////////////////////////////
/// Definition of the port pins the peripherals are are connected to 

#if defined(ARDUINO_AVR_NANO)
#define RF1_CE_PIN (9)
#define RF1_CS_PIN (6)
#define RF2_CE_PIN (7)
#define RF2_CS_PIN (8)
#define RF1_IRQ_PIN (2)
#define RF2_IRQ_PIN (3)
#define LED_PIN_STATUS (A0)

#elif defined(ESP8266)
#define RF1_CE_PIN (2)
#define RF1_CS_PIN (15)
#define RF1_IRQ_PIN (0)

#else
#error Unsupported board selection
#endif
