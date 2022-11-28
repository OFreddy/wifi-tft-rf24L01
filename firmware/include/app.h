#pragma once

// Wifi

#include <Ticker.h>

// HTTP Request
#include <AsyncHTTPRequest_Generic.hpp>

// Button
#include <Bounce2.h>

// NTP
#include "suntime.h"
//#include <WiFiUdp.h>
//#include <TimeLib.h>
//#include <sunset.h>

// TFT
#include "MiniGrafx.h"   // General graphic library
#include "ILI9341_SPI.h" // Hardware-specific library

// JSON
#include <ArduinoJson.h>

#include "user_config.h"

#include "web.h"

class web;

class app
{
public:
    app();
    ~app();

    void setup();
    void loop(bool bDisableUpdate = false);

    uint32_t getUnixTimeStamp();

protected:
private:
    // version
    char mVersion[12];

    // Wifi control
    unsigned long reconnectMillis = 0; // will store last time of WiFi reconnection retry
    unsigned long reconnectProgressMillis = 0;
    int wifiConnectProgress;
    wl_status_t oldWifiStatus = WL_IDLE_STATUS; // Wifi status indication
    String escapedMac;
    char hostname[33];
    char serverDescription[33]; // Name of module

    // NTP Service, time and sunset
    time_t mTimestamp;
    suntime st;
    Ticker *mUptimeTicker;
    uint16_t mUptimeInterval;
    uint32_t mUptimeSecs;

    // Http Service
    AsyncHTTPRequest *mHttpReq;

    IPAddress mHttpServerIP;
    uint32_t mHttpReqTicker;
    uint16_t mHttpReqInterval;
    uint32_t lastHttpResponse;

    // Button
    Bounce2::Button *buttonBounce = NULL;

    // Display on/off
    bool displayOn, displayState;
    uint32_t displayDelay;

    // TFT Display
    ILI9341_SPI *tft;
    MiniGrafx *gfx;

// Data
#if defined(ENV_FRI) 
    float gridPower;
    float solarPower;
    float solar1Power;
    float solar2Power;
    float solar3Power;
    float solarTotalEnergy;
    float solarTotalEnergyResolved;
    uint8_t garagePosOpen, garagePosClose;
#endif
#if defined(ENV_DEM)
    int StatusSNS_ENERGY_Power;
    float StatusSNS_ENERGY_Today;
    float StatusSNS_ENERGY_Yesterday;
    float StatusSNS_ENERGY_Total;
    int StatusSNS_ENERGY_Voltage;
#endif

#if defined(ENV_STR)
    float StatusSNS_ENERGY_Total;
#endif

    // General
    void cyclicTick(void);

    // Web sites
    web *mWebInst;

    // Web services
    static void httpRequestCb(void *optParm, AsyncHTTPRequest *request, int readyState);
    void prepareHostname(char *pdest, const char *pname);
    void getHttpData(void);
    void processHttpData(AsyncHTTPRequest *request);

    // Support functions
    int8_t getWifiQuality(void);
    void controlDisplay(void);
    void formatEng(char *dest, float val);

    // Display
    void drawProgress(uint8_t percentage, String text);
    void drawWifiQuality(void);
    void drawTime(void);
    void updateDisplay(void);
    void commitDisplay(void);
};

