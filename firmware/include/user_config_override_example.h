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
 *   (2) define your own settings below
 *
\*****************************************************************************************************/

/* Examples

// -- Setup your own Wifi settings  ---------------
#undef  STA_SSID1
#define STA_SSID1         "YourSSID1"         

#undef  STA_PASS1
#define STA_PASS1         "YourWifiPassword1"     

#undef  STA_SSID2
#define STA_SSID2         "YourSSID2"

#undef  STA_PASS2
#define STA_PASS2         "YourWifiPassword2"    

#undef  TIMESERVER_NAME
#define TIMESERVER_NAME   "pool.ntp.org"

#undef  NTP_TIMEZONEDESCR
#define NTP_TIMEZONEDESCR "Europe/Amsterdam" // Central European time +1

#undef  TIME_GMT_OFFSET_S  
#define TIME_GMT_OFFSET_S 3600               // The GMT offset in seconds for your location

#undef  TIME_DST_OFFSET_S  
#define TIME_DST_OFFSET_S 3600               // The DST offset in seconds dfor your location

#undef  SUNSET_LATITUDE
#define SUNSET_LATITUDE   "53.2197"          // Latitude - Your location to be used with sunrise and sunset

#undef  SUNSET_LONGITUDE
#define SUNSET_LONGITUDE  "7.98004"          // Longitude - Your location to be used with sunrise and sunset

#undef HTTP_REQUEST_INTERVALL 
#define HTTP_REQUEST_INTERVALL 30000         // Http Request intervall for requesting data from HTTP source

#undef HTTP_RESPONSE_TIMEOUT 
#define HTTP_RESPONSE_TIMEOUT  20000         // HTTP Response timeout to indicate missing responses


*/

