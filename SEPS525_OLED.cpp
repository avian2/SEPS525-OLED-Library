#include <SEPS525_OLED.h>
#include <SPI.h>

static const int pinVddEnable = 7;

static const int pinRS = 5;
static const int pinSS = 10;
static const int pinReset = 6;

static void seps525_reg(int idx, int value)
{
  digitalWrite(pinSS, LOW);
  digitalWrite(pinRS, LOW);
  SPI.transfer(idx);
  digitalWrite(pinRS, HIGH);
  digitalWrite(pinSS, HIGH);

  digitalWrite(pinSS, LOW);
  SPI.transfer(value);
  digitalWrite(pinSS, HIGH);
}

static inline void seps525_datastart(void)
{
  digitalWrite(pinSS, LOW);
  digitalWrite(pinRS, LOW);
  SPI.transfer(0x22);
  digitalWrite(pinRS, HIGH);
}

static inline void seps525_data(int value)
{
  SPI.transfer((value>>8)& 0xFF);
  SPI.transfer(value & 0xFF);
}

static inline void seps525_dataend(void)
{
  digitalWrite(pinSS, HIGH);
}

static void seps525_set_region(int x, int y, int xs, int ys)
{
  // draw region
  seps525_reg(0x17,x);
  seps525_reg(0x18,x+xs-1);
  seps525_reg(0x19,y);
  seps525_reg(0x1a,y+ys-1);
  
  // start position
  seps525_reg(0x20,x);
  seps525_reg(0x21,y);
}

static void seps525_setup(void) 
{
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  // pin for switcher enable (off by default)
  digitalWrite(pinVddEnable, HIGH);
  pinMode(pinVddEnable, OUTPUT);
  
  // pins for seps525
  digitalWrite(pinRS, HIGH);
  digitalWrite(pinSS, HIGH);
  digitalWrite(pinReset, HIGH);
  pinMode(pinRS, OUTPUT);
  pinMode(pinSS, OUTPUT);
  pinMode(pinReset, OUTPUT);
}
  
static void seps525_init(void)
{
  seps525_setup();
  
  // NOTE: this procedure is from Densitron 
  // DD-160128FC-2B datasheet
  
  digitalWrite(pinReset, LOW);
  delay(2);
  digitalWrite(pinReset, HIGH);
  delay(1);
  // display off, analog reset
  seps525_reg(0x04, 0x01);
  delay(1);
  // normal mode
  seps525_reg(0x04, 0x00);
  delay(1);
  // display off
  seps525_reg(0x06, 0x00);
  // turn on internal oscillator using external resistor
  seps525_reg(0x02, 0x01);
  // 90 hz frame rate, divider 0
  seps525_reg(0x03, 0x30);
  // duty cycle 127
  seps525_reg(0x28, 0x7f);
  // start on line 0
  seps525_reg(0x29, 0x00);
  // rgb_if
  seps525_reg(0x14, 0x31);
  // memory write mode
  seps525_reg(0x16, 0x66);

  // driving current r g b (uA)
  seps525_reg(0x10, 0x45);
  seps525_reg(0x11, 0x34);
  seps525_reg(0x12, 0x33);

  // precharge time r g b
  seps525_reg(0x08, 0x04);
  seps525_reg(0x09, 0x05);
  seps525_reg(0x0a, 0x05);

  // precharge current r g b (uA)
  seps525_reg(0x0b, 0x9d);
  seps525_reg(0x0c, 0x8c);
  seps525_reg(0x0d, 0x57);

  seps525_reg(0x80, 0x00);

  // mode set
  seps525_reg(0x13, 0x00);
  
  
  seps525_set_region(0, 0, 160, 128);

  seps525_datastart();
  int n;
  for(n = 0; n < 160*128; n++) {
    seps525_data(0xffff);
  }
  seps525_dataend();
  
  digitalWrite(pinVddEnable, LOW);
  delay(100);
  
  seps525_reg(0x06, 0x01);
}

SEPS525_OLED::SEPS525_OLED(void) : Adafruit_GFX(160, 128) 
{
}

void SEPS525_OLED::begin(void)
{
	seps525_init();
}

void SEPS525_OLED::drawPixel(int16_t x, int16_t y, uint16_t color)
{
	seps525_set_region(x, y, 1, 1);
	seps525_datastart();
	seps525_data(color);
	seps525_dataend();
}

void SEPS525_OLED::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	seps525_set_region(x, y, 1, h);
	seps525_datastart();
	int n;
	for(n = 0; n < h; n++) seps525_data(color);
	seps525_dataend();
}

void SEPS525_OLED::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	seps525_set_region(x, y, w, 1);
	seps525_datastart();
	int n;
	for(n = 0; n < w; n++) seps525_data(color);
	seps525_dataend();
}

void SEPS525_OLED::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
	seps525_set_region(x, y, w, h);
	seps525_datastart();
	int n;
	for(n = 0; n < h*w; n++) seps525_data(color);
	seps525_dataend();
}

uint16_t SEPS525_OLED::color565(uint8_t r, uint8_t g, uint8_t b)
{
	return (r << 11) | (g << 5) | b;
}
