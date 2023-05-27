#ifndef __ili9341_h__
#define __ili9341_h__

#include <circle/gpiopin.h>
#include <circle/spimaster.h>
#include <circle/screen.h>
#include <circle/spimaster.h>
#include <circle/gpiopin.h>
#include <circle/chargenerator.h>
#include <circle/timer.h>
#include <circle/util.h>
#include <circle/types.h>
#include <circle/logger.h>


/*
Adafruit PiTFT HAT https://www.adafruit.com/product/2455
STMPE610 12 bit touch controller
240x320 LCD 2.4"
ILI9341 display controller
HAT eeprom in I2C (27, 28)

Touch controller specifics
RT_INT          GPIO24      18
RT_CS_3V        GPIO7       26   #SPI_CE1

Display controller specifics
Backlight       On GPIO2 of the  Touch Controller
TFT_DC_3V (WR)  GPIO25      22
TFT_CS_3V       GPIO8       24   #SPI_CE0 

SPI shared across touch and display
SCLK_3V         SPI_SCLK    23
MISO            SPI_MISO    21
MOSI_3V         SPI_MOSI    19
*/

class CILI9341Display	/// Driver for ILI9341-based dot-matrix displays
{
public:
	static const unsigned None = GPIO_PINS;

	typedef u16 TILI9341Color;

	// really ((green) & 0x3F) << 5, but to have a 0-31 range for all colors
	#define ILI9341_COLOR(red, green, blue)	  bswap16 (((red) & 0x1F) << 11 \
						| ((green) & 0x1F) << 6 \
						| ((blue) & 0x1F))
	#define ILI9341_BLACK_COLOR	0
	#define ILI9341_RED_COLOR	0x00F8
	#define ILI9341_GREEN_COLOR	0xE007
	#define ILI9341_BLUE_COLOR	0x1F00
	#define ILI9341_WHITE_COLOR	0xFFFF


// Display width and height definition
#define ILI9341_WIDTH     320
#define ILI9341_HEIGHT    240

// Display commands list
#define ILI9341_NOP                        0x00
#define ILI9341_SOFTWARE_RESET             0x01
#define ILI9341_READ_IDENTIFICATION        0x04
#define ILI9341_READ_STATUS                0x09
#define ILI9341_READ_POWER_MODE            0x0A
#define ILI9341_READ_MADCTL                0x0B
#define ILI9341_READ_PIXEL_FORMAT          0x0C
#define ILI9341_READ_IMAGE_FORMAT          0x0D
#define ILI9341_READ_SIGNAL_MODE           0x0E
#define ILI9341_READ_SELF_DIAGNOSTIC       0x0F
#define ILI9341_SLEEP_IN                   0x10
#define ILI9341_SLEEP_OUT                  0x11
#define ILI9341_PARTIAL_MODE_ON            0x12
#define ILI9341_NORMAL_DISPLAY_MODE_ON     0x13
#define ILI9341_INVERSION_OFF              0x20
#define ILI9341_INVERSION_ON               0x21
#define ILI9341_GAMMA_SET                  0x26
#define ILI9341_DISPLAY_OFF                0x28
#define ILI9341_DISPLAY_ON                 0x29
#define ILI9341_COLUMN_ADDRESS_SET         0x2A
#define ILI9341_PAGE_ADDRESS_SET           0x2B
#define ILI9341_MEMORY_WRITE               0x2C
#define ILI9341_COLOR_SET                  0x2D
#define ILI9341_MEMORY_READ                0x2E
#define ILI9341_PARTIAL_AREA               0x30
#define ILI9341_VERTICAL_SCROLLING_DEF     0x33
#define ILI9341_TEARING_LINE_OFF           0x34
#define ILI9341_TEARING_LINE_ON            0x35
#define ILI9341_MEMORY_ACCESS_CONTROL      0x36
#define ILI9341_VERTICAL_SCROLLING         0x37
#define ILI9341_IDLE_MODE_OFF              0x38
#define ILI9341_IDLE_MODE_ON               0x39
#define ILI9341_PIXEL_FORMAT_SET           0x3A
#define ILI9341_WRITE_MEMORY_CONTINUE      0x3C
#define ILI9341_READ_MEMORY_CONTINUE       0x3E
#define ILI9341_SET_TEAR_SCANLINE          0x44
#define ILI9341_GET_SCANLINE               0x45
#define ILI9341_WRITE_BRIGHTNESS           0x51
#define ILI9341_READ_BRIGHTNESS            0x52
#define ILI9341_WRITE_CTRL_DISPLAY         0x53
#define ILI9341_READ_CTRL_DISPLAY          0x54
#define ILI9341_WRITE_CA_BRIGHTNESS        0x55
#define ILI9341_READ_CA_BRIGHTNESS         0x56
#define ILI9341_WRITE_CA_MIN_BRIGHTNESS    0x5E
#define ILI9341_READ_CA_MIN_BRIGHTNESS     0x5F
#define ILI9341_READ_ID1                   0xDA
#define ILI9341_READ_ID2                   0xDB
#define ILI9341_READ_ID3                   0xDC
#define ILI9341_RGB_INTERFACE_CONTROL      0xB0
#define ILI9341_FRAME_RATE_CONTROL_1       0xB1
#define ILI9341_FRAME_RATE_CONTROL_2       0xB2
#define ILI9341_FRAME_RATE_CONTROL_3       0xB3
#define ILI9341_DISPLAY_INVERSION_CONTROL  0xB4
#define ILI9341_BLANKING_PORCH_CONTROL     0xB5
#define ILI9341_DISPLAY_FUNCTION_CONTROL   0xB6
#define ILI9341_ENTRY_MODE_SET             0xB7
#define ILI9341_BACKLIGHT_CONTROL_1        0xB8
#define ILI9341_BACKLIGHT_CONTROL_2        0xB9
#define ILI9341_BACKLIGHT_CONTROL_3        0xBA
#define ILI9341_BACKLIGHT_CONTROL_4        0xBB
#define ILI9341_BACKLIGHT_CONTROL_5        0xBC
#define ILI9341_BACKLIGHT_CONTROL_7        0xBE
#define ILI9341_BACKLIGHT_CONTROL_8        0xBF
#define ILI9341_POWER_CONTROL_1            0xC0
#define ILI9341_POWER_CONTROL_2            0xC1
#define ILI9341_VCOM_CONTROL_1             0xC5
#define ILI9341_VCOM_CONTROL_2             0xC7
#define ILI9341_POWERA                     0xCB
#define ILI9341_POWERB                     0xCF
#define ILI9341_NV_MEMORY_WRITE            0xD0
#define ILI9341_NV_PROTECTION_KEY          0xD1
#define ILI9341_NV_STATUS_READ             0xD2
#define ILI9341_READ_ID4                   0xD3
#define ILI9341_POSITIVE_GAMMA_CORRECTION  0xE0
#define ILI9341_NEGATIVE_GAMMA_CORRECTION  0xE1
#define ILI9341_DIGITAL_GAMMA_CONTROL_1    0xE2
#define ILI9341_DIGITAL_GAMMA_CONTROL_2    0xE3
#define ILI9341_DTCA                       0xE8
#define ILI9341_DTCB                       0xEA
#define ILI9341_POWER_SEQ                  0xED
#define ILI9341_3GAMMA_EN                  0xF2
#define ILI9341_INTERFACE_CONTROL          0xF6
#define ILI9341_PUMP_RATIO_CONTROL         0xF7

//
// ILI9341_MEMORY_ACCESS_CONTROL registers
//
#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04
#define ILI9341_MADCTL_RGB 0x00

#define DISPLAY_ROTATION_270   (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)
#define DISPLAY_ROTATION_90    (ILI9341_MADCTL_MY | ILI9341_MADCTL_BGR)
#define DISPLAY_ROTATION_0     (ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)
#define DISPLAY_ROTATION_180   (ILI9341_MADCTL_MX | ILI9341_MADCTL_MY  \
                              | ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR)




public:
	/// \param pSPIMaster Pointer to SPI master object
	/// \param nDCPin GPIO pin number for DC pin
	/// \param nBackLightPin GPIO pin number for backlight pin (optional)
	/// \param nWidth Display width in number of pixels (default 240)
	/// \param nHeight Display height in number of pixels (default 240)
	/// \param CPOL SPI clock polarity (0 or 1, default 0)
	/// \param CPHA SPI clock phase (0 or 1, default 0)
	/// \param nClockSpeed SPI clock frequency in Hz
	/// \param nChipSelect SPI chip select (if connected, otherwise don't care)
	/// \note GPIO pin numbers are SoC number, not header positions.
	/// \note If SPI chip select is not connected, CPOL should probably be 1.
	CILI9341Display (CSPIMaster *pSPIMaster,
			unsigned nDCPin, 
			unsigned nWidth = 320, unsigned nHeight = 240,
			unsigned CPOL = 0, unsigned CPHA = 0, unsigned nClockSpeed = 15000000,
			unsigned nChipSelect = 0);

	/// \return Display width in number of pixels
	unsigned GetWidth (void) const		{ return m_nWidth; }
	/// \return Display height in number of pixels
	unsigned GetHeight (void) const		{ return m_nHeight; }

	/// \return Operation successful?
	boolean Initialize(void);
	void Off(void);
	void On(void);
	void fillRect(int xs, int ys, int xe, int ye, u16 color);
	void SetPixel(unsigned nPosX, unsigned nPosY, TILI9341Color Color);
	void DrawText(unsigned nPosX, unsigned nPosY, const char *pString,
			TILI9341Color Color, TILI9341Color BgColor = ILI9341_BLACK_COLOR);
	void Clear(TILI9341Color Color = ILI9341_BLACK_COLOR);


private:
	CSPIMaster *m_pSPIMaster;
	CGPIOPin m_DCPin;
	unsigned m_nWidth;
	unsigned m_nHeight;
	unsigned m_CPOL;
	unsigned m_CPHA;
	unsigned m_nClockSpeed;
	unsigned m_nChipSelect;

	CCharGenerator m_CharGen;
	CTimer *m_pTimer;

	void SendByte (u8 uchByte, boolean bIsData);
	void SendData (const void *pData, size_t nLength);
	void Command (u8 uchByte)	{ SendByte (uchByte, FALSE); }
	void Data (u8 uchByte)		{ SendByte (uchByte, TRUE); }
	void setWindow(int xs, int ys, int xe, int ye);
};

#endif
