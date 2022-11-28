/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#ifndef __HM_TYPES_H__
#define __HM_TYPES_H__

#include <stdint.h>
#include "hm_config.h"


#define HM_PACKETTYPE_RESPONSE   0x80
#define HM_PACKETTYPE_INFO       0X15
#define HM_PACKETTYPE_DEVCONTROL 0x51
#define ALL_FRAMES          0x80
#define SINGLE_FRAME        0x81

typedef enum {
    eCmd_InverterDevInform_Simple = 0,   // 0x00
    eCmd_InverterDevInform_All = 1,      // 0x01
    eCmd_GridOnProFilePara = 2,        // 0x02
    eCmd_HardWareConfig = 3,           // 0x03
    eCmd_SimpleCalibrationPara = 4,    // 0x04
    eCmd_SystemConfigPara = 5,           // 0x05
    eCmd_RealTimeRunData_Debug = 11,     // 0x0b
    eCmd_RealTimeRunData_Reality = 12, // 0x0c
    eCmd_RealTimeRunData_A_Phase = 13, // 0x0d
    eCmd_RealTimeRunData_B_Phase = 14, // 0x0e
    eCmd_RealTimeRunData_C_Phase = 15, // 0x0f
    eCmd_AlarmData = 17,                 // 0x11, Alarm data - all unsent alarms
    eCmd_AlarmUpdate = 18,               // 0x12, Alarm data - all pending alarms
    eCmd_RecordData = 19,              // 0x13
    eCmd_InternalData = 20,            // 0x14
    eCmd_GetLossRate = 21,               // 0x15
    eCmd_GetSelfCheckState = 30,       // 0x1e
    eCmd_InitDataState = 0xff
} InfoCmdType_e;

typedef enum {
    eCmd_TurnOn = 0,                   // 0x00
    eCmd_TurnOff = 1,                  // 0x01
    eCmd_Restart = 2,                  // 0x02
    eCmd_Lock = 3,                     // 0x03
    eCmd_Unlock = 4,                   // 0x04
    eCmd_ActivePowerContr = 11,        // 0x0b
    eCmd_ReactivePowerContr = 12,      // 0x0c
    eCmd_PFSet = 13,                   // 0x0d
    eCmd_CleanState_LockAndAlarm = 20, // 0x14
    eCmd_SelfInspection = 40,          // 0x28, self-inspection of grid-connected protection files
    eCmd_Init = 0xff
} DevControlCmdType_e;

typedef struct _Serial_header_t
{
  uint32_t timestamp;
  uint8_t packetsLost;
  uint8_t address[HM_RF_ADDRESSWIDTH]; // MSB first, always RF_MAX_ADDR_WIDTH bytes.
} Serial_header_t;

typedef struct _HM_Packet_t
{
    uint32_t timestamp;
    uint8_t packetsLost;
    uint8_t channel;
    uint8_t packet[HM_MAXPAYLOADSIZE];
} HM_Packet_t;

typedef enum 
{
    HM_State_Idle,
    HM_State_SetInvInstance,
    HM_State_Send,
    HM_State_CheckResponse,
    HM_State_Calculate,
    HM_State_CycleEnd,
    HM_State_SendDelay
} HM_InternalState_e;

// CH0 is default channel (freq, ac, temp)
enum {CH0 = 0, CH1, CH2, CH3, CH4};

enum {INV_TYPE_1CH = 0, INV_TYPE_2CH, INV_TYPE_4CH};

typedef struct {
    uint8_t    fieldId; // field id
    uint8_t    unitId;  // uint id
    uint8_t    ch;      // channel 0 - 4
    uint8_t    start;   // pos of first byte in buffer
    uint8_t    num;     // number of bytes in buffer
    uint16_t   div;     // divisor / calc command
} HM_ByteAssign_t;

union serial_u {
    uint64_t u64;
    uint8_t  b[8];
};

#endif // __HM_TYPES_H__
