
#include <arduino.h>
#include "_dbg.h"

#include "ArialRounded.h"
#include "tft_custom.h"

#include "usercontentbase.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

usercontentbase::usercontentbase(/* args */)
{
    // http client
    mHttpReq = new AsyncHTTPRequest();
    mHttpReq->setDebug(false);
    mHttpReq->onReadyStateChange(httpRequestCb, this);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usercontentbase::getHttpData(void)
{

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usercontentbase::httpRequestCb(void *optParm, AsyncHTTPRequest *request, int readyState)
{
    usercontentbase *inst = (usercontentbase *)optParm;

    inst->httpResponseCb(request, readyState);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usercontentbase::httpResponseCb(AsyncHTTPRequest *request, int readyState)
{
  
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usercontentbase::drawDisplayContent(MiniGrafx *gfx)
{
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setColor(MINI_RED);
    gfx->drawString(120, 80, "No custom");
    gfx->drawString(120, 112, "content!");
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->setColor(MINI_2ND);
    gfx->drawString(120, 162, "Rename usercontent_example.h");
    gfx->drawString(120, 178, "and usercontent_example.cpp");
    gfx->drawString(120, 194, "to usercontent.h and .cpp");
    gfx->drawString(120, 210, "and implement your own content.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if __has_include("usercontent.h")
  #include "usercontent.h"              // Custom user content

UserContentClass UserContentInst;
#else
usercontentbase UserContentInst;
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
