#if defined(ESP32) && defined(F)
#undef F
#define F(sl) (sl)
#endif

#include "web.h"

// Web page contents
#include "style_css.h"
#include "favicon.h"
#include "index_html.h"
#include "config_html.h"

#include "config_x.h"
#include "config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

web::web(app *main, char version[])
{
    mMain = main;
    mVersion = version;
#ifdef ESP8266
    mWebServer = new ESP8266WebServer(80);
    mUpdater = new ESP8266HTTPUpdateServer();
#elif defined(ESP32)
    mWeb = new WebServer(80);
    mUpdater = new HTTPUpdateServer();
#endif
    mUpdater->setup(mWebServer);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::setup(void)
{
    // Wifi configuration
    mConfigWifi = new WebConfig();
    DBG_PRINTF(DBG_PSTR("web::setup wifiCfg->setDescription\n"));
    mConfigWifi->setCfgName(PSTR("WiFi Settings"));
    mConfigWifi->setDescription(paramsWifi);
    DBG_PRINTF(DBG_PSTR("web::setup wifiCfg->readConfig\n"));
    mConfigWifi->readConfig(CONFFILENAMEWIFI);
    DBG_PRINTF(DBG_PSTR("web::setup finished\n"));

    mWebServer->on("/", std::bind(&web::showIndex, this));
    mWebServer->on("/index", std::bind(&web::showIndex, this));
    mWebServer->on("/style.css", std::bind(&web::showCss, this));
    mWebServer->on("/favicon.ico", std::bind(&web::showFavicon, this));
    mWebServer->onNotFound(std::bind(&web::showNotFound, this));
    mWebServer->on("/cfg", std::bind(&web::showConfig, this));
    mWebServer->on("/cfgwifi", std::bind(&web::showWifiCfg, this));
    mWebServer->on("/uptime", std::bind(&web::showUptime, this));
    mWebServer->on("/time", std::bind(&web::showTime, this));
    mWebServer->on("/cmdstat", std::bind(&web::showStatistics, this));

    mWebServer->begin();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::loop(void)
{
    mWebServer->handleClient();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showIndex(void)
{
    String html = FPSTR(index_html);
    html.replace(F("{VERSION}"), mVersion);
    html.replace(F("{GIT}"), AUTO_GIT_HASH);
    mWebServer->send(200, "text/html", html);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showCss(void)
{
    mWebServer->send(200, "text/css", FPSTR(style_css));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showFavicon(void)
{
    static const char favicon_type[] PROGMEM = "image/x-icon";
    static const char favicon_content[] PROGMEM = FAVICON_PANEL_16;
    mWebServer->send_P(200, favicon_type, favicon_content, sizeof(favicon_content));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showNotFound(void)
{
    DBG_PRINTF(DBG_PSTR("web::showNotFound - %s"), mWebServer->uri());
    String msg = F("File Not Found\n\nURI: ");
    msg += mWebServer->uri();
    msg += F("\nMethod: ");
    msg += (mWebServer->method() == HTTP_GET) ? "GET" : "POST";
    msg += F("\nArguments: ");
    msg += mWebServer->args();
    msg += "\n";

    for (uint8_t i = 0; i < mWebServer->args(); i++)
    {
        msg += " " + mWebServer->argName(i) + ": " + mWebServer->arg(i) + "\n";
    }

    mWebServer->send(404, F("text/plain"), msg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showConfig(void)
{
    String html = FPSTR(config_html);
    html.replace(F("{VERSION}"), mVersion);
    html.replace(F("{GIT}"), AUTO_GIT_HASH);
    mWebServer->send(200, "text/html", html);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showWifiCfg(void)
{
    DBG_PRINTF(DBG_PSTR("web::showWifiCfg"));
    dumpReceivedArgs();
    mConfigWifi->handleFormRequest(mWebServer);
    if (mWebServer->hasArg(F("save")))
    {
        uint8_t cnt = mConfigWifi->getCount();
        DBG_PRINTF(DBG_PSTR("*********** Wifi Konfiguration ************\n"));
        for (uint8_t i = 0; i < cnt; i++)
        {
            Serial.print(mConfigWifi->getName(i));
            Serial.print(" = ");
            Serial.println(mConfigWifi->values[i]);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showUptime(void)
{
    char time[20] = {0};

    uint32_t mUptimeSecs = 4711;

    int upTimeSc = uint32_t((mUptimeSecs) % 60);
    int upTimeMn = uint32_t((mUptimeSecs / (60)) % 60);
    int upTimeHr = uint32_t((mUptimeSecs / (60 * 60)) % 24);
    int upTimeDy = uint32_t((mUptimeSecs / (60 * 60 * 24)) % 365);

    snprintf(time, 20, "%d Tage, %02d:%02d:%02d", upTimeDy, upTimeHr, upTimeMn, upTimeSc);

    mWebServer->send(200, "text/plain", String(time));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showTime(void)
{
    char time_str[11];
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    // String datetime = WDAY_NAMES[timeinfo->tm_wday] + " " + String(timeinfo->tm_mday) + " " + MONTH_NAMES[timeinfo->tm_mon] + " " + String(1900 + timeinfo->tm_year);
    String datetime = String("olliDay");

    sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); // hh:mm:ss

    datetime += String(", ") + String(time_str);

    mWebServer->send(200, "text/plain", datetime.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showStatistics(void)
{
    mWebServer->send(200, F("text/plain"), String("Statistics"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::dumpReceivedArgs(void)
{
    String args = "ArgCnt: ";
    args += mWebServer->args();

    for (int i = 0; i < mWebServer->args(); i++)
    {
        args += "Arg " + (String)i + " -> ";   // Include the current iteration value
        args += mWebServer->argName(i) + ": "; // Get the name of the parameter
        args += mWebServer->arg(i) + "\n";     // Get the value of the parameter
    }

    DBG_PRINTF(DBG_PSTR("%s"), args.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
