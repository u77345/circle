#include <display/ili9341.h>>
#include <assert.h>

CILI9341Display::CILI9341Display (CSPIMaster *pSPIMaster,
				unsigned nDCPin, unsigned nBackLightPin,
				unsigned nWidth, unsigned nHeight,
				unsigned CPOL, unsigned CPHA, unsigned nClockSpeed,
				unsigned nChipSelect)
:	m_pSPIMaster (pSPIMaster),
	m_nBackLightPin (nBackLightPin),
	m_nWidth (nWidth),
	m_nHeight (nHeight),
	m_CPOL (CPOL),
	m_CPHA (CPHA),
	m_nClockSpeed (nClockSpeed),
	m_nChipSelect (nChipSelect),
	m_DCPin (nDCPin, GPIOModeOutput),
	m_pTimer (CTimer::Get ())
{
	assert (nDCPin != None);

	if (m_nBackLightPin != None)
	{
		m_BackLightPin.AssignPin (m_nBackLightPin);
		m_BackLightPin.SetMode (GPIOModeOutput, FALSE);
	}

	if (m_nResetPin != None)
	{
		m_ResetPin.AssignPin (m_nResetPin);
		m_ResetPin.SetMode (GPIOModeOutput, FALSE);
	}
}