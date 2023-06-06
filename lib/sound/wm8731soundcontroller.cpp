//
// wm8731soundcontroller.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2022  R. Stange <rsta2@o2online.de>
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
#include <circle/sound/wm8731soundcontroller.h>
#include <assert.h>

CWM8731SoundController::CWM8731SoundController (CI2CMaster *pI2CMaster, u8 uchI2CAddress)
:	m_pI2CMaster (pI2CMaster),
	m_uchI2CAddress (uchI2CAddress)
{
}

boolean CWM8731SoundController::Probe (void)
{
	if (m_uchI2CAddress)
	{
		return InitWM8731 (m_uchI2CAddress);
	}

	if (InitWM8731 (0x1A))
	{
		m_uchI2CAddress = 0x1A;

		return TRUE;
	}

	return FALSE;
}

#define WM8731_REG_LLINEIN	0
#define WM8731_REG_RLINEIN	1
#define WM8731_REG_LHEADOUT	2
#define WM8731_REG_RHEADOUT	3
#define WM8731_REG_ANALOG	4
#define WM8731_REG_DIGITAL	5
#define WM8731_REG_POWERDOWN	6
#define WM8731_REG_INTERFACE	7
#define WM8731_REG_SAMPLING	8
#define WM8731_REG_ACTIVE	9
#define WM8731_REG_RESET	15


// For Wolfsons, including WM8731 the i2c register is 7 bits and value is 9 bits,  
// so let's have a helper for packing this into two bytes
#define SHIFT_BIT(r, v) {((v&0x0100)>>8) | (r<<1), (v&0xff)}

boolean CWM8731SoundController::InitWM8731 (u8 uchI2CAddress)
{
	// based on https://github.com/RASPIAUDIO/ULTRA/blob/main/ultra.c
	// Licensed under GPLv3
	static const u8 initBytes[][2] =
	{
		/*
		{ 0, 0x0097 },
		{ 1, 0x0097 },
		{ 2, 0x0079 },
		{ 3, 0x0079 },
		{ 4, 0x000a },
		{ 5, 0x0000 },  //DACMO=0 - not sound still
		{ 6, 0x009f },
		{ 7, 0x000a },  //This line is good
		{ 8, 0x0000 },
		{ 9, 0x0000 }
		*/

		// reset
		SHIFT_BIT(WM8731_REG_RESET, 0),

		SHIFT_BIT(WM8731_REG_INTERFACE, 0x0A),   //was 0A OK. 4A, 8A is really bad. 08 clipped.
			// 7    BCLKINV:1 = Invert BCLK, 
			//				0 = Don’t invert BCLK
			// 6    MS:     1 = Enable Master Mode, 
			//				0 = Enable Slave Mode 
			// 5    LRSWAP: 1 = Right Channel DAC Data Left, 0 = Right Channel DAC Data Right
			// 4    LRP:    DACLRC phase control (in left, right or I2S modes)
			//		 		1 = Right Channel DAC data when DACLRC high
			//		 		0 = Right Channel DAC data when DACLRC low
			// 3:2  IWL:	Input Audio Data Bit Length Select 
			//				11 = 32 bits
			//				10 = 24 bits
			//				01 = 20 bits
			//				00 = 16 bits
			// 1:0  FORMAT: Audio Data Format Select
			//				11 = DSP Mode, frame sync + 2 data packed words
			//				10 = I2S Format, MSB-First left-1 justified
			//				01 = MSB-First, left justified 
			//				00 = MSB-First, right justified

		SHIFT_BIT(WM8731_REG_SAMPLING, 0x01),  //GG: MLCK 12MHz, Samle 48KHz, BOSR=0, SR3...SR0=0
			// 5:2	SR		0000: ADC, DAC 48KHz, MCLK 12MHz, expected BOSR=0
			// 1	BOSR	Base Over-Sampling Rate
			// 0 	MODESL  Mode Select
			//				1 = USB mode (250/272fs)  <==== Enabling USB mode made it rock solid !!!!
			//				0 = Normal mode (256/384fs)

		// In order to prevent pops, the DAC should first be soft-muted (DACMU),
		// the output should then be de-selected from the line and headphone output
		// (DACSEL), then the DAC powered down (DACPD).

		SHIFT_BIT(WM8731_REG_DIGITAL,   0x08),   // DAC soft mute
		SHIFT_BIT(WM8731_REG_ANALOG,    0x00),    // disable all
		SHIFT_BIT(WM8731_REG_POWERDOWN, 0x00), // codec powerdown
			
		SHIFT_BIT(WM8731_REG_LHEADOUT,  0x7F),      // volume off - was 0x80
			// 0x80 = WM8731_HEADOUT_ZCEN
		SHIFT_BIT(WM8731_REG_RHEADOUT,  0x7F),      // was 0x80

		SHIFT_BIT(WM8731_REG_ACTIVE, 1),
			
		SHIFT_BIT(WM8731_REG_DIGITAL, 0x00),   // DAC unmuted
		SHIFT_BIT(WM8731_REG_ANALOG, 0x10)    // DAC selected
		
	};

	assert (m_pI2CMaster);
	assert (uchI2CAddress);
	for (auto &command : initBytes)
	{
		if (   m_pI2CMaster->Write (uchI2CAddress, &command, sizeof (command))
		    != sizeof (command))
		{
			return FALSE;
		}
	}

	return TRUE;
}
