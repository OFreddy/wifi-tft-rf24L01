/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#include <Arduino.h>
//#include <TimeLib.h>
#include <sunset.h>

// August 1st, 2018
#define NTP_MIN_VALID_EPOCH 1659312000

typedef enum TimeState
{
    TimeState_Idle,
    TimeState_Init,
    TimeState_Valid

} E_TIMESTATE;

class suntime
{
public:
    suntime();
    ~suntime();

    void setup(double lat, double lon, int tz, int gmtoffset, int dstoffset, const char* tsname);
    time_t loop();
    time_t offsetDayLightSaving (uint32_t local_t);

    int sunrise(void);
    int sunset(void);
    int moonphase(void);

protected:
private:
    // NTP Service and sunset
    TimeState timeState;
    SunSet *sun;
    int mSunrise, mSunset, mMoonphase;
    int timezone;
    int currentday;

    void calcSun(struct tm *timeinfo);
};

