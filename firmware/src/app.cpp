#include <Arduino.h>
#include <umm_malloc/umm_heap_select.h>

#define DEBUG_HTTP_REQ false
#if (DEBUG_HTTP_REQ == true)
#define ASYNC_HTTP_DEBUG_PORT Serial
#define _ASYNC_HTTP_LOGLEVEL_ 2
#endif

#include <AsyncHTTPRequest_Generic.h>

#include "ArialRounded.h"
#include "weathericons.h"

#include "ms_ticker.h"

#if defined(ENV_KER) || defined(ENV_STR)
#include "hm_radio.h"
#include "hm_inverter.h"
extern HM_Radio hmRadio;
#endif

#include "config.h"
#include "network.h"
#include <time.h>
#include "TZinfo.h"
#include "SunsetClass.h"
#include "confignetwork.h"

#include "_dbg.h"
#include "version.h"
#include "app.h"

// HTTP Config
#define HTTPRESPONSE_TIMEOUT 20000

// -- Location ------------------------------------^
// http://api.openweathermap.org/data/2.5/weather?lat=50.8197&lon=7.74004&units=metric&lang=de&APPID=a950613e6fc0423912be0fe02aa897e4

const String WDAY_NAMES[] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MÄR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"};

// Peripherals
#define BUTTON_PIN D3

// LCD Display

#define LCD_PWR_PIN D1
#define TFT_SCK D5
#define TFT_MISO D6
#define TFT_MOSI D7
// #define TFT_CS D8 // Old assignment
#define TFT_CS D0 // RF and TFT Board
#define TFT_DC D2
//#define TFT_RESET D4

#define MINI_BLACK 0
#define MINI_WHITE 1
#define MINI_GREEN 2
#define MINI_RED 3

// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_GREEN, // 2
                      ILI9341_RED};  // 3

#define MINI_2ND MINI_GREEN

int SCREEN_WIDTH = 240;
int SCREEN_HEIGHT = 320;
int BITS_PER_PIXEL = 2; // 2^2 = 4 colors , 2^4 = 16 colors, larger value crashes heap

app::app()
{
    // Version
    snprintf(mVersion, 12, "%d.%02d.%04d", VERSION_MAJOR, VERSION_MINOR, VERSION_BUILD);

    reconnectMillis = reconnectProgressMillis = 0;

    mUptimeInterval = 1;
    mUptimeSecs = 0;

    // Http Request
#if defined(ENV_KER) || defined(ENV_STR)
    mHttpReqInterval = 30000;
#else
    mHttpReqInterval = 5000;
#endif

#if defined(ENV_FRI)
    garagePosOpen = garagePosClose = 0;
#endif
}

app::~app()
{
}

void app::setup()
{
    DBG_PRINTF(DBG_PSTR("Application startup...\n"));

    HeapSelectDram ephemeral;
    DBG_PRINTF(DBG_PSTR("IRAM free: %6d bytes\n"), ESP.getFreeHeap());
    {
        HeapSelectIram ephemeral;
        DBG_PRINTF(DBG_PSTR("DRAM free: %6d bytes\n"), ESP.getFreeHeap());
    }

    // File system and configuration
    DBG_PRINTF(DBG_PSTR("Initialize Filesystem...\n"));
    bool fsinit = false;
#ifdef ARDUINO_ARCH_ESP32
    fsinit = LittleFS.begin(false);
#else
    fsinit = LittleFS.begin();
#endif
    if (!fsinit)
    { // Do not format if mount failed
        DBG_PRINTF(DBG_PSTR("Filesystem failed!"));

#ifdef ARDUINO_ARCH_ESP32
        if (!LittleFS.begin(true))
        {
            DBG_PRINTF(DBG_PSTR("success!");
        }
        else
        {
            Serial.print("failed");
        }
#endif
    }
    else
    {
        Serial.println(F("done"));
    }
    DBG_PRINTF(DBG_PSTR("Loading settings...\n"));
    ConfigInst.read();

    // Configure timezone
    DBG_PRINTF(DBG_PSTR("Configuring timezone %s (%s)...\n"), 
        (char*)&ConfigInst.get().Ntp_TimezoneDescr, getTzInfo(ConfigInst.get().Ntp_TimezoneDescr).c_str());
    configTime(getTzInfo(ConfigInst.get().Ntp_TimezoneDescr).c_str(), ConfigInst.get().Ntp_Server);

    // Network instance
    NetworkInst.setup();

    // Button
    buttonBounce = new Bounce2::Button();
    buttonBounce->attach(BUTTON_PIN, INPUT_PULLUP); // USE INTERNAL PULL-UP
    buttonBounce->interval(20);

#if defined(LCD_PWR_PIN)
    pinMode(LCD_PWR_PIN, OUTPUT);    // sets the pin as output
    digitalWrite(LCD_PWR_PIN, HIGH); // power on
#endif

    DBG_PRINTF(DBG_PSTR("Heap before Web server: %6d bytes\n"), ESP.getFreeHeap());

    // Web server
    mWebInst = new web(this, mVersion);
    mWebInst->setup();

    DBG_PRINTF(DBG_PSTR("Heap before TFT display: %6d bytes\n"), ESP.getFreeHeap());

    // TFT Display
    displayOn = displayState = true;
    tft = new ILI9341_SPI(TFT_CS, TFT_DC);
    gfx = new MiniGrafx(tft, BITS_PER_PIXEL, palette, SCREEN_HEIGHT, SCREEN_WIDTH);
    gfx->init();
    gfx->setRotation(3);
    gfx->fillBuffer(0);

    DBG_PRINTF(DBG_PSTR("Heap before HTTPReq server: %6d bytes\n"), ESP.getFreeHeap());

    // http client
    mHttpReq = new AsyncHTTPRequest();
    mHttpReq->setDebug(DEBUG_HTTP_REQ);
    mHttpReq->onReadyStateChange(httpRequestCb, this);

    DBG_PRINTF(DBG_PSTR("Heap after HTTPReq server: %6d bytes\n"), ESP.getFreeHeap());

    // Sunset / dawn calculation
    SunsetClassInst.init();

    // Cyclic ticker
    mUptimeTicker = new Ticker();
    mUptimeTicker->attach(1, [this]()
                          { this->cyclicTick(); });

    DBG_PRINTF(DBG_PSTR("!!! Setup finished !!!\n"));
    DBG_PRINTF(DBG_PSTR("Hostname = %s\n"), ConfigNetworkInst.GetHostname().c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::loop(bool bDisableUpdate)
{
    // Update button
    buttonBounce->update();

    // Control display
    controlDisplay();

    // Network instance
    NetworkInst.loop();

    // Sunset
    SunsetClassInst.loop();

    // Web server
    mWebInst->loop();

    if (NetworkInst.IsConnected())
    {
        if (checkTicker(&mHttpReqTicker, mHttpReqInterval))
        {
            getHttpData();
        }
    }

    // Display
    static uint32_t displayTick = 0;
    if (checkTicker(&displayTick, 1000))
    {
        if(NetworkInst.IsConnected())
        {
            updateDisplay();
        }
        else
        {
            static int progress = 0;

            if ((progress += 10) >= 100)
                progress = 0;

            if (NetworkInst.GetNetworkState() == eNetworkstateAP)
            {
                drawProgress(progress, "Accesspoint open\nSSID = " + String(WIFI_AP_SSID) + "\nPassword = " + String(WIFI_AP_SSID) + "\nIP = 192.168.1.1");
            }
            else if (NetworkInst.GetNetworkState() == eNetworkstateSTAConnect)
            {
                drawProgress(progress, "Connecting to\n" + NetworkInst.GetStationSSID() + "...");
            }
        }
    }

    mTimestamp = time(nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t app::getUnixTimeStamp()
{
    return mTimestamp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::cyclicTick(void)
{
    mUptimeSecs++;

    if ((mUptimeSecs % 60) == 0)
    {
    }

    // updateDisplay();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Progress bar helper
void app::drawProgress(uint8_t percentage, String text)
{
    gfx->fillBuffer(MINI_BLACK);
    // gfx->drawPalettedBitmapFromPgm(20, 5, ThingPulseLogo);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setColor(MINI_WHITE);
    gfx->drawString(120, 90, "wifi-tft " + String(mVersion));
    gfx->setColor(MINI_2ND);

    gfx->drawStringMaxWidth(120, 146, SCREEN_WIDTH, text);
    gfx->setColor(MINI_WHITE);
    gfx->drawRect(10, 198, SCREEN_WIDTH - 20, 15);
    gfx->setColor(MINI_2ND);
    gfx->fillRect(12, 200, (SCREEN_WIDTH - 24) * percentage / 100, 11);

    gfx->commit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::controlDisplay(void)
{
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    int minPastMidnght = timeinfo->tm_hour * 60 + timeinfo->tm_min;

    if (!buttonBounce->read())
        displayDelay = millis() + 30000;

    int sunrise = SunsetClassInst.getSunriseMinutes();
    int sunset = SunsetClassInst.getSunsetMinutes();
    displayOn = (sunrise <= 0) || (sunset <= 0) || ((minPastMidnght >= sunrise) && (minPastMidnght <= sunset)) || (displayDelay > millis());

    if (displayOn && !displayState)
    {                                    // Turn on
        digitalWrite(LCD_PWR_PIN, HIGH); // power on

        tft->writecommand(ILI9341_DISPON);

        displayState = true;
    }
    else if (!displayOn && displayState)
    {                                   // Turn off
        digitalWrite(LCD_PWR_PIN, LOW); // power off

        tft->writecommand(ILI9341_DISPOFF);

        displayState = false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::httpRequestCb(void *optParm, AsyncHTTPRequest *request, int readyState)
{
    app *inst = (app *)optParm;

    if (readyState == readyStateDone)
    {
        String httpResponse = request->responseText();

        DBG_PRINTF(DBG_PSTR("\n%08i **ReadyStateDone %i\n"), millis(), request->responseHTTPcode());

        if (request->responseHTTPcode() == -4 || request->responseHTTPcode() == -5)
        {
            if (NetworkInst.IsConnected())
            {
                // retriggerTicker(&inst->mHttpReqTicker, 1000);
            }
        }

        if (request->responseHTTPcode() != 200)
            return;

        if (httpResponse.length() == 0)
        {
            DBG_PRINTF(DBG_PSTR("\n%08i Http received empty response\n"), millis());
            // inst->getHttpData();
            return;
        }

        retriggerTicker(&inst->mHttpReqTicker, inst->mHttpReqInterval);

        DBG_PRINTF(DBG_PSTR("\n%08i **************************************\n"), millis());
        DBG_PRINTF(httpResponse.c_str());
        DBG_PRINTF(DBG_PSTR("\n**************************************\n"));

#if defined(ENV_FRI)
        // see https://arduinojson.org/v6/assistant/#/step1
        StaticJsonDocument<32> filter;
        filter[0]["val"] = true;

        StaticJsonDocument<512> jsonDoc;

        auto error = deserializeJson(jsonDoc, httpResponse.c_str(), DeserializationOption::Filter(filter));

        if (error)
        {
            DBG_PRINTF(DBG_PSTR("JSON parsing failed! Code: %s\n"), error.c_str());
            return;
        }

        JsonArray root = jsonDoc.as<JsonArray>();

        inst->gridPower = root[0]["val"];
        inst->solarPower = root[1]["val"];
        inst->solar1Power = root[2]["val"];
        inst->solar2Power = root[3]["val"];
        inst->solar3Power = root[4]["val"];
        inst->solarTotalEnergy = root[5]["val"];
        inst->solarTotalEnergyResolved = root[6]["val"];
        inst->garagePosClose = root[7]["val"];
        inst->garagePosOpen = root[8]["val"];
#endif

#if defined(ENV_DEM)
        // see https://arduinojson.org/v6/assistant/#/step1
        StaticJsonDocument<512> jsonDoc;
        auto error = deserializeJson(jsonDoc, httpResponse.c_str());

        if (error)
        {
            DBG_PRINTF(DBG_PSTR("JSON parsing failed! Code: %s\n"), error.c_str());
            return;
        }

        // const char *StatusSNS_Time = jsonDoc["StatusSNS"]["Time"]; // "2022-06-04T10:31:45"

        JsonObject StatusSNS_ENERGY = jsonDoc["StatusSNS"]["ENERGY"];
        // const char *StatusSNS_ENERGY_TotalStartTime = StatusSNS_ENERGY["TotalStartTime"];
        inst->StatusSNS_ENERGY_Total = StatusSNS_ENERGY["Total"];         // 0.687
        inst->StatusSNS_ENERGY_Yesterday = StatusSNS_ENERGY["Yesterday"]; // 0.687
        inst->StatusSNS_ENERGY_Today = StatusSNS_ENERGY["Today"];         // 0
        inst->StatusSNS_ENERGY_Power = StatusSNS_ENERGY["Power"];         // 0
        // int StatusSNS_ENERGY_ApparentPower = StatusSNS_ENERGY["ApparentPower"]; // 0
        // int StatusSNS_ENERGY_ReactivePower = StatusSNS_ENERGY["ReactivePower"]; // 0
        // int StatusSNS_ENERGY_Factor = StatusSNS_ENERGY["Factor"];               // 0
        inst->StatusSNS_ENERGY_Voltage = StatusSNS_ENERGY["Voltage"]; // 0
                                                                      // int StatusSNS_ENERGY_Current = StatusSNS_ENERGY["Current"];             // 0
#endif

#if defined(ENV_STR)
        // see https://arduinojson.org/v6/assistant/#/step1
        StaticJsonDocument<512> jsonDoc;
        auto error = deserializeJson(jsonDoc, httpResponse.c_str());

        if (error)
        {
            DBG_PRINTF(DBG_PSTR("JSON parsing failed! Code: %s\n"), error.c_str());
            return;
        }

        // const char *StatusSNS_Time = jsonDoc["StatusSNS"]["Time"]; // "2022-06-04T10:31:45"

        JsonObject StatusSNS_ENERGY = jsonDoc["StatusSNS"]["STROM"];
        // const char *StatusSNS_ENERGY_TotalStartTime = StatusSNS_ENERGY["TotalStartTime"];
        inst->StatusSNS_ENERGY_Total = StatusSNS_ENERGY["Total_in"]; // 0.687
#endif

        inst->lastHttpResponse = millis();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::getHttpData(void)
{
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Beispiele für URLs:
    //   Über Internet auf offenen Port der Restful API Daten senden:

#if defined(ENV_KER)
    // Do nothing here
    return;
    // "https://of22.acedns.org:46879/prettyPrint&user=restapi&pass=gH$4pQV7&setBulk?javascript.0.solarnb_power=777.7"
    // String serverPath = "https://of22.acedns.org:46879/prettyPrint&user=restapi&pass=gH$4pQV7&setBulk?";
    String serverPath = "http://of22.acedns.org:46879/setBulk?prettyPrint&user=restapi&pass=gH$4pQV7&";
    serverPath += "javascript.0.solarnb_power=";
    HM_Inverter<float> *inv = hmPackets.GetInverterInstance(0);
    float f = inv->getValue(inv->getPosByChFld(CH0, FLD_PAC));
    serverPath += String(f);
    serverPath += "&javascript.0.solarnb1_power=";
    f = inv->getValue(inv->getPosByChFld(CH1, FLD_PDC));
    serverPath += String(f);
    serverPath += "&javascript.0.solarnb2_power=";
    f = inv->getValue(inv->getPosByChFld(CH2, FLD_PDC));
    serverPath += String(f);
    serverPath += "&javascript.0.solarnb3_power=";
    f = inv->getValue(inv->getPosByChFld(CH3, FLD_PDC));
    serverPath += String(f);
    serverPath += "&javascript.0.solarnb4_power=";
    f = inv->getValue(inv->getPosByChFld(CH4, FLD_PDC));
    serverPath += String(f);
#endif
#if defined(ENV_FRI)
    // https://of22.acedns.org:46879/prettyPrint&user=restapi&pass=gH$4pQV7&setBulk?javascript.0.solarnb_power=777.7
    const String serverPath = "http://192.168.178.3:8087/getBulk/smartmeter.0.1-0:16_7_0__255.value,javascript.0.solartotal_power,javascript.0.solar1_power,javascript.0.solar2_power,javascript.0.solar3_power,javascript.0.solartotal_energycalc,javascript.0.solartotal_energyresolved,mqtt.0.stat.Garage.POSCLOSE,mqtt.0.stat.Garage.POSOPEN";
    // const String serverPath = F("http://api.openweathermap.org/data/2.5/weather?lat=50.8197&lon=7.74004&units=metric&lang=de&APPID=a950613e6fc0423912be0fe02aa897e4");
#endif
#if defined(ENV_DEM)
    const String serverPath = "http://solar2.fritz.box/cm?cmnd=status%208";
#endif
#if defined(ENV_STR)
    const String serverPath = "http://strom.fritz.box/cm?cmnd=status%208";
#endif

    if (mHttpReq->readyState() == readyStateUnsent || mHttpReq->readyState() == readyStateDone)
    {
        bool requestOpenResult = mHttpReq->open("GET", serverPath.c_str());
        if (requestOpenResult)
        {
            // Only send() if open() returns true, or crash
            DBG_PRINTF(DBG_PSTR("%08i Sending Http request...\n"), millis());
            mHttpReq->send();
        }
        else
        {
            DBG_PRINTF(DBG_PSTR("Can't send bad request\n"));
            DBG_PRINTF(serverPath.c_str());
        }
    }
    else
    {
        (DBG_PSTR("Can't send request\n"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::processHttpData(AsyncHTTPRequest *request)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::drawWifiQuality(void)
{
    int8_t quality = NetworkInst.GetWifiQuality();

    gfx->setColor(MINI_WHITE);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->drawString(228, 9, String(quality) + "%");
    for (int8_t i = 0; i < 4; i++)
    {
        for (int8_t j = 0; j < 2 * (i + 1); j++)
        {
            if (quality > i * 25 || j == 0)
            {
                gfx->setPixel(230 + 2 * i, 18 - j);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::drawTime(void)
{
    char time_str[11];
    struct tm *timeinfo = localtime(&mTimestamp);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setColor(MINI_WHITE);
    String date = WDAY_NAMES[timeinfo->tm_wday] + " " + String(timeinfo->tm_mday) + " " + MONTH_NAMES[timeinfo->tm_mon] + " " + String(1900 + timeinfo->tm_year);
    gfx->drawString(120, 6, date);

    gfx->setFont(ArialRoundedMTBold_36);

    sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); // hh:mm:ss

    gfx->drawString(120, 20, time_str);

    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setColor(MINI_2ND);

    int sunrise = SunsetClassInst.getSunriseMinutes();
    int sunset = SunsetClassInst.getSunsetMinutes();

    sprintf(time_str, "%02u:%02u", sunrise / 60, sunrise % 60);
    gfx->drawString(240, 34, time_str);
    gfx->drawTriangle(205, 42, 211, 42, 208, 39);
    sprintf(time_str, "%02u:%02u", sunset / 60, sunset % 60);
    gfx->drawString(240, 44, time_str);
    gfx->drawTriangle(205, 49, 211, 49, 208, 52);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::formatEng(char *dest, float val)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::commitDisplay(void)
{
    noInterrupts();
    gfx->commit();
    interrupts();
    yield();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ENV_FRI)
void app::updateDisplay(void)
{
    char buf[32];

    if (!displayOn)
        return;

    gfx->fillBuffer(MINI_BLACK);
    drawWifiQuality();
    drawTime();

    // Horizontal lines
    // gfx->drawLine(0,  80, 240,  80);
    gfx->drawLine(0, 140, 240, 140);
    gfx->drawLine(0, 200, 240, 200);
    gfx->drawLine(0, 260, 240, 260);

    // Vertical lines
    gfx->drawLine(120, 140, 120, 320);

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setColor(MINI_2ND);
    gfx->drawString(60, 145, "Strom Summe [W]");
    gfx->drawString(180, 145, "Solar [W]");
    gfx->drawString(60, 205, "Netz [W]");
    gfx->drawString(180, 205, "Solar Summe [W]");
    gfx->setColor(MINI_WHITE);

    gfx->drawString(60, 265, "Solar Total");
    gfx->drawString(180, 265, "Solar Selbst");

    gfx->setColor(MINI_2ND);

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);

    gfx->drawString(126, 165, PSTR("Trecker:"));
    gfx->drawString(126, 180, PSTR("Garten:"));

    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);

    sprintf_P(&buf[0], PSTR("%.1f"), solar3Power);
    gfx->drawString(236, 165, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), solar1Power + solar2Power);
    gfx->drawString(236, 180, buf);

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    sprintf_P(&buf[0], PSTR("%.0f"), gridPower + solarPower);
    gfx->drawString(60, 160, buf);

    if (gridPower < 0.0)
        gfx->setColor(MINI_RED);
    sprintf_P(&buf[0], PSTR("%.0f"), gridPower);
    gfx->drawString(60, 220, buf);
    gfx->setColor(MINI_2ND);

    if (solarPower > 1000)
        sprintf_P(&buf[0], PSTR("%.0f"), solarPower);
    else
        sprintf_P(&buf[0], PSTR("%.1f"), solarPower);
    gfx->drawString(180, 220, buf);

    gfx->setColor(MINI_WHITE);
    sprintf_P(&buf[0], PSTR("%.1f"), solarTotalEnergy);
    gfx->drawString(60, 280, buf);

    sprintf_P(&buf[0], PSTR("%.1f"), solarTotalEnergyResolved);
    gfx->drawString(180, 280, buf);

    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    if (lastHttpResponse + HTTPRESPONSE_TIMEOUT < millis())
    {
        gfx->setColor(MINI_RED);
        gfx->drawString(120, 80, "Keine Daten!");
        gridPower = solarPower = solarTotalEnergy = solarTotalEnergyResolved = 0;
    }
    else
    {
        gfx->setFont(ArialMT_Plain_16);
        if ((garagePosOpen == 0) && (garagePosClose == 0))
        {
            gfx->setColor(MINI_RED);
            gfx->drawString(120, 80, "Garage Mittelstellung");
        }
        else if ((garagePosOpen == 1) && (garagePosClose == 1))
        {
            gfx->setColor(MINI_RED);
            gfx->drawString(120, 80, "Garage Fehler");
        }
        else if (garagePosOpen == 1)
        {
            gfx->setColor(MINI_WHITE);
            gfx->drawString(120, 80, "Garage AUF");
        }
        else if (garagePosClose == 1)
        {
            gfx->setColor(MINI_2ND);
            gfx->drawString(120, 80, "Garage ZU");
        }
    }

    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setColor(MINI_WHITE);
    if (!buttonBounce->read())
        gfx->drawString(0, 9, "Taste");

    commitDisplay();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ENV_KER)
void app::updateDisplay(void)
{
    char buf[32];
    float f;
    HM_Inverter<float> *inv = hmPackets.GetInverterInstance(0);

    if (!displayOn)
        return;

    gfx->fillBuffer(MINI_BLACK);
    drawWifiQuality();
    drawTime();

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->setColor(MINI_2ND);
    if ((mTimestamp - inv->lastPayloadRcvTime) > 36000)
        sprintf_P(&buf[0], PSTR("Daten nicht aktuell!"));
    else if (mTimestamp >= inv->lastPayloadRcvTime)
        sprintf_P(&buf[0], PSTR("Daten Alter: %u"), mTimestamp - inv->lastPayloadRcvTime);
    else
        sprintf_P(&buf[0], PSTR("Daten Alter: 0"));
    gfx->drawString(235, 125, buf);

    // Horizontal lines
    // gfx->drawLine(0,  80, 240,  80);
    gfx->drawLine(0, 140, 240, 140);
    gfx->drawLine(0, 200, 240, 200);
    gfx->drawLine(0, 260, 240, 260);

    // Vertical lines
    gfx->drawLine(120, 140, 120, 320);

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setColor(MINI_2ND);
    gfx->drawString(60, 145, "Spannung [V]");
    gfx->drawString(180, 145, "Temperatur [°C]");

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH0, FLD_UAC)));
    gfx->drawString(60, 160, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH0, FLD_T)));
    gfx->drawString(180, 160, buf);

    // Mittlere Reihe
    gfx->setColor(MINI_GREEN);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(60, 205, "Solar [W]");
    gfx->drawString(180, 205, "Solar Summe [W]");
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH1, FLD_PDC)));
    gfx->drawString(55, 225, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH2, FLD_PDC)));
    gfx->drawString(115, 225, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH3, FLD_PDC)));
    gfx->drawString(55, 240, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH4, FLD_PDC)));
    gfx->drawString(115, 240, buf);

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    f = inv->getValue(inv->getPosByChFld(CH0, FLD_PAC));
    if (f > 1000)
        sprintf_P(&buf[0], PSTR("%.0f"), f);
    else
        sprintf_P(&buf[0], PSTR("%.1f"), f);
    gfx->drawString(180, 220, buf);

    // Unterste Reihe Ertrag Tag/Total
    gfx->setColor(MINI_WHITE);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(60, 265, "Ertrag Tag [Wh]");
    gfx->drawString(180, 265, "Ertrag [kWh]");
    gfx->setFont(ArialRoundedMTBold_36);
    sprintf_P(&buf[0], PSTR("%.0f"), inv->getValue(inv->getPosByChFld(CH0, FLD_YD)));
    gfx->drawString(60, 280, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv->getValue(inv->getPosByChFld(CH0, FLD_YT)));
    gfx->drawString(180, 280, buf);

    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setColor(MINI_WHITE);
    if (!buttonBounce->read())
        gfx->drawString(0, 9, "Taste");

    commitDisplay();
}
#endif

#if defined(ENV_DEM)
void app::updateDisplay(void)
{
    char buf[20];

    if (!displayOn)
        return;

    gfx->fillBuffer(MINI_BLACK);
    drawWifiQuality();
    drawTime();

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setColor(MINI_2ND);
    gfx->drawString(60, 145, "Leistung");
    gfx->drawString(180, 145, "Spannung");
    gfx->setColor(MINI_WHITE);
    gfx->drawLine(0, 140, 240, 140);
    gfx->drawLine(120, 140, 120, 199);

    gfx->drawString(60, 205, "Energie Heute");
    gfx->drawString(180, 205, "Energie Gestern");
    gfx->drawString(120, 265, "Energie Total");
    gfx->drawLine(0, 200, 240, 200);
    gfx->drawLine(120, 200, 120, 260);
    gfx->drawLine(0, 260, 240, 260);

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setColor(MINI_2ND);
    sprintf_P(&buf[0], PSTR(" %i W "), StatusSNS_ENERGY_Power);
    gfx->drawString(60, 160, buf);
    sprintf_P(&buf[0], PSTR(" %i V "), StatusSNS_ENERGY_Voltage);
    gfx->drawString(180, 160, buf);

    gfx->setColor(MINI_WHITE);
    sprintf_P(&buf[0], PSTR(" %.1f "), StatusSNS_ENERGY_Today);
    gfx->drawString(60, 220, buf);

    sprintf_P(&buf[0], PSTR(" %.1f "), StatusSNS_ENERGY_Yesterday);
    gfx->drawString(180, 220, buf);

    sprintf_P(&buf[0], PSTR(" %.1f kWh "), StatusSNS_ENERGY_Total);
    gfx->drawString(120, 280, buf);

    if (lastHttpResponse + HTTPRESPONSE_TIMEOUT < millis())
    {
        gfx->setColor(MINI_RED);
        gfx->drawString(120, 80, "Keine Daten!");
        StatusSNS_ENERGY_Power = StatusSNS_ENERGY_Voltage = 0;
        StatusSNS_ENERGY_Today = StatusSNS_ENERGY_Yesterday = StatusSNS_ENERGY_Total = 0;
    }

    gfx->setColor(MINI_WHITE);
    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->setFont(ArialMT_Plain_10);
    if (!buttonBounce->read())
        gfx->drawString(0, 9, "Taste");

    commitDisplay();
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(ENV_STR)
void app::updateDisplay(void)
{
    char buf[32];
    float f;
    HM_Inverter<float> *inv1 = hmRadio.GetInverterInstance(0);
    HM_Inverter<float> *inv2 = hmRadio.GetInverterInstance(1);

    if (!displayOn)
        return;

    gfx->fillBuffer(MINI_BLACK);
    drawWifiQuality();
    drawTime();

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    gfx->setColor(MINI_2ND);
    if ((mTimestamp - inv1->lastPayloadRcvTime) > 36000)
        sprintf_P(&buf[0], PSTR("Daten nicht aktuell!"));
    else if (mTimestamp >= inv1->lastPayloadRcvTime)
        sprintf_P(&buf[0], PSTR("Daten Alter: %u"), mTimestamp - inv1->lastPayloadRcvTime);
    else
        sprintf_P(&buf[0], PSTR("Daten Alter: 0"));
    gfx->drawString(235, 125, buf);

    // Horizontal lines
    // gfx->drawLine(0,  80, 240,  80);
    gfx->drawLine(0, 140, 240, 140);
    gfx->drawLine(0, 200, 240, 200);
    gfx->drawLine(0, 260, 240, 260);

    // Vertical lines
    gfx->drawLine(120, 140, 120, 320);

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setColor(MINI_2ND);
    gfx->drawString(60, 145, "Spannung [V]");
    gfx->drawString(180, 145, "Temperatur [°C]");

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    sprintf_P(&buf[0], PSTR("%.1f kWh"), StatusSNS_ENERGY_Total);
    gfx->drawString(120, 90, buf);

    sprintf_P(&buf[0], PSTR("%.1f"), inv1->getValue(inv1->getPosByChFld(CH0, FLD_UAC)));
    gfx->drawString(60, 160, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv1->getValue(inv1->getPosByChFld(CH0, FLD_T)));
    gfx->drawString(180, 160, buf);

    // Mittlere Reihe
    gfx->setColor(MINI_GREEN);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(60, 205, "Solar [W]");
    gfx->drawString(180, 205, "Solar Summe [W]");
    gfx->setTextAlignment(TEXT_ALIGN_RIGHT);
    sprintf_P(&buf[0], PSTR("%.1f"), inv1->getValue(inv1->getPosByChFld(CH1, FLD_PDC)));
    gfx->drawString(55, 225, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv1->getValue(inv1->getPosByChFld(CH2, FLD_PDC)));
    gfx->drawString(115, 225, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv2->getValue(inv2->getPosByChFld(CH1, FLD_PDC)));
    gfx->drawString(55, 240, buf);
    sprintf_P(&buf[0], PSTR("%.1f"), inv2->getValue(inv2->getPosByChFld(CH2, FLD_PDC)));
    gfx->drawString(115, 240, buf);

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    f = inv1->getValue(inv1->getPosByChFld(CH0, FLD_PAC)) + inv2->getValue(inv2->getPosByChFld(CH0, FLD_PAC));
    if (f > 1000)
        sprintf_P(&buf[0], PSTR("%.0f"), f);
    else
        sprintf_P(&buf[0], PSTR("%.1f"), f);
    gfx->drawString(180, 220, buf);

    // Unterste Reihe Ertrag Tag/Total
    gfx->setColor(MINI_WHITE);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(60, 265, "Ertrag Tag [Wh]");
    gfx->drawString(180, 265, "Ertrag [kWh]");
    gfx->setFont(ArialRoundedMTBold_36);
    f = inv1->getValue(inv1->getPosByChFld(CH0, FLD_YD)) + inv2->getValue(inv2->getPosByChFld(CH0, FLD_YD));

    sprintf_P(&buf[0], PSTR("%.0f"), f);
    gfx->drawString(60, 280, buf);
    f = inv1->getValue(inv1->getPosByChFld(CH0, FLD_YT)) + inv2->getValue(inv2->getPosByChFld(CH0, FLD_YT));
    sprintf_P(&buf[0], PSTR("%.1f"), f);
    gfx->drawString(180, 280, buf);

    gfx->setTextAlignment(TEXT_ALIGN_LEFT);
    gfx->setFont(ArialMT_Plain_10);
    gfx->setColor(MINI_WHITE);
    if (!buttonBounce->read())
        gfx->drawString(0, 9, "Taste");

    commitDisplay();
}
#endif
