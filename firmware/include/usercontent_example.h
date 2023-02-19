#pragma once

#include "usercontentbase.h"

class UserContentClass : public usercontentbase
{
private:
    /* data */
    uint32_t lastHttpResponse;
    int mGridPower;

public:
    UserContentClass(/* args */);

    void getHttpData(void);
    void drawDisplayContent(MiniGrafx *gfx);
    void httpResponseCb(AsyncHTTPRequest *request, int readyState);
};
