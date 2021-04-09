/***************************************************
  This is our GFX example for the Adafruit ILI9341 Breakout and Shield
  ----> http://www.adafruit.com/products/1651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include <XPT2046_Touchscreen.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <OneWire.h>

static const int PIN_LED = 23;
static const int PIN_ONE_WIRE = 22;

Adafruit_ILI9341 tft = Adafruit_ILI9341(13, 27, 26, 25, 14, 33);
OneWire ibutton (PIN_ONE_WIRE); 	// I button connected on PIN 2.
byte addr[8] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77}; 			//array to store the Ibutton ID.

XPT2046_Touchscreen ts(32);
SPIClass SPI2(HSPI);

void drawCurrentID()
{
	tft.setCursor(0, 20);
	tft.print("iButtonID:\n");

	for (byte x = 0; x < 8; x++)
	{
		tft.printf("%02x",addr[x]); //print the buffer content in LSB. For MSB: for (int x = 8; x>0; x--)
		Serial.printf("%02x",addr[x]);
	}

	tft.print("\n");
	Serial.print("\n");
}

void drawButtons()
{
	tft.fillRect(0, 120, 160, 120, ILI9341_GREEN);
	tft.fillRect(160, 120, 160, 120, ILI9341_RED);

	tft.setCursor(50, 190);
	tft.print("READ");

	tft.setCursor(210, 190);
	tft.print("WRITE");
}

void setup()
{
	Serial.begin(115200);

	pinMode(PIN_LED, OUTPUT);
	digitalWrite(PIN_LED, LOW);

	tft.begin();
	tft.setRotation(1);
	tft.setFont(&FreeMonoBold12pt7b);
	tft.fillScreen(0x0000);

	SPI2.begin(18, 35, 17);
	SPI2.setFrequency(1000);
	ts.begin(SPI2);

	addr[7] = ibutton.crc8(addr, 7);

	drawCurrentID();
	drawButtons();

}

int writeByte(byte data) {
	int data_bit;
	for (data_bit = 0; data_bit < 8; data_bit++) {
		if (data & 1)
		{
			digitalWrite(PIN_ONE_WIRE, LOW);
			pinMode(PIN_ONE_WIRE, OUTPUT);
			delayMicroseconds(60);

			pinMode(PIN_ONE_WIRE, INPUT);
			digitalWrite(PIN_ONE_WIRE, HIGH);
			delay(10);
		}
		else
		{
			digitalWrite(PIN_ONE_WIRE, LOW);
			pinMode(PIN_ONE_WIRE, OUTPUT);
			pinMode(PIN_ONE_WIRE, INPUT);
			digitalWrite(PIN_ONE_WIRE, HIGH);
			delay(10);
		}
		data = data >> 1;
	}
	return 0;
}

void reading()
{
	tft.fillScreen(0x0000);

	tft.setCursor(0, 20);
	tft.print("Waiting for read...\n");

	while(!ibutton.search(addr))
	{
		ibutton.reset_search();
	}

	tft.print("\nDone.");
	delay(1000);
}

void programming()
{
	byte oldAddress[8];

	digitalWrite(PIN_LED, HIGH);

	tft.fillScreen(0x0000);

	if(addr[7] == ibutton.crc8(addr, 7))
	{
		drawCurrentID();
		tft.print("Waiting for new key...");

		while(!ibutton.search(oldAddress) && !ts.touched())
		{
			delay(1);
		}

		tft.print("Programming new key:\n");


		ibutton.skip();
		ibutton.reset();
		ibutton.write(0x33);

		Serial.print("  ID before write:");
		for (byte x=0; x<8; x++)
		{
		  Serial.print(' ');
		  Serial.print(ibutton.read(), HEX);
		}

		// send reset
		ibutton.skip();
		ibutton.reset();

		// send 0xD1
		ibutton.write(0xD1);

		// send logical 0
		digitalWrite(PIN_ONE_WIRE, LOW);

		pinMode(PIN_ONE_WIRE, OUTPUT);
		delayMicroseconds(60);

		pinMode(PIN_ONE_WIRE, INPUT);
		digitalWrite(PIN_ONE_WIRE, HIGH);
		delay(10);

		Serial.print('\n');
		Serial.print("  Writing iButton ID:\n    ");

		ibutton.skip();
		ibutton.reset();
		ibutton.write(0xD5);

		for (byte x = 0; x < 8; x++)
		{
			writeByte(addr[x]);
			tft.print('*');
		}

		ibutton.reset();

		// send 0xD1
		ibutton.write(0xD1);

		//send logical 1
		digitalWrite(PIN_ONE_WIRE, LOW);
		pinMode(PIN_ONE_WIRE, OUTPUT);
		delayMicroseconds(10);

		pinMode(PIN_ONE_WIRE, INPUT);
		digitalWrite(PIN_ONE_WIRE, HIGH);
		delay(10);

		tft.print("\nDone.");

		delay(1000);
		tft.fillScreen(0x0000);
	}
	else
	{
		tft.print("\nBad ID!!!\n Read the source again.");

		delay(5000);
		tft.fillScreen(0x0000);
	}

	digitalWrite(PIN_LED, LOW);
}

static const int b_prog_x_l = 1600;
static const int b_prog_x_h = 2800;

static const int b_prog_y_l = 2500;
static const int b_prog_y_h = 3200;

static const int b_read_x_l = 3000;
static const int b_read_x_h = 4096;

static const int b_read_y_l = 2500;
static const int b_read_y_h = 3200;

void loop(void)
{
	if(ts.touched())
	{

		TS_Point p = ts.getPoint();

		if ((p.x > b_prog_x_l) && (p.x < b_prog_x_h) && (p.y > b_prog_y_l)
				&& (p.y < b_prog_y_h))
		{
			programming();

			tft.fillRect(0, 0, 320, 50, 0x0000);
			drawCurrentID();
			drawButtons();
		} else if ((p.x > b_read_x_l) && (p.x < b_read_x_h)
				&& (p.y > b_read_y_l) && (p.y < b_read_y_h))
		{
			reading();

			tft.fillRect(0, 0, 320, 50, 0x0000);
			drawCurrentID();
			drawButtons();
		}

	}
}
