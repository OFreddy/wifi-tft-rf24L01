/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#include "Arduino.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Class forward declaration

#include "hm_crc.h"
#include "hm_radio.h"
#include "hm_data.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Macros
#define HM_CNT_CHANNELS (sizeof(usedChannels) / sizeof(usedChannels[0]))

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HM_Radio::HM_Radio(RF24 *rfRadio)
{
	// initialize internal structures
	cntInvInst = 0;

	// RF 24 radio instance
	_rf24Radio.reset(rfRadio);

	// CRC Generator for crc8 generation in frame
	crc8.setPolynome(0x01);
	crc8.setStartXOR(0);
	crc8.setEndXOR(0);

	// CRC Generator for crc16 generation in frame (Modbus compatible)
	crc16.setPolynome((uint16_t)0x18005);
	crc16.setStartXOR(0xFFFF);
	crc16.setEndXOR(0x0000);
	crc16.setReverseIn(true);
	crc16.setReverseOut(true);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HM_Radio::Begin()
{
	// Startup radio hardware
	if (!_rf24Radio->begin())
		return false;

	// Unix timestamp emulation
	epochTick = HM_GETTICKCOUNT;

	// Prepare serial header for shockburst crc calculation
	// Initialize serial header's address member to promiscuous address.
	uint64_t addr = HM_DTU_RADIO_ID;
	for (int8_t i = sizeof(serialHdr.address) - 1; i >= 0; --i)
	{
		serialHdr.address[i] = addr;
		addr >>= 8;
	}

	// Radio settings
	_rf24Radio->setDataRate(HM_RF_DATARATE);
	_rf24Radio->disableCRC();
	_rf24Radio->setPayloadSize(HM_MAXPAYLOADSIZE);
	_rf24Radio->setAddressWidth(HM_RF_ADDRESSWIDTH);

	// Send PA level for transmitting packages
	_rf24Radio->setPALevel(HM_RF_PA_LEVEL);

	// Disable shockburst for receiving and decode payload manually
	_rf24Radio->setAutoAck(false);
	_rf24Radio->setRetries(0, 0);

	// We wan't only RX irqs
	_rf24Radio->maskIRQ(true, true, false);

	// Configure listening pipe with the simulated DTU address and start listening
	_rf24Radio->openReadingPipe(1, HM_DTU_RADIO_ID);

	// Gazell timeslot Ticker
	gazellTimeslot.attach_ms(4, TimeslotCallback, (void *)this);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint64_t HM_Radio::SerialString2u64(const char *val)
{
	char tmp[3];
	uint64_t ret = 0ULL;
	uint64_t u64;
	memset(tmp, 0, 3);
	for (uint8_t i = 0; i < 6; i++)
	{
		tmp[0] = val[i * 2];
		tmp[1] = val[i * 2 + 1];
		if ((tmp[0] == '\0') || (tmp[1] == '\0'))
			break;
		u64 = strtol(tmp, NULL, 16);
		ret |= (u64 << ((5 - i) << 3));
	}
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HM_Radio::AddInverterInstance(uint64_t serial)
{
	// Number of configured inverter instances exceeded
	if (cntInvInst >= HM_MAXINVERTERINSTANCES)
		return false;

	// inverter address
	HM_PRINTF(HM_PSTR("Adding inverter %12lX\n"), serial);
	inverter[cntInvInst].rfAddress.u64 = 0ULL;
	inverter[cntInvInst].serial.u64 = serial;
	for (int8_t i = HM_RF_ADDRESSWIDTH - 2; i >= 0; i--)
	{
		inverter[cntInvInst].rfAddress.b[i] = serial;
		serial >>= 8;
	}

	dumpData(&inverter[cntInvInst].serial.b[0], sizeof(inverter[cntInvInst].serial.b));
	dumpData(&inverter[cntInvInst].rfAddress.b[0], sizeof(inverter[cntInvInst].rfAddress.b));
	HM_PRINTF(HM_PSTR("\n"));

	// Inverter type
	if (inverter[cntInvInst].serial.b[5] == 0x11)
	{
		switch (inverter[cntInvInst].serial.b[4])
		{
		case 0x21:
			inverter[cntInvInst].type = INV_TYPE_1CH;
			break;
		case 0x41:
			inverter[cntInvInst].type = INV_TYPE_2CH;
			break;
		case 0x61:
			inverter[cntInvInst].type = INV_TYPE_4CH;
			break;
		default:
			HM_PRINTF(HM_PSTR("unknown inverter type: 11"));
			HM_PRINTF(String(inverter[cntInvInst].serial.b[4], HEX).c_str());
			break;
		}
	}
	else
		HM_PRINTF(HM_PSTR("inverter type can't be detected!"));

	// preset instance data
	inverter[cntInvInst].activeSndChannel = usedChannels[0];
	inverter[cntInvInst].activeRcvChannel = usedChannels[HM_CNT_CHANNELS - 1];

	inverter[cntInvInst].Init();

	// Inkrement inverter instance
	cntInvInst++;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HM_Inverter<float> *HM_Radio::GetInverterInstance(uint8_t inst)
{
	if (inst >= cntInvInst)
		return NULL;

	return &inverter[inst];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::RadioIrqCallback(void)
{
	static uint8_t lostPacketCount = 0;
	uint8_t pipe;

	// Loop until RX buffer(s) contain no more packets.
	while (_rf24Radio->available(&pipe))
	{
		if (!packetBuffer.isFull())
		{
			HM_Packet_t p;
			p.timestamp = millis();
			p.channel = inverter[curInvInst].activeRcvChannel;
			p.packetsLost = lostPacketCount;
			uint8_t packetLen = _rf24Radio->getPayloadSize();
			if (packetLen > HM_MAXPAYLOADSIZE)
				packetLen = HM_MAXPAYLOADSIZE;

			_rf24Radio->read(p.packet, packetLen);

			// Get payload length and id from PCF
			uint8_t payloadLen = ((p.packet[0] & 0xFC) >> 2);
			// uint8_t payloadID = (p.packet[0] & 0x03); // For processing of repeated frames

			// Add to buffer for further processing
			if (HM_MAXPAYLOADSIZE >= payloadLen)
			{
				packetBuffer.unshift(p);
			}

			lostPacketCount = 0;
		}
		else
		{
			// Buffer full. Increase lost packet counter.
			bool tx_ok, tx_fail, rx_ready;
			if (lostPacketCount < 255)
				lostPacketCount++;
			// Call 'whatHappened' to reset interrupt status.
			_rf24Radio->whatHappened(tx_ok, tx_fail, rx_ready);
			// Flush buffer to drop the packet.
			_rf24Radio->flush_rx();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HM_Radio::Cyclic(void)
{
	// Simulate unix timestamp
	if (HM_GETTICKCOUNT >= epochTick)
	{
		epochTick += 1000;
		UnixTimeStampTick();
	}

	return StateMachine();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HM_Radio::PacketAvailable(void)
{
	return (!packetBuffer.isEmpty());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

HM_Packet_t HM_Radio::GetPacket(void)
{
	return packetBuffer.pop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::SetUnixTimeStamp(uint32_t ts)
{
	epochTime = ts;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t HM_Radio::GetUnixTimeStamp(void)
{
	return epochTime;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::UnixTimeStampTick()
{
	epochTime++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::prepareBuffer(uint8_t *buf)
{
	// minimal buffer size of 32 bytes is assumed
	memset(buf, 0x00, HM_MAXPAYLOADSIZE);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::copyToBuffer(uint8_t *buf, uint32_t val)
{
	buf[0] = (uint8_t)(val >> 24);
	buf[1] = (uint8_t)(val >> 16);
	buf[2] = (uint8_t)(val >> 8);
	buf[3] = (uint8_t)(val & 0xFF);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::copyToBufferBE(uint8_t *buf, uint32_t val)
{
	memcpy(buf, &val, sizeof(uint32_t));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t HM_Radio::PrepareControlPacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t cmd1, uint8_t cmd2, uint16_t *data)
{
	PrepareCmdPacket(wrAdr, dtuAdr, HM_PACKETTYPE_DEVCONTROL, ALL_FRAMES, false);

	sendBuffer[10] = cmd1; // cmd --> 0x0b => Type_ActivePowerContr, 0 on, 1 off, 2 restart, 12 reactive power, 13 power factor
	sendBuffer[11] = cmd2;

	if ((cmd1 == eCmd_ActivePowerContr) || (cmd1 == eCmd_PFSet))
	{
		sendBuffer[12] = ((data[0] * 10) >> 8) & 0xff; // power limit
		sendBuffer[13] = ((data[0] * 10)) & 0xff;	   // power limit
		sendBuffer[14] = ((data[1]) >> 8) & 0xff;	   // setting for persistens handlings
		sendBuffer[15] = ((data[1])) & 0xff;		   // setting for persistens handling
	}

	// CRC16
	crc16.restart();
	crc16.add(&sendBuffer[10], 6);
	sendBuffer[16] = crc16.getCRC() >> 8;
	sendBuffer[17] = crc16.getCRC() & 0xFF;

	// CRC16
	crc8.restart();
	crc8.add(&sendBuffer[0], 18);
	sendBuffer[18] = crc8.getCRC();

	return 19;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t HM_Radio::PrepareTimePacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t cmd1, uint8_t cmd2, uint16_t alarmMesId)
{
	PrepareCmdPacket(wrAdr, dtuAdr, HM_PACKETTYPE_INFO, ALL_FRAMES, false);

	sendBuffer[10] = cmd1;
	sendBuffer[11] = cmd2;

	copyToBuffer(&sendBuffer[12], epochTime);

	if ((cmd1 == eCmd_RealTimeRunData_Debug) || (cmd1 == eCmd_AlarmData))
	{
		sendBuffer[18] = (alarmMesId >> 8) & 0xff;
		sendBuffer[19] = (alarmMesId)&0xff;
	}

	// CRC16
	crc16.restart();
	crc16.add(&sendBuffer[10], 14);
	sendBuffer[24] = crc16.getCRC() >> 8;
	sendBuffer[25] = crc16.getCRC() & 0xFF;

	// CRC16
	crc8.restart();
	crc8.add(&sendBuffer[0], 26);
	sendBuffer[26] = crc8.getCRC();

	return 27;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t HM_Radio::PrepareCmdPacket(uint64_t wrAdr, uint32_t dtuAdr, uint8_t mid, uint8_t pid, bool calcCrc)
{
	prepareBuffer(reinterpret_cast<uint8_t *>(sendBuffer));

	sendBuffer[0] = mid;							 // Message Id
	copyToBufferBE(&sendBuffer[1], (uint32_t)wrAdr); // Inverter address
	copyToBufferBE(&sendBuffer[5], dtuAdr);			 // Dtu Address
	sendBuffer[9] = pid;							 // Process Id

	if (calcCrc)
	{
		// crc8
		crc8.restart();
		crc8.add(&sendBuffer[0], 26);
		sendBuffer[10] = crc8.getCRC();
	}

	return 11;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::dumpMillis(HM_TICKCOUNTTYPE mil)
{
	HM_PRINTF(HM_PSTR("%05u."), mil / 1000);
	HM_PRINTF(HM_PSTR("%03u|"), mil % 1000);
}

void HM_Radio::dumpData(uint8_t *p, int len)
{
	while (len--)
	{
		HM_PRINTF(HM_PSTR("%02X"), *p++);
	}
	HM_PRINTF(HM_PSTR("|"));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// private member implementation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::TimeslotCallback(void *ptr)
{
	HM_Radio *inst = (HM_Radio *)ptr;

	if (inst->eState == HM_State_CheckResponse)
	{
		inst->IncrementChannel(&inst->inverter[inst->curInvInst].activeRcvChannel);
		inst->_rf24Radio->setChannel(inst->inverter[inst->curInvInst].activeRcvChannel);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::IncrementChannel(uint8_t *pChannel)
{
	int curChannel;
	int cntChannels = HM_CNT_CHANNELS;
	for (curChannel = 0; curChannel < cntChannels; curChannel++)
	{
		if (*pChannel == usedChannels[curChannel])
			break;
	}
	if (curChannel >= cntChannels - 1)
		*pChannel = usedChannels[0];
	else
		*pChannel = usedChannels[curChannel + 1];
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HM_Radio::StateMachine(void)
{
	bool ret = false;
	switch (eState)
	{
	case HM_State_Idle:
		if ((cntInvInst > 0) && (cntInvInst <= HM_MAXINVERTERINSTANCES))
			eState = HM_State_SetInvInstance;
		break;

	case HM_State_SetInvInstance:
		if (curInvInst >= cntInvInst)
			curInvInst = 0;
		inverter[curInvInst].retries = 0;
		eState = HM_State_Send;
		break;

	case HM_State_Send:
		PreparePacket();
		SendPacket();
		inverter[curInvInst].rcvPeriod.set(1000);
		inverter[curInvInst].payloadFrameFlag = 0x0000;
		inverter[curInvInst].payloadLastFrame = 0x00;

		eState = HM_State_CheckResponse;
		ret = true;
		break;

	case HM_State_CheckResponse:
		if ((!inverter[curInvInst].rcvPeriod.occured()) && (!inverter[curInvInst].payloadLastFrame))
		{
			ProcessPacket();
			ret = true;
		}
		else
		{
			if (dumpRFData)
				HM_PRINTF(HM_PSTR("RCV period end %04X %u\n"), inverter[curInvInst].payloadFrameFlag, inverter[curInvInst].rcvRetryToChannelSwitch);
			if ((inverter[curInvInst].payloadFrameFlag == 0) && (++inverter[curInvInst].rcvRetryToChannelSwitch >= 3))
			{
				inverter[curInvInst].rcvRetryToChannelSwitch = 0;
				IncrementChannel(&inverter[curInvInst].activeSndChannel);
				if (dumpRFData)
					HM_PRINTF(HM_PSTR("Channel switch to %u\n"), inverter[curInvInst].activeSndChannel);
			}
#if defined(ENV_KER)
			if ((inverter[curInvInst].payloadFrameFlag & 0x000F) == 0x000F)
#endif
#if defined(ENV_STR)
				if ((inverter[curInvInst].payloadFrameFlag & 0x0007) == 0x0007)
#endif
				{ // All frames received
					inverter[curInvInst].lastPayloadRcvTime = epochTime;
					eState = HM_State_Calculate;
				}
				else
				{
					if (++inverter[curInvInst].retries < 3)
					{
						eState = HM_State_Send;
					}
					else
					{
						eState = HM_State_CycleEnd;
					}
				}
		}
		break;

	case HM_State_Calculate:
		inverter[curInvInst].getAssignment();
		for (uint8_t i = 0; i < inverter[curInvInst].listLen; i++)
		{
			inverter[curInvInst].addValue(i, (uint8_t *)&inverter[curInvInst].payload[0][0]); // cmd value decides which parser is used to decode payload
			yield();
		}

		CalculateValues();
		eState = HM_State_CycleEnd;
		break;

	case HM_State_CycleEnd:
		inverter[curInvInst].setQueuedCmdFinished();
		curInvInst++;
		sendIntervall = HM_GETTICKCOUNT + 500;
		eState = HM_State_SendDelay;
		break;

	case HM_State_SendDelay:
		if (HM_GETTICKCOUNT > sendIntervall)
			eState = HM_State_SetInvInstance;
		break;

	default:
		// Illegal state
		eState = HM_State_Idle;
		break;
	}

	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::PreparePacket(void)
{
	uint8_t cmd = inverter[curInvInst].getQueuedCmd();

	sendBytes = PrepareTimePacket(inverter[curInvInst].rfAddress.u64, HM_DTU_RADIO_ID >> 8, cmd, 0x00, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::SendPacket(void)
{

	if (dumpRFData)
	{
		// Debugging output
		HM_PRINTF(HM_PSTR("=>|%02u|"), inverter[curInvInst].activeSndChannel);
		dumpMillis(HM_GETTICKCOUNT);
		dumpMillis(HM_GETTICKCOUNT - lastDump);
		lastDump = HM_GETTICKCOUNT;

		HM_PRINTF(HM_PSTR("00|%02u|%08lX01|"), inverter[curInvInst].activeSndChannel, inverter[curInvInst].rfAddress.u64);
		HM_PRINTF(HM_PSTR("    |  | |%02x|"), sendBuffer[0]);
		dumpData(&sendBuffer[1], 4);
		dumpData(&sendBuffer[5], 4);
		dumpData(&sendBuffer[9], sendBytes - 9);
		HM_PRINTF(HM_PSTR("\r\n"));

		// Overwrite send dump output
		sendLineLF = false;
	}

	HM_DISABLE_EINT;
	_rf24Radio->stopListening();
	_rf24Radio->setChannel(inverter[curInvInst].activeSndChannel);
	_rf24Radio->openWritingPipe(((inverter[curInvInst].rfAddress.u64 & 0XFFFFFFFF) << 8) | 0x01);
	_rf24Radio->setCRCLength(RF24_CRC_16);
	_rf24Radio->enableDynamicPayloads();
	_rf24Radio->setAutoAck(true);
	_rf24Radio->setRetries(3, 15);

	_rf24Radio->write(sendBuffer, sendBytes);

	// Try to avoid zero payload acks => has no effect Reason for zero payload acks is unknown
	_rf24Radio->openWritingPipe(HM_DUMMY_RADIO_ID);

	_rf24Radio->setAutoAck(false);
	_rf24Radio->setRetries(0, 0);
	_rf24Radio->disableDynamicPayloads();
	_rf24Radio->setCRCLength(RF24_CRC_DISABLED);
	_rf24Radio->setChannel(inverter[curInvInst].activeRcvChannel);
	_rf24Radio->startListening();
	HM_ENABLE_EINT;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::ProcessPacket(void)
{
	while (PacketAvailable())
	{
		// One or more records present
		HM_Packet_t p = GetPacket();

		// Shift payload data due to 9-bit packet control field
		for (int16_t j = sizeof(p.packet) - 1; j >= 0; j--)
		{
			if (j > 0)
				p.packet[j] = (byte)(p.packet[j] >> 7) | (byte)(p.packet[j - 1] << 1);
			else
				p.packet[j] = (byte)(p.packet[j] >> 7);
		}

		serialHdr.timestamp = p.timestamp;
		serialHdr.packetsLost = p.packetsLost;

		// Check CRC
		uint16_t crc = 0xFFFF;
		crc = HM_crc16((uint8_t *)&serialHdr.address, sizeof(serialHdr.address), crc, 0, BYTES_TO_BITS(sizeof(serialHdr.address)));
		// Payload length
		uint8_t payloadLen = ((p.packet[0] & 0x01) << 5) | (p.packet[1] >> 3);

		// Add one byte and one bit for 9-bit packet control field
		crc = HM_crc16((uint8_t *)&p.packet[0], sizeof(p.packet), crc, 7, BYTES_TO_BITS(payloadLen + 1) + 1);

		if (checkCRC)
		{
			// If CRC is invalid only show lost packets
			if (((crc >> 8) != p.packet[payloadLen + 2]) || ((crc & 0xFF) != p.packet[payloadLen + 3]))
			{
				if (p.packetsLost > 0)
				{
					HM_PRINTF(HM_PSTR(" Lost: %u"), p.packetsLost);
				}
				continue;
			}

			// Dump a decoded packet only once
			if (lastPacketCRC == crc)
			{
				continue;
			}

			lastPacketCRC = crc;
		}

		// Valid packet received. Set timeout
		inverter[curInvInst].rcvRetryToChannelSwitch = 0;

		// Don't dump mysterious ack packages
		if (payloadLen == 0)
		{
			continue;
		}

		if (sendLineLF)
			HM_PRINTF(HM_PSTR("\n"));

		sendLineLF = false;
		// lastPacketRcv = serialHdr.timestamp;

		lastDump = serialHdr.timestamp;

		// Channel
		if (dumpRFData)
		{
			HM_PRINTF(HM_PSTR("<=|%02u|"), p.channel);

			// Write timestamp, packets lost, address and payload length
			dumpMillis(serialHdr.timestamp);
			dumpMillis(serialHdr.timestamp - lastDump);

			dumpData((uint8_t *)&serialHdr.packetsLost, sizeof(serialHdr.packetsLost));
			printf_P(PSTR("%02u|"), p.channel);
			dumpData((uint8_t *)&serialHdr.address, sizeof(serialHdr.address));

			// Trailing bit?!?
			dumpData(&p.packet[0], 2);

			// Payload length from PCF
			dumpData(&payloadLen, sizeof(payloadLen));

			// Packet control field - PID Packet identification
			uint8_t val = (p.packet[1] >> 1) & 0x03;
			HM_PRINTF(HM_PSTR("%u|"), val);
		}

		if (payloadLen > 9)
		{
			if (dumpRFData)
			{

				dumpData(&p.packet[2], 1);
				dumpData(&p.packet[3], 4);
				dumpData(&p.packet[7], 4);
			}
			uint16_t remain = payloadLen - 2 - 1 - 4 - 4 + 4;

			if (remain < 32)
			{
				if (dumpRFData)
				{
					dumpData(&p.packet[11], remain);
					HM_PRINTF(HM_PSTR("%04X|"), crc);

					if (((crc >> 8) != p.packet[payloadLen + 2]) || ((crc & 0xFF) != p.packet[payloadLen + 3]))
						HM_PRINTF(HM_PSTR("0"));
					else
						HM_PRINTF(HM_PSTR("1"));
				}
				// From here on this is a valid packet to process
				if (memcmp(&p.packet[3], &p.packet[7], 4) == 0)
				{ // Check for correct inverter address
					if (memcmp(&p.packet[3], &inverter[curInvInst].rfAddress.b[0], 4) == 0)
					{
						if (p.packet[2] == (HM_PACKETTYPE_INFO | HM_PACKETTYPE_RESPONSE))
						{ // Response from get information command
							uint8_t pid = p.packet[11] & 0x7F;
							if ((pid > 0) && (pid < 5))
							{
								if (p.packet[11] & 0x80)
									inverter[curInvInst].payloadLastFrame = 0x01;
								memcpy(&inverter[curInvInst].payload[pid - 1], &p.packet[12], HM_MAX_RF_NET_PAYLOAD_SIZE);
								inverter[curInvInst].payloadFrameFlag |= 0x01 << (pid - 1);
								if (dumpRFData)
									HM_PRINTF(HM_PSTR("| ok %u %u %u"), curInvInst, pid, epochTime);
							}
							else
							{
								if (dumpRFData)
									HM_PRINTF(HM_PSTR("|*PID_MISM"));
							}
						}
						else if (p.packet[2] == (HM_PACKETTYPE_DEVCONTROL | HM_PACKETTYPE_RESPONSE))
						{ // Response from device control command
						  // TBD
						}
						else
						{
							if (dumpRFData)
								HM_PRINTF(HM_PSTR("|*PID_UNKN"));
						}
					}
					else
					{
						HM_PRINTF(HM_PSTR("|*ADR_MISM"));
						dumpData(&inverter[curInvInst].rfAddress.b[0], sizeof(inverter[curInvInst].rfAddress.b));
					}
				}
				else
				{
					HM_PRINTF(HM_PSTR("|*ADRFLD_MISM"));
				}
			}
			else
			{
				HM_PRINTF(HM_PSTR("Ill remain %u\n"), remain);
			}
		}
		else
		{
			dumpData(&p.packet[2], payloadLen + 2);
			HM_PRINTF(HM_PSTR("%04X|"), crc);
		}

		if (dumpRFData)
			HM_PRINTF(HM_PSTR("\n"));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HM_Radio::CalculateValues(void)
{
	inverter[curInvInst].doCalculations();

	if (dumpRFData)
	{
		HM_PRINTF(HM_PSTR("Inverter: %u:\n"), curInvInst);

		for (uint8_t i = 0; i < inverter[curInvInst].listLen; i++)
		{
			if (0.0f != inverter[curInvInst].getValue(i))
			{
				HM_PRINTF(HM_PSTR("%s = "), inverter[curInvInst].getFieldName(i));
				HM_PRINTF(HM_PSTR("%.3f %s\n"), inverter[curInvInst].getValue(i), inverter[curInvInst].getUnit(i));
			}
			yield();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
