/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include "config.h"
#include <string.h>
#include <ArduinoJson.h>

#include "ms_ticker.h"
#include "_dbg.h"

CONFIG_T config;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigClass::init()
{
    memset(&config, 0x00, sizeof(config));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigClass::loop()
{
    if (checkTicker(&secondTick, 1000))
    {
        if (restartCount > 0)
        {
            if ((restartCount--) == 1)
            {
                ESP.restart();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ConfigClass::requestRestart()
{
    restartCount = 2;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigClass::write()
{
    File f = LittleFS.open(CONFIG_FILENAME, "w");

    if (!f)
        return false;
    config.Cfg_SaveCount++;

    DynamicJsonDocument doc(JSON_BUFFER_SIZE);

    JsonObject cfg = doc.createNestedObject("cfg");
    cfg["version"] = config.Cfg_Version;
    cfg["save_count"] = config.Cfg_SaveCount;

    JsonObject wifi = doc.createNestedObject("wifi");
    wifi["ssid1"] = config.WiFi_Ssid1;
    wifi["password1"] = config.WiFi_Password1;
    wifi["ssid2"] = config.WiFi_Ssid2;
    wifi["password2"] = config.WiFi_Password2;

    JsonObject ntp = doc.createNestedObject("ntp");
    ntp["server"] = config.Ntp_Server;
    ntp["port"] = config.Ntp_Port;
    ntp["timezone"] = config.Ntp_TimezoneDescr;

    JsonObject sunset = doc.createNestedObject("sunset");
    sunset["enabled"] = config.Sunset_Enabled;
    sunset["latitude"] = config.Sunset_Latitude;
    sunset["longitude"] = config.Sunset_Longitude;
    sunset["sunrise_offset"] = config.Sunset_Sunriseoffset;
    sunset["sunset_offset"] = config.Sunset_Sunsetoffset ;  

    // Serialize JSON to file
    if (serializeJson(doc, f) == 0)
    {
        DBG_PRINTF(DBG_PSTR("Failed to write file"));
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigClass::read()
{
    File f = LittleFS.open(CONFIG_FILENAME, "r");

    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    DeserializationError error = deserializeJson(doc, f);

    if (!f || error)
    {
        DBG_PRINTF(DBG_PSTR("Failed to read config file. Loading default configuration.\n"));
    }

    JsonObject cfg = doc["cfg"];

    config.Cfg_Version = cfg["version"] | CONFIG_VERSION;
    config.Cfg_SaveCount = cfg["save_count"] | 0;

    JsonObject wifi = doc["wifi"];
    strlcpy(config.WiFi_Ssid1, wifi["ssid1"] | STA_SSID1, sizeof(config.WiFi_Ssid1));
    strlcpy(config.WiFi_Password1, wifi["password1"] | STA_PASS1, sizeof(config.WiFi_Password1));
    strlcpy(config.WiFi_Ssid2, wifi["ssid2"] | STA_SSID2, sizeof(config.WiFi_Ssid2));
    strlcpy(config.WiFi_Password2, wifi["password2"] | STA_PASS2, sizeof(config.WiFi_Password2));
    strlcpy(config.WiFi_Hostname, wifi["hostname"] | APP_HOSTNAME, sizeof(config.WiFi_Hostname));

    JsonObject ntp = doc["ntp"];
    strlcpy(config.Ntp_Server, ntp["server"] | TIMESERVER_NAME, sizeof(config.Ntp_Server));
    config.Ntp_Port = ntp["port"] | TIMESERVER_PORT;
    strlcpy(config.Ntp_TimezoneDescr, ntp["timezone"] | NTP_TIMEZONEDESCR, sizeof(config.Ntp_TimezoneDescr));

    JsonObject sunset = doc["sunset"];
    config.Sunset_Enabled = sunset["enabled"] | SUNSET_ENABLED;
    strlcpy(config.Sunset_Latitude, sunset["latitude"] | SUNSET_LATITUDE, sizeof(config.Sunset_Latitude));
    strlcpy(config.Sunset_Longitude, sunset["longitude"] | SUNSET_LONGITUDE, sizeof(config.Sunset_Longitude));
    config.Sunset_Sunriseoffset = sunset["sunrise_offset"] | SUNSET_SUNRISEOFFSET;
    config.Sunset_Sunsetoffset = sunset["sunset_offset"] | SUNSET_SUNSETOFFSET;    

    DBG_PRINTF(DBG_PSTR("SSID1: %s.\n"), config.WiFi_Ssid1);
    DBG_PRINTF(DBG_PSTR("SSID2: %s.\n"), config.WiFi_Ssid2);

    DBG_PRINTF(DBG_PSTR("Settings loaded.\n"));

    f.close();

    DBG_PRINTF(DBG_PSTR("write_to_file returns: %u.\n"), write_to_file("/test.txt", "Teststring in file"));

    String text = load_from_file("/test.txt");
    DBG_PRINTF(DBG_PSTR("load_from_file returns: %s.\n"), text.c_str());

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CONFIG_T &ConfigClass::get()
{
    return config;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String ConfigClass::load_from_file(String file_name)
{
    String result = "";

    File this_file = LittleFS.open(file_name, "r");
    if (!this_file)
    { // failed to open the file, retrn empty result
        return result;
    }

    while (this_file.available())
    {
        result += (char)this_file.read();
    }

    this_file.close();
    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ConfigClass::write_to_file(String file_name, String contents)
{
    File this_file = LittleFS.open(file_name, "w");
    if (!this_file)
    { // failed to open the file, return false
        return false;
    }

    int bytesWritten = this_file.print(contents);

    if (bytesWritten == 0)
    { // write failed
        return false;
    }

    this_file.close();
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ConfigClass ConfigInst; // Network class instance singleton

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
