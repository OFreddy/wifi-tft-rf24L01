#if defined(ESP32) && defined(F)
  #undef F
  #define F(sl) (sl)
#endif

#include "web.h"

#include "html/h/index_html.h"
#include "html/h/style_css.h"
#include "favicon.h"
#include "html/h/setup_html.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

web::web(app *main, char version[]) {
    mMain    = main;
    mVersion = version;
    #ifdef ESP8266
        mWeb     = new ESP8266WebServer(80);
        mUpdater = new ESP8266HTTPUpdateServer();
    #elif defined(ESP32)
        mWeb     = new WebServer(80);
        mUpdater = new HTTPUpdateServer();
    #endif
    mUpdater->setup(mWeb);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::setup(void) {

    mWeb->begin();

    mWeb->on("/",               std::bind(&web::showIndex,         this));
    mWeb->on("/style.css",      std::bind(&web::showCss,           this));
    mWeb->on("/favicon.ico",    std::bind(&web::showFavicon,       this));
    mWeb->onNotFound (          std::bind(&web::showNotFound,      this));
    mWeb->on("/uptime",         std::bind(&web::showUptime,        this));
    mWeb->on("/time",           std::bind(&web::showTime,          this));
    mWeb->on("/cmdstat",        std::bind(&web::showStatistics,    this));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::loop(void) {
    mWeb->handleClient();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showIndex(void) {
    String html = FPSTR(index_html);
    html.replace(F("{DEVICE}"), String("Ollis"));
    html.replace(F("{VERSION}"), mVersion);
    html.replace(F("{TS}"), String(5) + " ");
    html.replace(F("{JS_TS}"), String(5 * 1000));
    html.replace(F("{BUILD}"), String(4711));
    mWeb->send(200, "text/html", html);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showCss(void)
{
    mWeb->send(200, "text/css", FPSTR(style_css));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showFavicon(void)
{
    static const char favicon_type[] PROGMEM = "image/x-icon";
    static const char favicon_content[] PROGMEM = FAVICON_PANEL_16;
    mWeb->send_P(200, favicon_type, favicon_content, sizeof(favicon_content));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showNotFound(void)
{
    DBG_PRINTF(DBG_PSTR("app::showNotFound - %s"), mWeb->uri());
    String msg = F("File Not Found\n\nURI: ");
    msg += mWeb->uri();
    msg += F("\nMethod: ");
    msg += (mWeb->method() == HTTP_GET) ? "GET" : "POST";
    msg += F("\nArguments: ");
    msg += mWeb->args();
    msg += "\n";

    for (uint8_t i = 0; i < mWeb->args(); i++)
    {
        msg += " " + mWeb->argName(i) + ": " + mWeb->arg(i) + "\n";
    }

    mWeb->send(404, F("text/plain"), msg);
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

    mWeb->send(200, "text/plain", String(time));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showTime(void)
{
    char time_str[11];
    time_t now = time(nullptr);
    struct tm *timeinfo = localtime(&now);

    //String datetime = WDAY_NAMES[timeinfo->tm_wday] + " " + String(timeinfo->tm_mday) + " " + MONTH_NAMES[timeinfo->tm_mon] + " " + String(1900 + timeinfo->tm_year);
    String datetime = String("olliDay");

    sprintf(time_str, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec); // hh:mm:ss

    datetime += String(", ") + String(time_str);

    mWeb->send(200, "text/plain", datetime.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void web::showStatistics(void) 
{
    mWeb->send(200, F("text/plain"), String("Statistics"));
}
