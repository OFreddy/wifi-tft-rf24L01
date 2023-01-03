/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include <Arduino.h>

#include "_dbg.h"

#include "TimeoutHelper.h"
#include "config.h"
#include "confignetwork.h"
#include "network.h"

#include "user_config.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass::NetworkClass()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass::~NetworkClass()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::setup()
{
    CONFIG_T &config = ConfigInst.get();

    // Dns and Udp
    mDnsServer = new DNSServer();
    mUdp = new WiFiUDP();

    // Check Wifi settings
    mWifiSet1Valid = strlen(config.WiFi_Ssid1) > 0 && strlen(config.WiFi_Password1) > 8;
    mWifiSet2Valid = strlen(config.WiFi_Ssid2) > 0 && strlen(config.WiFi_Password2) > 8;
    if (!mWifiSet1Valid)
        DBG_PRINTF(DBG_PSTR("WiFi Setting 1 is invalid!\n"));
    if (!mWifiSet2Valid)
        DBG_PRINTF(DBG_PSTR("WiFi Setting 2 is invalid!\n"));

    networkConfig = eNetworkConfigUse1;
    networkState = eNetworkStateInit;

    hostname = ConfigNetworkInst.GetHostname();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::loop()
{
    statemachine();

    if (WiFi.status() != mOldWifiStatus)
    {
        mOldWifiStatus = WiFi.status();
        if (WiFi.status() == WL_CONNECTED)
        {
            DBG_PRINTF(DBG_PSTR("WiFi connected. IP address: %s\n"), WiFi.localIP().toString().c_str());
        }
        else
        {
            DBG_PRINTF(DBG_PSTR("WiFi status changed to %u\n"), WiFi.status());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool NetworkClass::IsConnected(void)
{
    return WiFi.status() == WL_CONNECTED;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int8_t NetworkClass::GetWifiQuality(void)
{
    // Convert the dBm value into a percentual range
    int32_t dbm = WiFi.RSSI();
    if ((dbm <= -100) || !WiFi.isConnected())
    {
        return 0;
    }
    else if (dbm >= -50)
    {
        return 100;
    }
    else
    {
        return 2 * (dbm + 100);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkState_E NetworkClass::GetNetworkState()
{
    return networkState;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String NetworkClass::GetStationSSID(void)
{
    return mSTA_SSIDName;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::statemachine(void)
{
    switch (networkState)
    {
    case eNetworkStateInit:
        // Choose active config
        chooseconfig();
        if (!mApActive)
        {
            WifiBeginSTA();

            connectTimeout.set(10000);
            networkState = eNetworkstateSTAConnect;
        }
        else
        {
            WifiBeginAP(WIFI_AP_SSID, WIFI_AP_PWD);
            connectTimeout.set(WIFI_AP_ACTIVE_TIME * 1000);
            networkState = eNetworkstateAP;
        }
        break;

    case eNetworkstateSTAConnect:
        if (WiFi.status() == WL_CONNECTED)
        {
            DBG_PRINTF(DBG_PSTR("WiFi connected.\n"));
            networkState = eNetworkstateSTAConnected;
        }
        else if (connectTimeout.occured())
        {
            DBG_PRINTF(DBG_PSTR("WiFi connection timeout!\n"));
            networkState = eNetworkstateSTADisconnected;
        }
        break;

    case eNetworkstateSTAConnected:
        if (WiFi.status() != WL_CONNECTED)
            networkState = eNetworkstateSTADisconnected;
        break;

    case eNetworkstateSTADisconnected:
        networkState = eNetworkstateNext;
        break;

    case eNetworkstateAP:
        mDnsServer->processNextRequest();
        if (WiFi.softAPgetStationNum() > 0) // Station connected to AP => Extend timeout
            connectTimeout.set(WIFI_AP_ACTIVE_TIME * 1000);
        if (connectTimeout.occured())
        {
            networkState = eNetworkstateNext;
        }
        break;

    case eNetworkstateNext:
        networkConfig = (NetworkConfig_E)(networkConfig + 1);
        if (networkConfig >= eNetworkConfigIll)
            networkConfig = eNetworkConfigUse1;
        networkState = eNetworkStateInit;
        break;

    default:
        networkState = eNetworkStateInit;
        break;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::chooseconfig(void)
{
    while (true)
    {
        if (networkConfig == eNetworkConfigUse1)
        {
            if (mWifiSet1Valid)
            {
                mSTA_SSIDName = String(ConfigInst.get().WiFi_Ssid1);
                mSTA_Pass = String(ConfigInst.get().WiFi_Password1);
                mApActive = false;
                break;
            }
            else
                networkConfig = eNetworkConfigUse2;
        }
        else if (networkConfig == eNetworkConfigUse2)
        {
            if (mWifiSet2Valid)
            {
                mSTA_SSIDName = String(ConfigInst.get().WiFi_Ssid2);
                mSTA_Pass = String(ConfigInst.get().WiFi_Password2);
                mApActive = false;
                break;
            }
            else
                networkConfig = eNetworkConfigAP;
        }
        else if (networkConfig == eNetworkConfigAP)
        {
            mApActive = true;
            break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiBeginSTA(void)
{
    WiFi.persistent(false);
    WiFi.disconnect(true);
    delay(200);

    WifiSetMode(WIFI_STA);
    WifiSetSleepMode();

#ifdef ESP32
    if (Wifi.phy_mode)
    {
        WiFi.setPhyMode(WiFiPhyMode_t(Wifi.phy_mode)); // 1-B/2-BG/3-BGN
    }
#endif

    if (!WiFi.getAutoConnect())
    {
        WiFi.setAutoConnect(true);
    }

    WiFi.hostname(hostname);

    DBG_PRINTF(DBG_PSTR("WiFi connecting to %s...\n"), mSTA_SSIDName.c_str());
    WiFi.begin(mSTA_SSIDName, mSTA_Pass);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiBeginAP(const char *ssid, const char *pwd)
{
    IPAddress apIp(192, 168, 1, 1);

    WiFi.persistent(false);
    WiFi.disconnect(true);
    delay(200);

    int channel = WIFI_SOFT_AP_CHANNEL;
    if ((channel < 1) || (channel > 13))
        channel = 1;

    WifiSetMode(WIFI_AP);
    DBG_PRINTF(DBG_PSTR("WiFi starting access point mode SSID %s PASS %s channel %i...\n"), WIFI_AP_SSID, WIFI_AP_PWD, channel);

    if (!WiFi.softAPConfig(apIp, apIp, IPAddress(255, 255, 255, 0)))
        DBG_PRINTF(DBG_PSTR("Failed to config SoftAP!\n"));
    if (!WiFi.softAP(ssid, pwd, channel, 0, 1))
        DBG_PRINTF(DBG_PSTR("Failed to start SoftAP!\n"));
    delay(500); // Without delay IP addres may be blank

    mDnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    if (!mDnsServer->start(WIFI_SOFT_AP_DNS_PORT, "*", WiFi.softAPIP()))
        DBG_PRINTF(DBG_PSTR("Failed to start DNS Server!\n"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetMode(WiFiMode_t wifi_mode)
{
    DBG_PRINTF(DBG_PSTR("Setting WiFi mode to %u...\n"), wifi_mode);
    if (WiFi.getMode() == wifi_mode)
    {
        DBG_PRINTF(DBG_PSTR("WiFimode already set!\n"));
        return;
    }

    if (wifi_mode != WIFI_OFF)
    {
        WiFi.hostname(hostname); // ESP32 needs this here (before WiFi.mode) for core 2.0.0
        // See: https://github.com/esp8266/Arduino/issues/6172#issuecomment-500457407
        WiFi.forceSleepWake(); // Make sure WiFi is really active.
        delay(100);
    }

    uint32_t retry = 2;
    while (!WiFi.mode(wifi_mode) && retry--)
    {
        DBG_PRINTF(DBG_PSTR("Retry set Mode...\n"));
        delay(100);
    }

    if (wifi_mode == WIFI_OFF)
    {
        delay(1000);
        WiFi.forceSleepBegin();
        delay(1);
    }
    else
    {
        delay(30); // Must allow for some time to init.
    }
    DBG_PRINTF(DBG_PSTR("WiFimode sucessfully set.\n"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetSleepMode(void)
{
    bool wifi_no_sleep = true; // TODO: Move to settings
    bool wifi_normal_sleep = false;

    if (wifi_no_sleep)
    {
        WiFi.setSleepMode(WIFI_NONE_SLEEP); // Disable sleep
    }
    else
    {
        if (wifi_normal_sleep)
            WiFi.setSleepMode(WIFI_LIGHT_SLEEP); // Allow light sleep during idle times
        else
            WiFi.setSleepMode(WIFI_MODEM_SLEEP); // Sleep (Esp8288/Arduino core and sdk default)
    }
    WifiSetOutputPower();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void NetworkClass::WifiSetOutputPower(void)
{
    // Set output power in thre range from max: +20.5dBm  to min: 0dBm
    WiFi.setOutputPower((float)20.5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

NetworkClass NetworkInst; // Network class instance singleton

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
