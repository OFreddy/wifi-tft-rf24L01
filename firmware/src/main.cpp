
#include "Arduino.h"

// Applicatiom
#include "app.h"

#if defined(ENV_KER) || defined(ENV_STR)
// RF24 Radio
#include <SPI.h>
#include <CircularBuffer.h>
#include <RF24.h>
#include <RF24_config.h>
#include "peripherals.h"
#include "hm_crc.h"
#include "hm_radio.h"
#endif

app myApp;

#if defined(ENV_KER) || defined(ENV_STR)
// RF24 Radio
static RF24 radio(RF1_CE_PIN, RF1_CS_PIN);
// Hoymiles packets instance
HM_Radio hmRadio(&radio);

#if defined(ENV_KER)
#define INV1_SERIAL ((uint64_t)0x116181672101ULL) // 116181672101 = Norbert HM-1500
//#define INV1_SERIAL ((uint64_t)0x116181670977ULL) // 116181670977 = Treckerschuppen HM-1500
#endif
#if defined(ENV_STR)
#define INV1_SERIAL ((uint64_t)0x114173104619ULL) // 114173104619 = Gartenhütte FR 1
#define INV2_SERIAL ((uint64_t)0x114173104439ULL) // 114173104439 = Gartenhütte FR 2

//#define INV1_SERIAL ((uint64_t)0x114173104924ULL) // 114173104619 = Gartenhütte STR 1
//#define INV2_SERIAL ((uint64_t)0x114173105307ULL) // 114173104439 = Gartenhütte STR 2
#endif

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

static void DumpMenu()
{
	Serial.println("Console menu");
	Serial.println("============\n");
	Serial.println("c = Print RF24 module config");
	Serial.println("r = Reset module");
	Serial.println("t = Toggle RF24 traffic debug output");
}

static void activateConf(void)
{
	// Attach interrupt handler to NRF IRQ output. Overwrites any earlier handler.
	attachInterrupt(digitalPinToInterrupt(RF1_IRQ_PIN), handleNrf1Irq, FALLING); // NRF24 Irq pin is active low.

	DumpConfig();
}
#endif

//-----------------------------------------------------------------------------
// ICACHE_RAM_ATTR void handleIntr(void) {

// }

//-----------------------------------------------------------------------------
void setup()
{
	Serial.begin(SERIAL_BAUDRATE);
	Serial.printf_P(PSTR("\n\n------------------------------------------\nWifiDisplay starting...\n"));

	myApp.setup();

#if defined(ENV_KER) || defined(ENV_STR)
	// Configure nRF IRQ input
	pinMode(RF1_IRQ_PIN, INPUT);

	// Add inverter instances - check HM_MAXINVERTERINSTANCES in hm_config.h
	bool res = hmRadio.AddInverterInstance(INV1_SERIAL);
#if defined(ENV_STR)
	res = hmRadio.AddInverterInstance(INV2_SERIAL);
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
	static bool bDumpRFData = false;

	myApp.loop(bHMTransmitting);

#if defined(ENV_KER) || defined(ENV_STR)
	// Cyclic communication processing
	hmRadio.SetUnixTimeStamp(myApp.getUnixTimeStamp());
	bHMTransmitting = hmRadio.Cyclic();

	// Config info
	if (Serial.available())
	{
		uint8_t cmd = Serial.read();

		if (cmd == '?')
		{
			DumpMenu();
		}
		else if (cmd == 'c')
		{
			DumpConfig();
		}
		else if (cmd == 'r')
		{
			DBG_PRINTF(DBG_PSTR("Resetting..."));
			delay(200);
			ESP.reset();
		}
		else if (cmd == 't')
		{
			bDumpRFData = !bDumpRFData;
			hmRadio.DumpRFData(bDumpRFData);
		}
	}
#endif

	//    digitalWrite(LED_BUILTIN, LOW);
	//   delay(500);
	// digitalWrite(LED_BUILTIN, HIGH);
	// delay(500);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// End of file
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
