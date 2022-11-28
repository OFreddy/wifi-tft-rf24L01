/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#include <stdint.h>
#include <Arduino.h>

inline bool checkTicker(uint32_t *ticker, uint32_t interval)
{
	uint32_t mil = millis();
	if (mil >= *ticker)
	{
		*ticker = mil + interval;
		return true;
	}
	else if (mil < (*ticker - interval))
	{
		*ticker = mil + interval;
		return true;
	}

	return false;
}

inline void retriggerTicker(uint32_t *ticker, uint32_t interval = 0)
{
	uint32_t mil = millis();
	*ticker = mil + interval;
}

