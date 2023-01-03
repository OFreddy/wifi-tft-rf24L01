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
    void showConfig(void);
    void showWifiCfg(void);
    void showNtpCfg(void);
    void showUptime(void);
    void showTime(void);
    void showStatistics(void);
    void showFile(void);

private:
#ifdef ESP8266
    ESP8266WebServer *mWebServer;
    ESP8266HTTPUpdateServer *mUpdater;
#elif defined(ESP32)
    WebServer *mWeb;
    HTTPUpdateServer *mUpdater;
#endif

    void dumpReceivedArgs(void);
    String fillStandardParms(String html);
    bool getArg(const char* arg, char* out, size_t max);

    char *mVersion;
    app *mMain;
};
