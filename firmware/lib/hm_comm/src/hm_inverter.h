/*
 Copyright (C)
    2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Global includes
#include <memory>
#include <queue>

#include <TimeoutHelper.h>

#include "hm_config.h"
#include "hm_config_x.h"
#include "hm_data.h"
#include "hm_types.h"

// prototypes

template <class RECORDTYPE = float>
class HM_Inverter;


class CommandBase
{
public:
    CommandBase(uint8_t txType = 0, uint8_t cmd = 0)
    {
        _TxType = txType;
        _Cmd = cmd;
    };
    virtual ~CommandBase(){};

    const uint8_t getCmd()
    {
        return _Cmd;
    }

protected:
    uint8_t _TxType;
    uint8_t _Cmd;
};

class CommandInfo : public CommandBase
{
public:
    CommandInfo(uint8_t cmd)
    {
        _TxType = 0x15;
        _Cmd = cmd;
    }
};

template <class RECORDTYPE>
class HM_Inverter
{
public:
    // Inverter unique id
    uint8_t id;

    // Inverter data
    serial_u serial;
    serial_u rfAddress;

    // Channel handling
    HM_TICKCOUNTTYPE channelHopTick;
    uint8_t activeRcvChannel;
    uint8_t activeSndChannel;

    // Retries
    int16_t retries;

    // Timeout monitoring
    TimeoutHelper rcvPeriod;
    volatile int16_t rcvRetryToChannelSwitch;

    // Payload data
    uint32_t lastPayloadRcvTime;
    uint8_t payload[HM_MAX_PAYLOAD_ENTRIES][HM_MAX_RF_NET_PAYLOAD_SIZE];
    uint8_t payloadLastFrame;
    uint16_t payloadFrameFlag;

    // Inverter type and data
    uint8_t type;            // integer which refers to inverter type
    HM_ByteAssign_t *assign; // type of inverter
    uint8_t listLen;         // length of assignments
    uint8_t channels;        // number of PV channels (1-4)
    RECORDTYPE *record;      // pointer for values

    uint16_t alarmMesIndex;  // Last recorded Alarm Message Index
    uint16_t fwVersion;      // Firmware Version from Info Command Request
    uint16_t powerLimit[2];  // limit power output
    uint16_t actPowerLimit;  //
    uint16_t chMaxPwr[4];    // maximum power of the modules (Wp)

    String lastAlarmMsg;

    // Class constructor
    HM_Inverter(void)
    {
    }

    // Initilization
    void Init(void)
    {
        getAssignment();
        record = new RECORDTYPE[listLen];
        memset(record, 0x00, sizeof(RECORDTYPE) * listLen);
    }

    // Command queue
    template <typename T>
    void enqueCommand(uint8_t cmd)
    {
        _commandQueue.push(std::make_shared<T>(cmd));
    }

    uint8_t getQueuedCmd()
    {
        if (_commandQueue.empty())
        {
            // Fill with default commands
            enqueCommand<CommandInfo>(eCmd_RealTimeRunData_Debug);
        }

        return _commandQueue.front().get()->getCmd();
    }

    void setQueuedCmdFinished()
    {
        if (!_commandQueue.empty())
        {
            // Will destroy CommandAbstract Class Object (?)
            _commandQueue.pop();
        }
    }

    void clearCmdQueue(void)
    {
        while (!_commandQueue.empty())
        {
            // Will destroy CommandAbstract Class Object (?)
            _commandQueue.pop();
        }
    }

    // Value converter
    bool getAssignment(void);
    uint8_t getPosByChFld(uint8_t channel, uint8_t fieldId);
    void addValue(uint8_t pos, uint8_t buf[]);
    RECORDTYPE getValue(uint8_t pos)
    {
        return record[pos];
    }
    const char *getFieldName(uint8_t pos)
    {
        return fields[assign[pos].fieldId];
    }
    const char *getUnit(uint8_t pos)
    {
        return units[assign[pos].unitId];
    }
    void doCalculations();

private:
    std::queue<std::shared_ptr<CommandBase>> _commandQueue;
};


