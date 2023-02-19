#include <Arduino.h>
#include <umm_malloc/umm_heap_select.h>

#include <AsyncHTTPRequest_Generic.h>

#include "ArialRounded.h"
#include "weathericons.h"

#include "ms_ticker.h"

#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
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

#include "tft_custom.h"
#include "usercontentinst.h"

#include "_dbg.h"
#include "version.h"
#include "app.h"

const String WDAY_NAMES[] = {"SO", "MO", "DI", "MI", "DO", "FR", "SA"};
const String MONTH_NAMES[] = {"JAN", "FEB", "MÄR", "APR", "MAI", "JUN", "JUL", "AUG", "SEP", "OKT", "NOV", "DEZ"};

// Peripherals
#define BUTTON_PIN D3

// LCD Display
#define LCD_PWR_PIN D1
#define TFT_SCK D5
#define TFT_MISO D6
#define TFT_MOSI D7
#define TFT_CS D0 // RF and TFT Board
#define TFT_DC D2


// defines the colors usable in the paletted 16 color frame buffer
uint16_t palette[] = {ILI9341_BLACK, // 0
                      ILI9341_WHITE, // 1
                      ILI9341_GREEN, // 2
                      ILI9341_RED};  // 3

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
    mHttpReqInterval = HTTP_REQUEST_INTERVALL;
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
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Progress bar helper
void app::drawProgress(uint8_t percentage, String text)
{
    gfx->fillBuffer(MINI_BLACK);
    gfx->drawPalettedBitmapFromPgm(80, 5, unknown);
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

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void app::getHttpData(void)
{
    UserContentInst.getHttpData();
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

void app::updateDisplay(void)
{
    if (!displayOn)
        return;

    gfx->fillBuffer(MINI_BLACK);
    drawWifiQuality();
    drawTime();

    // Draw custom display content
    UserContentInst.drawDisplayContent(gfx);

    // Key press for debugging purposes
    gfx->setFont(ArialMT_Plain_10);
    gfx->setColor(MINI_WHITE);
    if (!buttonBounce->read())
        gfx->drawString(0, 9, "Taste");

    // Update display hardware
    commitDisplay();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
