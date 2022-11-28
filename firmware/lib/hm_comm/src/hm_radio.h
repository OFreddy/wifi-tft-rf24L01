/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Library requires project reference to 
// https://github.com/RobTillaart/CRC
#include <CRC8.h>
#include <CRC16.h>

#include <CircularBuffer.h>

#include <RF24.h>
#include <RF24_config.h>

#include <Ticker.h>

#include "hm_config_x.h"
#include "hm_config.h"
#include "hm_types.h"
#include "hm_inverter.h"


class HM_Radio
{
private:
	// CRC genaration library
	CRC8 crc8;
	CRC16 crc16;

	// Unix epoch time
	uint32_t epochTime = 0x623C8EA3;
	HM_TICKCOUNTTYPE epochTick;

	// Channels to send on and to listen to
	const uint8_t usedChannels[5] = {3, 23, 40, 61, 75};

	// Radio instance
	std::unique_ptr<RF24> _rf24Radio;

	// DTU Serial header
	Serial_header_t serialHdr;

	// Inverter instances
	HM_Inverter<float> inverter[HM_MAXINVERTERINSTANCES];
	uint8_t cntInvInst;
	uint8_t curInvInst;

	// Internal processing state
	HM_InternalState_e eState = HM_State_Idle;

	// Internal receive buffer for storing payload from irq
	CircularBuffer<HM_Packet_t, HM_PACKET_BUFFER_SIZE> packetBuffer;
	uint16_t lastPacketCRC;

	// Internal send buffer
	uint8_t sendBytes;
	uint8_t sendBuffer[HM_MAXPAYLOADSIZE];

	// Internal state processing
	bool StateMachine(void);

	// Packet handling
	void PreparePacket();
	void SendPacket(void);
	void ProcessPacket(void);

	void CalculateValues(void);

	void prepareBuffer(uint8_t *buf);
	void copyToBuffer(uint8_t *buf, uint32_t val);
	void copyToBufferBE(uint8_t *buf, uint32_t val);

	uint8_t PrepareControlPacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t cmd1, uint8_t cmd2, uint16_t *data);
	uint8_t PrepareTimePacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t cmd1, uint8_t cmd2, uint16_t alarmMesId);
	uint8_t PrepareCmdPacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t mid, uint8_t pid, bool calcCrc = true);

	// Channel hopping based on gazell link layer
	Ticker gazellTimeslot;
	static void TimeslotCallback(void *ptr);
	void IncrementChannel(uint8_t *pChannel);

	// Settings
	bool checkCRC = true;

	// Debugging
	HM_TICKCOUNTTYPE lastDump;
	bool sendLineLF;
	bool dumpRFData = false;
	uint32_t sendIntervall;

public:
	// Class constructor
	HM_Radio(RF24 *rfRadio);

	// Init and startup
	bool Begin(void);

	// Interrupt callback
	void RadioIrqCallback(void);

	// Cylcic processing routine
	bool Cyclic(void);

	// Packet buffer handling
	bool PacketAvailable(void);
	HM_Packet_t GetPacket(void);

	void SetUnixTimeStamp(uint32_t ts);
	uint32_t GetUnixTimeStamp(void);
	void UnixTimeStampTick();

	// Inverter instances
	uint64_t SerialString2u64(const char *val);
	bool AddInverterInstance(uint64_t addr);
	HM_Inverter<float> *GetInverterInstance(uint8_t inst);

	// Debugging routines
	void dumpMillis(HM_TICKCOUNTTYPE mil);
	void dumpData(uint8_t *p, int len);

	void DumpRFData(bool dump)
    {
        dumpRFData = dump;
    }
};

