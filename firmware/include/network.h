
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

enum NetworkState_E
{
    eNetworkStateInit,
    eNetworkstateConnect,
    eNetworkstateConnected,
    eNetworkstateDisconnected,
};

class NetworkClass
{
public:
    NetworkClass();
    ~NetworkClass();

    void setup();
    void loop();

private:
    String hostname;

    NetworkState_E networkState;
    TimeoutHelper connecTimeout;

    void statemachine();

    void WifiSetMode(WiFiMode_t wifi_mode);
    void WifiSetSleepMode(WiFiMode_t wifi_mode);
    void WifiSetOutputPower(void);
};

extern NetworkClass NetworkInst;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
