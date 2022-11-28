/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include <Arduino.h>

#include "_dbg.h"

#include "TimeoutHelper.h"
#include "network.h"

#include "user_config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass::NetworkClass()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass::~NetworkClass()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::setup()
{
    networkState = eNetworkStateInit;

    hostname = "Olli";

#if defined(ESP8266) || defined(ESP32)
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    WiFi.setSleep(true);
    // Set and check host name
    escapedMac = WiFi.macAddress();
    escapedMac.replace(":", "");
    escapedMac.toLowerCase();
    prepareHostname(hostname, WIFI_HOST);
    WiFi.setHostname(hostname);
#if defined(ESP32)
    WiFi.setTxPower(WIFI_POWER_17dBm);
#endif
    oldWifiStatus = WL_NO_SHIELD;
    DBG_PRINTF(DBG_PSTR("Setting hostname '%s'\r\n"), hostname);
    DBG_PRINTF(DBG_PSTR("Connecting to '%s'\r\n"), WIFI_SSID);
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::loop()
{
    statemachine();

    if (WiFi.status() != oldWifiStatus)
    {
        oldWifiStatus = WiFi.status();
        if (WiFi.status() == WL_CONNECTED)
        {
            DBG_PRINTF(DBG_PSTR("WiFi connected. IP address: %s\r\n"), WiFi.localIP().toString().c_str());
        }
        else
        {
            DBG_PRINTF(DBG_PSTR("WiFi status changed to %u\n"), WiFi.status());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::statemachine()
{
    switch (networkState)
    {
    case eNetworkStateInit:
        break;

    case eNetworkstateConnect:
        break;

    case eNetworkstateDisconnected:
        break;

    default:
        networkState = eNetworkStateInit;
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetMode(WiFiMode_t wifi_mode)
{
    if (WiFi.getMode() == wifi_mode)
        return;

    if (wifi_mode != WIFI_OFF)
    {
        WiFi.hostname(hostname); // ESP32 needs this here (before WiFi.mode) for core 2.0.0
        // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
        WiFi.forceSleepWake(); // Make sure WiFi is really active.
        delay(100);
    }

    uint32_t retry = 2;
    while (!WiFi.mode(wifi_mode) && retry--)
    {
        DBG_PRINTF(DBG_PSTR("Retry set Mode..."));
        delay(100);
    }

    if (wifi_mode == WIFI_OFF)
    {
        delay(1000);
        WiFi.forceSleepBegin();
        delay(1);
    }
    else
    {
        delay(30); // Must allow for some time to init.
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetSleepMode(WiFiMode_t wifi_mode)
{
    bool wifi_no_sleep = true; // TODO: Move to settings
    bool wifi_normal_sleep = false;

    if (wifi_no_sleep)
    {
        WiFi.setSleepMode(WIFI_NONE_SLEEP); // Disable sleep
    }
    else
    {
        if (wifi_normal_sleep)
            WiFi.setSleepMode(WIFI_LIGHT_SLEEP); // Allow light sleep during idle times
        else
            WiFi.setSleepMode(WIFI_MODEM_SLEEP); // Sleep (Esp8288/Arduino core and sdk default)
    }
    WifiSetOutputPower();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetOutputPower(void)
{
    // Set output power in thre range from max: +20.5dBm  to min: 0dBm
    WiFi.setOutputPower((float)20.5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass NetworkInst; // Network class instance singleton

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
