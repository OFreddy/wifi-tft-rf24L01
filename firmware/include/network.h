/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#if defined(ESP32)
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#endif

#include <WiFiUdp.h>
#include <DNSServer.h>

#include "TimeoutHelper.h"

enum NetworkState_E
{
    eNetworkStateInit,
    eNetworkstateSTAConnect,
    eNetworkstateSTAConnected,
    eNetworkstateSTADisconnected,
    eNetworkstateAP,
    eNetworkstateNext,
};

enum NetworkConfig_E : int8_t
{
    eNetworkConfigUse1,
    eNetworkConfigUse2,
    eNetworkConfigAP,
    eNetworkConfigIll
};


class NetworkClass
{
public:
    NetworkClass();
    ~NetworkClass();

    void setup();
    void loop();

    bool IsConnected(void);
    NetworkState_E GetNetworkState();
    int8_t GetWifiQuality(void);
    String GetStationSSID(void);

private:
    bool mApActive = false;
    bool mWifiSet1Valid = false;
    bool mWifiSet2Valid = false;

    // Access point mode
    DNSServer *mDnsServer;
    WiFiUDP *mUdp;

    // Channel mode
    String mSTA_SSIDName;
    String mSTA_Pass;
    String hostname;
    wl_status_t mOldWifiStatus;

    NetworkConfig_E networkConfig;
    NetworkState_E networkState;
    TimeoutHelper connectTimeout;

    void statemachine();
    void chooseconfig();

    void WifiBeginSTA(void);
    void WifiBeginAP(const char *ssid, const char *pwd);

    void WifiSetMode(WiFiMode_t wifi_mode);
    void WifiSetSleepMode(void);
    void WifiSetOutputPower(void);
};

extern NetworkClass NetworkInst;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
