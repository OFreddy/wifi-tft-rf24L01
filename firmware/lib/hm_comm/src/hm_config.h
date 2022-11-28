/*
 Copyright (C)
	2022            OFreddy

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 3 as published by the Free Software Foundation.
*/

#pragma once

#define HM_DTU_RADIO_ID ((uint64_t)0x1234567801ULL) // DTU address of the simulated DTU
#define HM_DUMMY_RADIO_ID ((uint64_t)0xDEADBEEF01ULL) // Dummy radio id to prevent rf24 from sending acks

#define HM_RF_PA_LEVEL RF24_PA_HIGH    // Radio PA level for transmitting packages
#define HM_RF_DATARATE (RF24_250KBPS) // Datarate
#define HM_RF_ADDRESSWIDTH (5)        // Length in bytes of hoymiles address field

#define HM_MAXINVERTERINSTANCES (4) // Maximum number of supported converter instances
#define HM_MAXPAYLOADSIZE (32)      // Maximum size for payload buffer
#define HM_PACKET_BUFFER_SIZE (22)  // Maximum number of packets that can be buffered between reception and processing by application

#define HM_MAX_PAYLOAD_ENTRIES (15) // Maximum number of payload fragments 
#define HM_MAX_RF_NET_PAYLOAD_SIZE (16) // Maximum number of net payload bytes in one RF fragment


