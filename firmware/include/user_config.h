/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

/*****************************************************************************************************\
 * USAGE:
 *   To modify the stock configuration according to your personal settings:
 *   (1) copy this file to "user_config_override.h" (It will be ignored by Git)
 *   (2) define your own settings in "user_config_override.h"
 *
\*****************************************************************************************************/

#define SERIAL_BAUDRATE   115200 // UART Debuggging console baudrate

#define CFG_MGCNUMBER     0x2225 // Change this value to load SECTION1 configuration parameters to flash
#define CGF_VERSION       0x0101 // Configuration set version

#define WIFI_AP_ACTIVE_TIME   60 // Period for which ESP will remain in AP mode in case of STA connect error

#define WIFI_SOFT_AP_CHANNEL   1 // Soft Access Point Channel number between 1 and 13 

#define WIFI_SOFT_AP_DNS_PORT 53 // Soft Access Point DNS port number

// Access Point Info
// In case there is no WiFi Network or Ahoy can not connect to it, it will act as an Access Point

#define WIFI_AP_SSID      "WiFiTFT24"
#define WIFI_AP_PWD       "wifitft24"

#define APP_HOSTNAME      "WiFiTFT-%06X"

#define STA_SSID1         "YourSSID1"         
#define STA_PASS1         "YourWifiPassword1"     

#define STA_SSID2         "YourSSID2"         
#define STA_PASS2         "YourWifiPassword2"     

#define TIMESERVER_NAME   "pool.ntp.org"
#define TIMESERVER_PORT   123

#define NTP_TIMEZONEDESCR "Europe/Amsterdam"

#define SUNSET_ENABLED    true      // Enables sunset calculation

#define SUNSET_LATITUDE   "50.941291" // Latitude - Your location to be used with sunrise and sunset
#define SUNSET_LONGITUDE  "6.95828" // Longitude - Your location to be used with sunrise and sunset

#define SUNSET_SUNRISEOFFSET -15    // Offset in minutes to be addded to calculated sunrise time
#define SUNSET_SUNSETOFFSET 15      // Offset in minutes to be addded to calculated sunset time

#define NUMBER_OF_INVERTERS 1       // Amount of Hoymiles inverters to be processed
#define INV1_SERIAL ((uint64_t)0x116181542101ULL)  // 116181542101 = Inverter serial number

#define HTTP_REQUEST_INTERVALL 5000 // Http Request intervall for requesting data from HTTP source

#define HTTP_RESPONSE_TIMEOUT 20000 // HTTP Response timeout to indicate missing responses


#if __has_include("user_config_override.h")
  #include "user_config_override.h"              // Configuration overrides for my_user_config.h
#endif

