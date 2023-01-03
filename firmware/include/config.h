/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#include <Arduino.h>
#include <FS.h>
#define SPIFFS LittleFS
#include <LittleFS.h>

#define CONFIG_FILENAME "/config.json"
#define CONFIG_VERSION 0x00010100 // 0.01.01 // make sure to clean all after change

#define WIFI_MAX_SSID_STRLEN 31
#define WIFI_MAX_PASSWORD_STRLEN 64
#define WIFI_MAX_HOSTNAME_STRLEN 31

#define NTP_MAX_SERVER_STRLEN 31
#define NTP_MAX_TIMEZONE_STRLEN 50

#define SUNSET_MAX_LONGLAT_STRLEN 10

#define JSON_BUFFER_SIZE 1200

struct CONFIG_T
{
    uint32_t Cfg_Version;
    uint16_t Cfg_SaveCount;

    // Wifi configuration
    char WiFi_Ssid1[WIFI_MAX_SSID_STRLEN + 1];
    char WiFi_Password1[WIFI_MAX_PASSWORD_STRLEN + 1];
    char WiFi_Ssid2[WIFI_MAX_SSID_STRLEN + 1];
    char WiFi_Password2[WIFI_MAX_PASSWORD_STRLEN + 1];
    char WiFi_Hostname[WIFI_MAX_HOSTNAME_STRLEN + 1];

    // NTP
    char Ntp_Server[NTP_MAX_SERVER_STRLEN + 1];
    uint16_t Ntp_Port;
    char Ntp_TimezoneDescr[NTP_MAX_TIMEZONE_STRLEN + 1];

    // Sunrise / Sunset settings
    bool Sunset_Enabled;
    char Sunset_Latitude[SUNSET_MAX_LONGLAT_STRLEN + 1];
    char Sunset_Longitude[SUNSET_MAX_LONGLAT_STRLEN + 1];
    int16_t Sunset_Sunriseoffset;
    int16_t Sunset_Sunsetoffset;


};

class ConfigClass
{
public:
    void init();
    void loop();
    bool read();
    bool write();
    CONFIG_T &get();
    void requestRestart();
    
    String load_from_file(String file_name);
    bool write_to_file(String file_name, String contents);

private:
    uint32_t secondTick;
    int16_t restartCount;

};

extern ConfigClass ConfigInst;