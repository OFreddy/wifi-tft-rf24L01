#pragma once

#include <AsyncHTTPRequest_Generic.hpp>

#include "_dbg.h"

#include "MiniGrafx.h"   // General graphic library
#include "ILI9341_SPI.h" // Hardware-specific library

#include <ArduinoJson.h>


#include "user_config.h"

class usercontentbase
{
private:
    static void httpRequestCb(void *optParm, AsyncHTTPRequest *request, int readyState);
 
 protected:
    // Http Service
    AsyncHTTPRequest *mHttpReq;

public:
    usercontentbase(/* args */);

    virtual void getHttpData(void);
    virtual void drawDisplayContent(MiniGrafx *gfx);
    virtual void httpResponseCb(AsyncHTTPRequest *request, int readyState);
};


