/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include <string.h>
#include <time.h>

#include "_dbg.h"

#include "TimeLib.h"
#include "ms_ticker.h"
#include "config.h"

#include "SunsetClass.h"

#ifdef ESP8266
static bool getLocalTime(struct tm *info, uint32_t ms)
{
    uint32_t start = millis();
    time_t now;
    while ((millis() - start) <= ms)
    {
        time(&now);
        localtime_r(&now, info);
        if (info->tm_year > (2016 - 1900))
        {
            return true;
        }
        delay(10);
    }
    return false;
}
#endif

typedef enum TimeState
{
    TimeState_Idle,
    TimeState_Init,
    TimeState_Valid

} E_TIMESTATE;

SunsetClass::SunsetClass()
{
    _initialized = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SunsetClass::init()
{
    setLocation();
    _isDayTime = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SunsetClass::loop()
{
    if (!ConfigInst.get().Sunset_Enabled)
    {
        if (_initialized)
        {
            _isDayTime = true;
            _currentDay = -1;
            _sunriseMinutes = _sunsetMinutes = 0;
            _initialized = false;
        }
        return;
    }
    if (!_initialized)
        setLocation();

    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 5))
    { // Time is not valid
        _isDayTime = true;
        _currentDay = -1;
        _sunriseMinutes = _sunsetMinutes = 0;
        return;
    }

    if (_currentDay != timeinfo.tm_mday)
    {
        DBG_PRINTF("SUNSET currentday: %i day: %i\n", _currentDay, timeinfo.tm_mday);
        _currentDay = timeinfo.tm_mday;
        _sunSet.setCurrentDate(1900 + timeinfo.tm_year, timeinfo.tm_mon + 1, timeinfo.tm_mday);

        // If you have daylight savings time, make sure you set the timezone appropriately as well
        _sunSet.setTZOffset(_timezoneOffset + (timeinfo.tm_isdst != 0 ? 1 : 0));
        _sunriseMinutes = (int)_sunSet.calcSunrise();
        _sunsetMinutes = (int)_sunSet.calcSunset();

        int secondsPastMidnight = timeinfo.tm_hour * 60 + timeinfo.tm_min;
        _isDayTime = (secondsPastMidnight >= (_sunriseMinutes + ConfigInst.get().Sunset_Sunriseoffset)) && (secondsPastMidnight < (_sunsetMinutes + ConfigInst.get().Sunset_Sunsetoffset));

        DBG_PRINTF("SUNSET offset: %u Sunrise: %u Sunset: %u\n", _timezoneOffset, _sunriseMinutes, _sunsetMinutes);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SunsetClass::setLocation()
{
    _latitude = strtod(ConfigInst.get().Sunset_Latitude, NULL);
    _longitude = strtod(ConfigInst.get().Sunset_Longitude, NULL);

    // Set default values
    _currentDay = -1;
    _isDayTime = true;
    _sunriseMinutes = _sunsetMinutes = 0;

    // Get timezone offset
    struct tm dt;
    memset(&dt, 0, sizeof(struct tm));
    dt.tm_mday = 1;
    dt.tm_year = 70;
    time_t tzlag = mktime(&dt);
    _timezoneOffset = -tzlag / 3600;

    _sunSet.setPosition(_latitude, _longitude, (double)_timezoneOffset);

    _initialized = true;
}

int SunsetClass::getTimezoneOffset()
{
    return _timezoneOffset;
}

int SunsetClass::getSunriseMinutes()
{
    return _sunriseMinutes;
}

int SunsetClass::getSunsetMinutes()
{
    return _sunsetMinutes;
}

bool SunsetClass::isDayTime()
{
    return _isDayTime;
}

SunsetClass SunsetClassInst;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
