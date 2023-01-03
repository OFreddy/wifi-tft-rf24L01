/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#include <Arduino.h>
#include <sunset.h>

// August 1st, 2018
#define NTP_MIN_VALID_EPOCH 1659312000

class SunsetClass
{
public:
    SunsetClass();

    void setup(char *lat, char *lon, const char *tsname);
    void setup(double lat, double lon,  const char* tsname);
    void init();
    void loop();
    time_t offsetDayLightSaving (uint32_t local_t);

    void setLocation();
    int getTimezoneOffset();
    int getSunriseMinutes();
    int getSunsetMinutes();
    bool isDayTime();

private:
    SunSet _sunSet;
    bool _initialized;
    double _latitude;
    double _longitude;
    int _currentDay;
    int _timezoneOffset;
    bool _isDayTime;
    int _sunriseMinutes;
    int _sunsetMinutes;
};

extern SunsetClass SunsetClassInst;

