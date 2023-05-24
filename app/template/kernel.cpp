//
// kernel.cpp
//
#include "kernel.h"
#include <circle/gpiopin.h>
#include <circle/startup.h>

#define SPI_MASTER_DEVICE	0		// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise
#define SPI_CLOCK_SPEED		15000000	// Hz
#define SPI_CPOL			1		// try 0, if it does not work
#define SPI_CPHA			0		// try 1, if it does not work
#define SPI_CHIP_SELECT		0		// 0 or 1; don't care, if not connected

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_SPIMaster (SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE)
{
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
		//m_Serial.RegisterMagicReceivedHandler ("PICORBOO", reboot); //Killing it here
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

	//m_Serial.RegisterMagicReceivedHandler ("PICORBOO", reboot);
	// Causes: serial.cpp(544): assertion failed: m_pInterruptSystem != 0

	// TODO: call Initialize () of added members here (if required)

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);
	//m_Serial.Write("Hello Serial World!\n", 13);

	m_Logger.Write (FromKernel, LogNotice, "Screen size %u x %u", m_Screen.GetWidth(), m_Screen.GetHeight());

	//m_Serial.RegisterMagicReceivedHandler ("PICORBOO", reboot);
	// Causes: serial.cpp(544): assertion failed: m_pInterruptSystem != 0

	// TODO: add your code here
	while(true) {
		m_ActLED.On ();
		CTimer::SimpleMsDelay (200);

		m_ActLED.Off ();
		CTimer::SimpleMsDelay (500);
	}

	return ShutdownHalt;
}
