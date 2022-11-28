/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include "_dbg.h"

#include "TZinfo.h"
#include "TimeLib.h"
#include "ms_ticker.h"
#include "suntime.h"

suntime::suntime()
{
    sun = new SunSet();

    timeState = TimeState_Idle;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

suntime::~suntime()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void suntime::setup(double lat, double lon, int tz, int gmtoffset, int dstoffset, const char *tsname)
{
    timezone = tz;

    // Sunset / dawn calculation
    sun->setPosition(lat, lon, tz);

    DBG_PRINTF(DBG_PSTR("Configuring time for timezone %i\r\n"), tz);
    // configTime(gmtoffset, dstoffset, tsname);
    configTime(getTzInfo("Europe/Berlin").c_str(), tsname);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

time_t suntime::loop(void)
{
    time_t now = 0;

    if (timeState == TimeState_Idle)
    {
        timeState = TimeState_Init;
    }
    else if (timeState == TimeState_Init)
    {
        if ((now = time(nullptr)) >= NTP_MIN_VALID_EPOCH)
        {
            DBG_PRINTF(DBG_PSTR("Local time: %s\r\n"), asctime(localtime(&now))); // print formated local time, same as ctime(&now)
            DBG_PRINTF(DBG_PSTR("UTC time:   %s\r\n"), asctime(gmtime(&now)));    // print formated GMT/UTC time
            currentday = -1;
            timeState = TimeState_Valid;
        }
    }
    else if (timeState == TimeState_Valid)
    {
        now = time(nullptr);
        struct tm *timeinfo = localtime(&now);

        if (currentday != timeinfo->tm_mday)
        {
            currentday = timeinfo->tm_mday;
            calcSun(timeinfo);
        }
    }
    return now;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int suntime::sunrise(void) { return mSunrise; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int suntime::sunset(void) { return mSunset; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int suntime::moonphase(void) { return mMoonphase; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void suntime::calcSun(struct tm *timeinfo)
{
    sun->setCurrentDate(1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday);

    // If you have daylight savings time, make sure you set the timezone appropriately as well
    sun->setTZOffset(timezone + (timeinfo->tm_isdst != 0 ? 1 : 0));
    mSunrise = (int)sun->calcSunrise();
    mSunset = (int)sun->calcSunset();
    mMoonphase = sun->moonPhase(std::time(nullptr));

    DBG_PRINTF(DBG_PSTR("Sun phase for %04u-%02u-%02u ********\r\n"), 1900 + timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    DBG_PRINTF(DBG_PSTR("Time: %02u:%02u:%02u dst: %u\r\n"), timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec, timeinfo->tm_isdst);
    DBG_PRINTF(DBG_PSTR("Sunrise: %i %02u:%02u\r\n"), mSunrise, mSunrise / 60, mSunrise % 60);
    DBG_PRINTF(DBG_PSTR("Sunset:  %i %02u:%02u\r\n"), mSunset, mSunset / 60, mSunset % 60);
    DBG_PRINTF(DBG_PSTR("Moon:    %i\r\n"), mMoonphase);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculates the daylight saving time for middle Europe. Input: Unixtime in UTC
// from: https://forum.arduino.cc/index.php?topic=172044.msg1278536#msg1278536
time_t suntime::offsetDayLightSaving(uint32_t local_t)
{
    int m = month(local_t);
    if (m < 3 || m > 10)
        return 0; // no DSL in Jan, Feb, Nov, Dez
    if (m > 3 && m < 10)
        return 1; // DSL in Apr, May, Jun, Jul, Aug, Sep
    int y = year(local_t);
    int h = hour(local_t);
    int hToday = (h + 24 * day(local_t));
    if ((m == 3 && hToday >= (1 + TIMEZONE + 24 * (31 - (5 * y / 4 + 4) % 7))) || (m == 10 && hToday < (1 + TIMEZONE + 24 * (31 - (5 * y / 4 + 1) % 7))))
        return 1;
    else
        return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
