/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include <stdio.h>
#include <stdint.h>
#include "hm_crc.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// NRF24 CRC16 calculation with poly 0x1021 = (1) 0001 0000 0010 0001 = x^16+x^12+x^5+1
// Description: Due to the 9 bit PCF field in Shockburst frames the crc caclulation must be bit orientated
uint16_t HM_crc16(uint8_t *buf, const uint16_t bufLen, const uint16_t startCRC, const uint16_t startBit, const uint16_t len_bits)
{
	uint16_t crc = startCRC;
	if ((len_bits > 0) && (len_bits <= BYTES_TO_BITS(bufLen)))
	{
		// The length of the data might not be a multiple of full bytes.
		// Therefore we proceed over the data bit-by-bit (like the NRF24 does) to
		// calculate the CRC.
		uint16_t data;
		uint8_t byte, shift;
		uint16_t bitoffs = startBit;

		// Get a new byte for the next 8 bits.
		byte = buf[bitoffs >> 3];
		while (bitoffs < len_bits + startBit)
		{
			shift = bitoffs & 7;
			// Shift the active bit to the position of bit 15
			data = ((uint16_t)byte) << (8 + shift);

			// Assure all other bits are 0
			data &= 0x8000;
			crc ^= data;
			if (crc & 0x8000)
			{
				crc = (crc << 1) ^ 0x1021; // 0x1021 = (1) 0001 0000 0010 0001 = x^16+x^12+x^5+1
			}
			else
			{
				crc = (crc << 1);
			}
			++bitoffs;
			if (0 == (bitoffs & 7))
			{
				// Get a new byte for the next 8 bits.
				byte = buf[bitoffs >> 3];
			}
		}
	}
	return crc;
}