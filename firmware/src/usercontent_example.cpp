#include <arduino.h>

#include "ArialRounded.h"
#include "tft_custom.h"

#if __has_include("usercontent.h") 
#include "usercontent.h"
#else
#include "usercontent_example.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UserContentClass::UserContentClass(/* args */)
{
    mHttpReq->setReqHeader("accept", "application/json");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserContentClass::getHttpData(void)
{
    // Insert your custom HTTP request here
    const String requestUrl = "http://192.168.178.3:8087/getBulk/alias.0.Energie.Strom.Netz.bezug_watt";

    if (mHttpReq->readyState() == readyStateUnsent || mHttpReq->readyState() == readyStateDone)
    {
        bool requestOpenResult = mHttpReq->open("GET", requestUrl.c_str());

        if (requestOpenResult)
        {
            // Only send() if open() returns true, or crash
            DBG_PRINTF(DBG_PSTR("%08i Sending Http request...\n"), millis());
            mHttpReq->send();
        }
        else
        {
            DBG_PRINTF(DBG_PSTR("Can't send bad request\n"));
            DBG_PRINTF(requestUrl.c_str());
        }
    }
    else
    {
        (DBG_PSTR("Can't send request\n"));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserContentClass::drawDisplayContent(MiniGrafx *gfx)
{
    // Insert your custom display content here

    char buf[32];

    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    if (lastHttpResponse + HTTP_RESPONSE_TIMEOUT < millis())
    {
        gfx->setColor(MINI_RED);
        gfx->drawString(120, 80, "Data timeout!");
    }

    gfx->setColor(MINI_2ND);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);

    gfx->drawString(120, 100, "Power:");
    sprintf_P(&buf[0], PSTR("%u W"), mGridPower);
    gfx->drawString(120, 135, buf);

    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(120, 194, "Last Response:");
    gfx->setFont(ArialRoundedMTBold_36);
    gfx->setColor(MINI_RED);
    sprintf_P(&buf[0], PSTR(" %u s"), (millis() - lastHttpResponse) / 1000);
    gfx->drawString(120, 210, buf);

    gfx->setColor(MINI_WHITE);
    gfx->setFont(ArialRoundedMTBold_14);
    gfx->drawString(120, 284, "Here could be");
    gfx->drawString(120, 300, "your advertising!");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserContentClass::httpResponseCb(AsyncHTTPRequest *request, int readyState)
{
    // Insert your custom http Response processing
    DBG_PRINTF(DBG_PSTR("Custom response callback...\n"), millis());

    if (readyState == readyStateDone)
    {
        String httpResponse = request->responseText();

        DBG_PRINTF(DBG_PSTR("\n%08i **ReadyStateDone %i\n"), millis(), request->responseHTTPcode());

        if (request->responseHTTPcode() == -4 || request->responseHTTPcode() == -5)
        {
            // Something went wrong
        }

        if (request->responseHTTPcode() != 200)
            return;

        if (httpResponse.length() == 0)
        {
            DBG_PRINTF(DBG_PSTR("\n%08i Http received empty response\n"), millis());
            // inst->getHttpData();
            return;
        }

        // retriggerTicker(&inst->mHttpReqTicker, inst->mHttpReqInterval);

        DBG_PRINTF(DBG_PSTR("\n%08i **************************************\n"), millis());
        DBG_PRINTF(httpResponse.c_str());
        DBG_PRINTF(DBG_PSTR("\n**************************************\n"));

        // Deserializing
        // see https://arduinojson.org/v6/assistant/#/step1
        StaticJsonDocument<32> filter;
        filter[0]["val"] = true;

        StaticJsonDocument<192> doc;

        DeserializationError error = deserializeJson(doc, httpResponse.c_str(), DeserializationOption::Filter(filter));

        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
            return;
        }

        JsonArray root = doc.as<JsonArray>();

        mGridPower = root[0]["val"]; // Power

        lastHttpResponse = millis();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
