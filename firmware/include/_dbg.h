/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Target depending debug macro defines

#pragma once

// Set to 0 for debbuging console output
#define SERIAL_DEBUG (1)

#if defined(ARDUINO_AVR_NANO)
// Debugging output
#define DBG_PRINTF printf_P
#define DBG_PSTR(x) PSTR(x)

#elif defined(ESP8266)
// Debugging output
#define DBG_PRINTF printf_P
#define DBG_PSTR(x) PSTR(x)

#elif defined(ESP32)
#error Unsupported board selection
#else
// Todo: add more targets
#error Unsupported board selection
#endif


// Undefine platform dependend predefinitions when debugging is disabled
#if !SERIAL_DEBUG
#undef DBG_PRINTF
#undef DBG_PSTR

#define DBG_PRINTF(...)
#endif

