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

#define JSON_BUFFER_SIZE 1200


struct CONFIG_T {
    uint32_t Cfg_Version;
    uint16_t Cfg_SaveCount;

    char WiFi_Ssid1[WIFI_MAX_SSID_STRLEN + 1];
    char WiFi_Password1[WIFI_MAX_PASSWORD_STRLEN + 1];
    char WiFi_Ssid2[WIFI_MAX_SSID_STRLEN + 1];
    char WiFi_Password2[WIFI_MAX_PASSWORD_STRLEN + 1];
    char WiFi_Hostname[WIFI_MAX_HOSTNAME_STRLEN + 1];

};

class ConfigClass
{
    public:
        void init();
        bool read();
        bool write();
        CONFIG_T& get();


    private:
    String load_from_file(String file_name);
    bool write_to_file(String file_name, String contents);

};

extern ConfigClass ConfigInst;