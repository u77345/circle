//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2021  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <circle/string.h> 
#include <circle/gpiopin.h>
#include <circle/machineinfo.h>


#define SPI_MASTER_DEVICE	0		// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_CLOCK_SPEED		15000000	// Hz
#define SPI_CPOL		1		// try 0, if it does not work
#define SPI_CPHA		0		// try 1, if it does not work
#define SPI_CHIP_SELECT	0       // 0 or 1; don't care, if not connected - 0: GPIO_CE0

#define WIDTH			320		// display width in pixels
#define HEIGHT			240		// display height in pixels
#define DC_PIN			25		// gpio 25

//MCP23017 GPIO
#define MCP23107_ADDR 			0x20
#define MCP23017_INTA_GPIO 		16  
#define MCP23017_INTB_GPIO 		12  
#define MCP23017_I2C_CLOCKHZ	500000

//#define MY_COLOR		ILI9341_COLOR (31, 31, 15)	// any color
#define MY_COLOR		ILI9341_COLOR (31, 31, 0)	// any color

static const char FromKernel[] = "kernel";
//static unsigned g_seed;
#define I2C_MASTER_DEVICE	(CMachineInfo::Get ()->GetDevice (DeviceI2CMaster))
#define I2C_ADDR 0x00 

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_SPIMaster (SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE),
	m_I2CMaster (I2C_MASTER_DEVICE),
	m_Display (&m_SPIMaster, DC_PIN, WIDTH, HEIGHT,
		   SPI_CPOL, SPI_CPHA, SPI_CLOCK_SPEED, SPI_CHIP_SELECT),
	m_GPIOManager (&m_Interrupt),

	m_GPIOExtender (&m_I2CMaster, &m_GPIOManager, MCP23107_ADDR, MCP23017_INTA_GPIO, MCP23017_INTB_GPIO, MCP23017_I2C_CLOCKHZ)
{
	//m_ActLED.Blink (5);	// show we are alive
	//Turn off backlight here
	m_ActLED.On(); //Show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		} else {
			pTarget = &m_Serial;
		}


		bOK = m_Logger.Initialize (pTarget);
	}

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_I2CMaster.Initialize ();
	}

	if (bOK)
	{
		bOK = m_GPIOManager.Initialize ();
	}

	if (bOK)
	{
		bOK = m_SPIMaster.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Display.Initialize ();
	}

	if (bOK)
	{
		bOK = m_GPIOExtender.Initialize();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    m_Display.fillRect(0, 0, WIDTH, HEIGHT, 0x0000);
	m_Display.DrawText (10, 10, "Turbo Pascal 2.0", MY_COLOR);

	m_GPIOExtender.RegisterEventHandler(EventHandler, this);

	//23017 Debug
	while (false) {
		unsigned portA = m_GPIOExtender.readRegister(0x12);
		CString String;
		String.Format ("%2X", portA);
		m_Display.DrawText (10, 50, String, MY_COLOR);

		unsigned portB = m_GPIOExtender.readRegister(0x13);
		String.Format ("%2X", portB);
		m_Display.DrawText (10, 100, String, MY_COLOR);
	}

	// countdown
	while (true) {
		static const unsigned InitialCounter = 500;
		for (int n = InitialCounter; n > 0; n--)
		{
			CString String;
			String.Format ("%4d", n);

			m_Display.DrawText (10, 50, String, MY_COLOR);

			unsigned nPosX = (WIDTH-1) * (InitialCounter-n) / InitialCounter;
			m_Display.SetPixel (nPosX, 98, MY_COLOR);
			m_Display.SetPixel (nPosX, 99, MY_COLOR);
		}
		//m_GPIOExtender.dumpRegisters();
		//m_GPIOExtender.readRegister(0x12);
		//m_GPIOExtender.readRegister(0x13);
	}
	

	m_Display.Clear ();


	// draw rectangle
	for (unsigned nPosX = 0; nPosX < WIDTH; nPosX++)
	{
		m_Display.SetPixel (nPosX, 0, MY_COLOR);
		m_Display.SetPixel (nPosX, WIDTH-1, MY_COLOR);
	}
	for (unsigned nPosY = 0; nPosY < WIDTH; nPosY++)
	{
		m_Display.SetPixel (0, nPosY, MY_COLOR);
		m_Display.SetPixel (WIDTH-1, nPosY, MY_COLOR);
	}

	// draw cross
	for (unsigned nPosX = 0; nPosX < WIDTH; nPosX++)
	{
		unsigned nPosY = nPosX * HEIGHT / WIDTH;

		m_Display.SetPixel (nPosX, nPosY, MY_COLOR);
		m_Display.SetPixel (WIDTH-nPosX-1, nPosY, MY_COLOR);
	}

	/*
	while(true) {
		m_ActLED.On();

		CTimer::SimpleMsDelay (100);

		m_ActLED.Off();
		CTimer::SimpleMsDelay (100);
	}
	*/

	m_Timer.MsDelay (5000);

	//m_Display.Off();
	m_Display.Clear();

	return ShutdownReboot;
}

/*
static unsigned randomNumber()
{
	g_seed = (214013*g_seed+2531011);
	return (g_seed>>16)&0x7fff;
}
*/


void CKernel::EventHandler (uint8_t event, void *pParam)
{
	CKernel *pThis = static_cast<CKernel *> (pParam);
	assert (pThis != 0);
	CLogger::Get ()->Write (FromKernel, LogWarning,	"Evt: %02X", event);
	pThis->m_ActLED.Toggle();
}
