#include <display/ili9341.h>
#include <assert.h>
#include <stdint.h>


static const uint8_t ili9341_init_seq[] = {
  // cmd, len, data...,
  // SW reset
  ILI9341_SOFTWARE_RESET, 0,
  // display off
  ILI9341_DISPLAY_OFF, 0,
  // Power control B
  ILI9341_POWERB, 3, 0x00, 0x83, 0x30,
  // Power on sequence control
  ILI9341_POWER_SEQ, 4, 0x64, 0x03, 0x12, 0x81,
  //ILI9341_POWER_SEQ, 4, 0x55, 0x01, 0x23, 0x01,
  // Driver timing control A
  ILI9341_DTCA, 3, 0x85, 0x01, 0x79,
  //ILI9341_DTCA, 3, 0x84, 0x11, 0x7a,
  // Power control A
  ILI9341_POWERA, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,
  // Pump ratio control
  ILI9341_PUMP_RATIO_CONTROL, 1, 0x20,
  // Driver timing control B
  ILI9341_DTCB, 2, 0x00, 0x00,
  // POWER_CONTROL_1
  ILI9341_POWER_CONTROL_1, 1, 0x26,
  // POWER_CONTROL_2
  ILI9341_POWER_CONTROL_2, 1, 0x11,
  // VCOM_CONTROL_1
  ILI9341_VCOM_CONTROL_1, 2, 0x35, 0x3E,
  // VCOM_CONTROL_2
  ILI9341_VCOM_CONTROL_2, 1, 0xBE,
  // MEMORY_ACCESS_CONTROL
  //ILI9341_MEMORY_ACCESS_CONTROL, 1, 0x48, // portlait
  ILI9341_MEMORY_ACCESS_CONTROL, 1, DISPLAY_ROTATION_180, // landscape
  // COLMOD_PIXEL_FORMAT_SET : 16 bit pixel
  ILI9341_PIXEL_FORMAT_SET, 1, 0x55,
  // Frame Rate
  ILI9341_FRAME_RATE_CONTROL_1, 2, 0x00, 0x1B,
  // Gamma Function Disable
  ILI9341_3GAMMA_EN, 1, 0x08,
  // gamma set for curve 01/2/04/08
  ILI9341_GAMMA_SET, 1, 0x01,
  // positive gamma correction
//ILI9341_POSITIVE_GAMMA_CORRECTION, 15, 0x1F,  0x1A,  0x18,  0x0A,  0x0F,  0x06,  0x45,  0x87,  0x32,  0x0A,  0x07,  0x02,  0x07, 0x05,  0x00,
  // negativ gamma correction
//ILI9341_NEGATIVE_GAMMA_CORRECTION, 15, 0x00,  0x25,  0x27,  0x05,  0x10,  0x09,  0x3A,  0x78,  0x4D,  0x05,  0x18,  0x0D,  0x38, 0x3A,  0x1F,
  // Column Address Set
//ILI9341_COLUMN_ADDRESS_SET, 4, 0x00, 0x00, 0x01, 0x3f, // width 320
  // Page Address Set
//ILI9341_PAGE_ADDRESS_SET, 4, 0x00, 0x00, 0x00, 0xef,   // height 240
  // entry mode
  ILI9341_ENTRY_MODE_SET, 1, 0x06,
  // display function control
  ILI9341_DISPLAY_FUNCTION_CONTROL, 4, 0x0A, 0x82, 0x27, 0x00,
  // Interface Control (set WEMODE=0)
  ILI9341_INTERFACE_CONTROL, 3, 0x00, 0x00, 0x00,
  // control display
  //ILI9341_WRITE_CTRL_DISPLAY, 1, 0x0c,
  // diaplay brightness
  //ILI9341_WRITE_BRIGHTNESS, 1, 0xff,
  // sleep out
  ILI9341_SLEEP_OUT, 0,
  // display on
  ILI9341_DISPLAY_ON, 0,
  0 // sentinel
};

CILI9341Display::CILI9341Display (CSPIMaster *pSPIMaster,
				unsigned nDCPin, 
				unsigned nWidth, unsigned nHeight,
				unsigned CPOL, unsigned CPHA, unsigned nClockSpeed,
				unsigned nChipSelect)
:	m_pSPIMaster (pSPIMaster),
	m_DCPin (nDCPin, GPIOModeOutput),
	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_CPOL (CPOL),
	m_CPHA (CPHA),
	m_nClockSpeed (nClockSpeed),
	m_nChipSelect (nChipSelect),
	m_pTimer (CTimer::Get ())
{
	assert (nDCPin != None);
}

boolean CILI9341Display::Initialize (void)
{
	assert (m_pSPIMaster != 0);
	assert (m_pTimer != 0);

	const uint8_t *p;
	for (p = ili9341_init_seq; *p; ) {
		Command(p[0]);
		if ( p[1] > 0)
			SendData(&p[2], p[1]);
		p += 2 + p[1];
		m_pTimer->MsDelay(5);
	}

	return TRUE;
}

void CILI9341Display::SendByte (u8 uchByte, boolean bIsData)
{
	assert (m_pSPIMaster != 0);

	m_DCPin.Write (bIsData ? HIGH : LOW);

	m_pSPIMaster->SetClock (m_nClockSpeed);
	m_pSPIMaster->SetMode (m_CPOL, m_CPHA);

#ifndef NDEBUG
	int nResult =
#endif
		m_pSPIMaster->Write (m_nChipSelect, &uchByte, sizeof uchByte);
	assert (nResult == (int) sizeof uchByte);
}

void CILI9341Display::SendData (const void *pData, size_t nLength)
{
	assert (pData != 0);
	assert (nLength > 0);
	assert (m_pSPIMaster != 0);

	m_DCPin.Write (HIGH);

	m_pSPIMaster->SetClock (m_nClockSpeed);
	m_pSPIMaster->SetMode (m_CPOL, m_CPHA);

#ifndef NDEBUG
	int nResult =
#endif
		m_pSPIMaster->Write (m_nChipSelect, pData, nLength);
	assert (nResult == (int) nLength);
}

void CILI9341Display::setWindow(int xs, int ys, int xe, int ye)
{
    u8 buf[4];
    buf[0] = xs >> 8;
    buf[1] = xs & 0xFF;
    buf[2] = xe >> 8;
    buf[3] = xe & 0xFF;
    Command(0x2A);
	SendData(&buf, 4);       // MIPI_DCS_SET_COLUMN_ADDRESS

    buf[0] = ys >> 8;
    buf[1] = ys & 0xFF;
    buf[2] = ye >> 8;
    buf[3] = ye & 0xFF;
    Command(0x2B);
	SendData(&buf, 4);        // MIPI_DCS_SET_PAGE_ADDRESS

    Command(0x2C);      // MIPI_DCS_WRITE_MEMORY_START
}

/*
void CILI9341Display::fillRect(int xs, int ys, int xe, int ye, u16 color)
//void CILI9341Display::fillRect(int x, int y, int w, int h, int color)
{
	uint8_t xx[4] = { x >> 8, x, (x+w-1) >> 8, (x+w-1) };
	uint8_t yy[4] = { y >> 8, y, (y+h-1) >> 8, (y+h-1) };
	//uint32_t xx = __REV16(x | ((x + w - 1) << 16));
	//uint32_t yy = __REV16(y | ((y + h - 1) << 16));
	
	//send_command(ILI9341_COLUMN_ADDRESS_SET, 4, (uint8_t*)&xx);
	Command(ILI9341_COLUMN_ADDRESS_SET);
	SendData((uint8_t*)&xx, 4);

	//send_command(ILI9341_PAGE_ADDRESS_SET, 4, (uint8_t*)&yy);
	Command(ILI9341_PAGE_ADDRESS_SET);
	SendData((uint8_t*)&yy, 4);

	//send_command(ILI9341_MEMORY_WRITE, 0, NULL);
	Command(ILI9341_MEMORY_WRITE);

	int32_t len = w * h;
	while (len-- > 0) {
		//m_pSPIMaster->SetClock(m_nClockSpeed);
		//m_pSPIMaster->Write(m_nChipSelect, &color, 2);
		SendData(&color, 2);
	}
	CLogger::Get ()->Write ("ILI", LogNotice, "Fill Done.");
}
*/

void CILI9341Display::fillRect(int xs, int ys, int xe, int ye, u16 color)
{
    setWindow(xs,ys,xe,ye);
    u32 pixels = (xe-xs+1) * (ye-ys+1);
    
    u8 buf[2];
    buf[0] = color >> 8;
    buf[1] = color & 0xff;

    while (pixels--)
    {
		m_pSPIMaster->SetClock(m_nClockSpeed);
        m_pSPIMaster->Write(m_nChipSelect, buf, 2);
    }
}


void CILI9341Display::SetPixel (unsigned nPosX, unsigned nPosY, TILI9341Color Color)
{
	setWindow (nPosX, nPosY, nPosX, nPosY);

	SendData (&Color, sizeof Color);
}

void CILI9341Display::DrawText (unsigned nPosX, unsigned nPosY, const char *pString,
			       TILI9341Color Color, TILI9341Color BgColor)
{
	assert (pString != 0);

	unsigned nCharWidth = m_CharGen.GetCharWidth () * 2;
	unsigned nCharHeight = m_CharGen.GetCharHeight () * 2;

	TILI9341Color Buffer[nCharHeight][nCharWidth];

	char chChar;
	while ((chChar = *pString++) != '\0')
	{
		for (unsigned y = 0; y < nCharHeight; y++)
		{
			for (unsigned x = 0; x < nCharWidth; x++)
			{
				Buffer[y][x] =   m_CharGen.GetPixel (chChar, x/2, y/2)
					       ? Color : BgColor;
			}
		}

		setWindow (nPosX, nPosY, nPosX+nCharWidth-1, nPosY+nCharHeight-1);

		SendData (Buffer, sizeof Buffer);

		nPosX += nCharWidth;
	}
}

void CILI9341Display::Clear (TILI9341Color Color)
{
	assert (m_nWidth > 0);
	assert (m_nHeight > 0);

	setWindow (0, 0, m_nWidth-1, m_nHeight-1);

	TILI9341Color Buffer[m_nWidth];
	for (unsigned x = 0; x < m_nWidth; x++)
	{
		Buffer[x] = Color;
	}

	for (unsigned y = 0; y < m_nHeight; y++)
	{
		SendData (Buffer, sizeof Buffer);
	}
}
void CILI9341Display::On (void)
{
	assert (m_pTimer != 0);

	Command (ILI9341_DISPLAY_ON);
	m_pTimer->MsDelay (100);
}

void CILI9341Display::Off(void)
{
	Command (ILI9341_DISPLAY_OFF);
}