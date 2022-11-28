/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
*/

#pragma once

#define BITS_TO_BYTES(x)  (((x)+7)>>3)
#define BYTES_TO_BITS(x)  ((x)<<3)

extern uint16_t HM_crc16(uint8_t* buf, const uint16_t bufLen, const uint16_t startCRC, const uint16_t startBit, const uint16_t len_bits);


