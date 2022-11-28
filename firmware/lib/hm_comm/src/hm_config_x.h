/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Target depending library defines

#pragma once

#include <Arduino.h>
#include <stdint.h>

// Set to 0 for debbuging console output
#define SERIAL_DEBUG (1)

#if defined(ARDUINO_AVR_NANO)

// Macros
#define HM_DISABLE_EINT (EIMSK = 0x00) // Disable internal intterrupt
#define HM_ENABLE_EINT (EIMSK = 0x01)  // Enable external interrupt

#define HM_TICKCOUNTTYPE uint32_t  // Data type for tickcounter
#define HM_GETTICKCOUNT (millis()) // Get tick counter; should be a millisecond timebase

// Debugging output
#define HM_PRINTF printf_P
#define HM_PSTR(x) PSTR(x)

#elif defined(ESP8266)

// Macros
#define HM_DISABLE_EINT noInterrupts() // Disable internal intterrupt
#define HM_ENABLE_EINT interrupts()  // Enable external interrupt

#define HM_TICKCOUNTTYPE uint32_t  // Data type for tickcounter
#define HM_GETTICKCOUNT (millis()) // Get tick counter; should be a millisecond timebase

// Debugging output
#define HM_PRINTF printf_P
#define HM_PSTR(x) PSTR(x)

#elif defined(ESP32)
#error Unsupported board selection
#else
// Todo: add more targets
#error Unsupported board selection
#endif


// Undefine platform dependend predefinitions when debugging is disabled
#if !SERIAL_DEBUG
#undef HM_PRINTF
#undef HM_PSTR

#define HM_PRINTF(...)
#endif

