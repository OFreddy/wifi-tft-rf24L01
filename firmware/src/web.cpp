#if defined(ESP32) && defined(F)
#undef F
#define F(sl) (sl)
#endif

#include "web.h"

#include "i18n.h"

// Web page contents
#include "style_css.h"
#include "favicon.h"
#include "index_html.h"
#include "config_html.h"
#include "config_wifi_html.h"
#include "config_ntp_html.h"
#include "restart_html.h"

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

    mWebServer->on("/", std::bind(&web::showIndex, this));
    mWebServer->on("/index", std::bind(&web::showIndex, this));
    mWebServer->on("/style.css", std::bind(&web::showCss, this));
    mWebServer->on("/favicon.ico", std::bind(&web::showFavicon, this));
    mWebServer->onNotFound(std::bind(&web::showNotFound, this));
    mWebServer->on("/cfg", std::bind(&web::showConfig, this));
    mWebServer->on("/cfgwifi", std::bind(&web::showWifiCfg, this));
    mWebServer->on("/cfgntp", std::bind(&web::showNtpCfg, this));
    mWebServer->on("/uptime", std::bind(&web::showUptime, this));
    mWebServer->on("/time", std::bind(&web::showTime, this));
    mWebServer->on("/cmdstat", std::bind(&web::showStatistics, this));
    mWebServer->on("/file", std::bind(&web::showFile, this));

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
    if (mWebServer->hasArg(F("rst")))
    {
        // Send response
        String html = FPSTR(restart_html);
        html = fillStandardParms(html);
        html.replace(F("{MSG}"), F("Restart requested!"));
        mWebServer->send(200, "text/html", html);

        ConfigInst.requestRestart();
        return;
    }

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
    html = fillStandardParms(html);
    mWebServer->send(200, "text/html", html);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showWifiCfg(void)
{
    DBG_PRINTF(DBG_PSTR("web::showWifiCfg\n"));
    dumpReceivedArgs();
    if (mWebServer->hasArg(F("save")))
    {
        char tmp[64];

        uint8_t cnt = mWebServer->args();
        DBG_PRINTF(DBG_PSTR("*********** Wifi Konfiguration ************\n"));
        for (uint8_t i = 0; i < cnt; i++)
        {
            Serial.print(mWebServer->argName(i).c_str());
            Serial.print(" = ");
            Serial.println(mWebServer->arg(i).c_str());
        }

        if (getArg(PSTR("ssid1"), tmp, sizeof(tmp)))
            strncpy((char *)&ConfigInst.get().WiFi_Ssid1, tmp, sizeof(ConfigInst.get().WiFi_Ssid1));
        if (getArg(PSTR("pwd1"), tmp, sizeof(tmp)))
            if(strcmp((char*)&tmp, D_ASTERISK_PWD) != 0)
                strncpy((char *)&ConfigInst.get().WiFi_Password1, tmp, sizeof(ConfigInst.get().WiFi_Password1));
        if (getArg(PSTR("ssid2"), tmp, sizeof(tmp)))
            if(strcmp((char*)&tmp, D_ASTERISK_PWD) != 0)
                strncpy((char *)&ConfigInst.get().WiFi_Ssid2, tmp, sizeof(ConfigInst.get().WiFi_Ssid2));
        if (getArg(PSTR("pwd2"), tmp, sizeof(tmp)))
            strncpy((char *)&ConfigInst.get().WiFi_Password2, tmp, sizeof(ConfigInst.get().WiFi_Password2));
        if (getArg(PSTR("nostname"), tmp, sizeof(tmp)))
            strncpy((char *)&ConfigInst.get().WiFi_Hostname, tmp, sizeof(ConfigInst.get().WiFi_Hostname));

        ConfigInst.write();

        // Send response
        String html = FPSTR(restart_html);
        html = fillStandardParms(html);
        html.replace(F("{MSG}"), F("Configuration saved!"));
        mWebServer->send(200, "text/html", html);

        ConfigInst.requestRestart();

        return;
    }

    String html = FPSTR(config_wifi_html);

    html = fillStandardParms(html);

    html.replace(F("{SSID1}"), ConfigInst.get().WiFi_Ssid1);
    html.replace(F("{PWD1}"), F("****"));
    html.replace(F("{SSID2}"), ConfigInst.get().WiFi_Ssid2);
    html.replace(F("{PWD2}"), F("****"));
    html.replace(F("{HOSTNAME}"), ConfigInst.get().WiFi_Hostname);

    DBG_PRINTF(DBG_PSTR("web::showWifiCfg Sending response...\n"));
    mWebServer->send(200, "text/html", html);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showNtpCfg(void)
{
    char tmp[64];

    DBG_PRINTF(DBG_PSTR("web::showNtpCfg\n"));
    dumpReceivedArgs();
    if (mWebServer->hasArg(F("save")))
    {

        uint8_t cnt = mWebServer->args();
        DBG_PRINTF(DBG_PSTR("*********** NTP Konfiguration ************\n"));
        for (uint8_t i = 0; i < cnt; i++)
        {
            Serial.print(mWebServer->argName(i).c_str());
            Serial.print(" = ");
            Serial.println(mWebServer->arg(i).c_str());
        }

        if (getArg(PSTR("ssen"), tmp, sizeof(tmp)))
            ConfigInst.get().Sunset_Enabled = true;
        if (getArg(PSTR("lat"), tmp, sizeof(tmp)))
            strncpy((char *)&ConfigInst.get().Sunset_Latitude, tmp, sizeof(ConfigInst.get().Sunset_Latitude));
        if (getArg(PSTR("lon"), tmp, sizeof(tmp)))
            strncpy((char *)&ConfigInst.get().Sunset_Longitude, tmp, sizeof(ConfigInst.get().Sunset_Longitude));
        if (getArg(PSTR("ofsr"), tmp, sizeof(tmp)))
            ConfigInst.get().Sunset_Sunriseoffset = atoi(tmp);
        if (getArg(PSTR("ofss"), tmp, sizeof(tmp)))
            ConfigInst.get().Sunset_Sunsetoffset = atoi(tmp);

        ConfigInst.write();

        // Send response
        String html = FPSTR(restart_html);
        html = fillStandardParms(html);
        html.replace(F("{MSG}"), F("Configuration saved!"));
        mWebServer->send(200, "text/html", html);

        ConfigInst.requestRestart();

        return;
    }

    String html = FPSTR(config_ntp_html);

    html = fillStandardParms(html);

    html.replace(F("{NTPA}"), ConfigInst.get().Ntp_Server);
    sprintf((char*)&tmp, "%u", ConfigInst.get().Ntp_Port);
    html.replace(F("{NTPP}"), tmp);
    sprintf((char*)&tmp, "\"%s\"", ConfigInst.get().Ntp_TimezoneDescr);
    html.replace(tmp, String(tmp) + " selected");

    html.replace(F("{SSCHK}"), ConfigInst.get().Sunset_Enabled?"checked":"");
    html.replace(F("{LAT}"), ConfigInst.get().Sunset_Latitude);
    html.replace(F("{LON}"), ConfigInst.get().Sunset_Longitude);
    sprintf((char*)&tmp, "%i", ConfigInst.get().Sunset_Sunriseoffset);
    html.replace(F("{OFSR}"), tmp);
    sprintf((char*)&tmp, "%i", ConfigInst.get().Sunset_Sunsetoffset);
    html.replace(F("{OFSS}"), tmp);

    DBG_PRINTF(DBG_PSTR("web::showNtpCfg Sending response...\n"));
    mWebServer->send(200, "text/html", html);
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

void web::showFile(void)
{
    String resp = ConfigInst.load_from_file(CONFIG_FILENAME);

    mWebServer->send(200, F("text/plain"), resp.c_str());
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

    DBG_PRINTF(DBG_PSTR("%s\n"), args.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String web::fillStandardParms(String html)
{
    html.replace(F("{VERSION}"), mVersion);
    html.replace(F("{GIT}"), AUTO_GIT_HASH);

    return html;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool web::getArg(const char *arg, char *out, size_t max)
{
    String s = mWebServer->arg((const __FlashStringHelper *)arg);

    if (s.length() > 0)
    {
        strlcpy(out, s.c_str(), max);
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
