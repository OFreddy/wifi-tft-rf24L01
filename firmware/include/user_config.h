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

#define CFG_MGCNUMBER     0x2225 // Change this value to load SECTION1 configuration parameters to flash
#define CGF_VERSION       0x0101 // Conbfiguration set version

#define STA_SSID1         "YourSSID1"         
#define STA_PASS1         "YourWifiPassword1"     

#define STA_SSID2         "YourSSID2"         
#define STA_PASS2         "YourWifiPassword2"     

#define TIMESERVER_NAME   "pool.ntp.org"

#define TIMEZONE          1        // Central European time +1

#define TIME_GMT_OFFSET_S 3600     // The GMT offset in seconds for your location

#define TIME_DST_OFFSET_S 3600     // The DST offset in seconds dfor your location

#define LATITUDE          53.2197  // [Latitude] Your location to be used with sunrise and sunset
#define LONGITUDE         7.98004  // [Longitude] Your location to be used with sunrise and sunset

#if __has_include("user_config_override.h")
  #include "user_config_override.h"              // Configuration overrides for my_user_config.h
#endif
