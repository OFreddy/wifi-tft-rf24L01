#pragma once

#include "_dbg.h"
#ifdef ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#elif defined(ESP32)
#include <WebServer.h>
#include <HTTPUpdateServer.h>
#endif

#include "app.h"

class app;

class web
{
public:
    web(app *main, char version[]);
    ~web() {}

    void setup(void);
    void loop(void);

    void showIndex(void);
    void showCss(void);
    void showFavicon(void);
    void showNotFound(void);
    void showUptime(void);
    void showTime(void);
    void showStatistics(void);

private:
#ifdef ESP8266
    ESP8266WebServer *mWeb;
    ESP8266HTTPUpdateServer *mUpdater;
#elif defined(ESP32)
    WebServer *mWeb;
    HTTPUpdateServer *mUpdater;
#endif

    // config_t *mConfig;
    // sysConfig_t *mSysCfg;
    char *mVersion;
    app *mMain;
};


