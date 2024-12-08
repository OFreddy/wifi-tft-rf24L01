
#include "Arduino.h"

// Applicatiom
#include "app.h"

app myApp;

#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
// RF24 Radio
#include <SPI.h>
#include <CircularBuffer.h>
#include <RF24.h>
#include <RF24_config.h>
#include "peripherals.h"
#include "hm_crc.h"
#include "hm_radio.h"

static RF24 radio(RF1_CE_PIN, RF1_CS_PIN);
// Hoymiles packets instance
HM_Radio hmRadio(&radio);

static IRAM_ATTR void handleNrf1Irq()
{
	hmRadio.RadioIrqCallback();
}

static void DumpConfig()
{
	Serial.println(F("\nRadio:"));
	radio.printDetails();

	Serial.println("");
}

static void activateConf(void)
{
	// Attach interrupt handler to NRF IRQ output. Overwrites any earlier handler.
	attachInterrupt(digitalPinToInterrupt(RF1_IRQ_PIN), handleNrf1Irq, FALLING); // NRF24 Irq pin is active low.

	DumpConfig();
}
#endif

//-----------------------------------------------------------------------------

static void DumpMenu()
{
	Serial.println("Console menu");
	Serial.println("============\n");
#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
	Serial.println("c = Print RF24 module config");
#endif
	Serial.println("r = Reset module");
#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
	Serial.println("t = Toggle RF24 traffic debug output");
#endif	
}

//-----------------------------------------------------------------------------
void setup()
{
	Serial.begin(SERIAL_BAUDRATE);
	Serial.println(F("\n\n------------------------------------------\nWifiDisplay starting...\n"));

	myApp.setup();

#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
	// Configure nRF IRQ input
	pinMode(RF1_IRQ_PIN, INPUT);

	// Add inverter instances - check HM_MAXINVERTERINSTANCES in hm_config.h
	bool res = hmRadio.AddInverterInstance(INV1_SERIAL);
#if (NUMBER_OF_INVERTERS > 1)
	res = hmRadio.AddInverterInstance(INV2_SERIAL);
#if (NUMBER_OF_INVERTERS > 2)
	res = hmRadio.AddInverterInstance(INV3_SERIAL);
#if (NUMBER_OF_INVERTERS > 3)
	res = hmRadio.AddInverterInstance(INV4_SERIAL);
#endif
#endif
#endif
	if (!res)
	{
		Serial.println(F("Failed to add inverter instances!"));
		while (1)
			;
	}
	if (!hmRadio.Begin())
	{
		Serial.println(F("Failed to start up radio!\n"));
		while (1)
			;
	}
	activateConf();
#endif
}

//-----------------------------------------------------------------------------
void loop()
{
	static bool bHMTransmitting = false;
#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
	static bool bDumpRFData = false;
#endif

	myApp.loop(bHMTransmitting);

#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
	// Cyclic communication processing
	hmRadio.SetUnixTimeStamp(myApp.getUnixTimeStamp());
	bHMTransmitting = hmRadio.Cyclic();
#endif

	// Config info
	if (Serial.available())
	{
		uint8_t cmd = Serial.read();

		if (cmd == '?')
		{
			DumpMenu();
		}
#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
		else if (cmd == 'c')
		{
			DumpConfig();
		}
#endif
		else if (cmd == 'r')
		{
			DBG_PRINTF(DBG_PSTR("Resetting..."));
			delay(200);
			ESP.reset();
		}
#if defined(NUMBER_OF_INVERTERS) && (NUMBER_OF_INVERTERS > 0)
		else if (cmd == 't')
		{
			bDumpRFData = !bDumpRFData;
			hmRadio.DumpRFData(bDumpRFData);
		}
#endif
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
