/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include <Esp.h>
#include "Utils.h"

uint32_t Utils::getChipId()
{
    uint32_t rawChipId = 0;

#ifdef ARDUINO_ARCH_ESP32
    rawChipId = ESP.getEfuseMac();

    uint32_t chipId = 0;
    for (int i = 0; i < 17; i += 8) {
        chipId |= ((rawChipId >> (40 - i)) & 0xff) << i;
    }

    return chipId;
#else
    rawChipId = ESP.getChipId();

    return rawChipId;
#endif
}

uint64_t Utils::generateDtuSerial()
{
    uint32_t chipId = getChipId();
    uint64_t dtuId = 0;

    // Product category (char 1-4): 1 = Micro Inverter, 999 = Dummy
    dtuId |= 0x199900000000;

    // Year of production (char 5): 1 equals 2015 so hard code 8 = 2022
    dtuId |= 0x80000000;

    // Week of production (char 6-7): Range is 1-52 s hard code 1 = week 1
    dtuId |= 0x0100000;

    // Running Number (char 8-12): Derived from the ESP chip id
    for (uint8_t i = 0; i < 5; i++) {
        dtuId |= (chipId % 10) << (i * 4);
        chipId /= 10;
    }

    return dtuId;
}