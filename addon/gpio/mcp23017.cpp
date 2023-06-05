#include "mcp23017.h"
#include <assert.h>
#include <circle/logger.h>


// MCP23x17 Registers
#define MCP23x17_IODIRA		0x00
#define MCP23x17_IPOLA	    0x02
#define MCP23x17_GPINTENA	0x04
#define MCP23x17_DEFVALA	0x06
#define MCP23x17_INTCONA	0x08
#define MCP23x17_IOCON		0x0A
#define MCP23x17_GPPUA		0x0C
#define MCP23x17_INTFA		0x0E
#define MCP23x17_INTCAPA	0x10
#define MCP23x17_GPIOA		0x12
#define MCP23x17_OLATA		0x14
#define MCP23x17_IODIRB		0x01
#define MCP23x17_IPOLB		0x03
#define MCP23x17_GPINTENB	0x05
#define MCP23x17_DEFVALB	0x07
#define MCP23x17_INTCONB	0x09
#define MCP23x17_IOCONB		0x0B
#define MCP23x17_GPPUB		0x0D
#define MCP23x17_INTFB		0x0F
#define MCP23x17_INTCAPB	0x11
#define MCP23x17_GPIOB		0x13
#define MCP23x17_OLATB		0x15

const char FromMCP23017[] = "mcp23017";


mcp23017::mcp23017(CI2CMaster *pI2CMaster, CGPIOManager *pGPIOManager, uint8_t ucSlaveAddress, unsigned intAPin, unsigned intBPin, unsigned I2CClockHz) 
:
    m_pI2CMaster(pI2CMaster),
    m_GpioIntA(intAPin, GPIOModeInput, pGPIOManager),
    m_GpioIntB(intBPin, GPIOModeInput, pGPIOManager),
    m_ucSlaveAddress(ucSlaveAddress),
    m_nI2CClockHz (I2CClockHz),
    lastA(0),
    lastB(0)
{   
}

boolean mcp23017::Initialize(void) 
{
	assert (m_pI2CMaster != 0);
	m_pI2CMaster->SetClock (m_nI2CClockHz);
	if (m_pI2CMaster->Write (m_ucSlaveAddress, 0, 0) != 0)
	{
		CLogger::Get ()->Write (FromMCP23017, LogError, "Device is not present (addr 0x%02X)",
					(unsigned) m_ucSlaveAddress);
		return FALSE;
	} 
    m_GpioIntA.ConnectInterrupt (InterruptHandlerA, this); 
    m_GpioIntB.ConnectInterrupt (InterruptHandlerB, this);

    uint8_t reg = 0xff;
    writeRegister(MCP23x17_IODIRA, reg);
    writeRegister(MCP23x17_IODIRB, reg);
    reg = 0xff;
    writeRegister(MCP23x17_GPPUA, reg);
    writeRegister(MCP23x17_GPPUB, reg);
    reg = 0;
    writeRegister(MCP23x17_IPOLA, reg);
    writeRegister(MCP23x17_IPOLB, reg);
    reg = 0;
    writeRegister(MCP23x17_INTCONA, reg);
    writeRegister(MCP23x17_INTCONB, reg);
    uint8_t ioconf = readRegister(MCP23x17_IOCON);
    ioconf = ioconf & 0xBB;
    ioconf = ioconf | 0x02;
    writeRegister(MCP23x17_IOCON, ioconf);
    ioconf = readRegister(MCP23x17_IOCONB);
    ioconf = ioconf & 0xBB;
    ioconf = ioconf | 0x02;
    writeRegister(MCP23x17_IOCONB, ioconf);
    reg = 0xff;
    writeRegister(MCP23x17_GPINTENA, reg);
    writeRegister(MCP23x17_GPINTENB, reg);

    // Make INT go inactive
    readRegister(MCP23x17_INTCAPA);
    readRegister(MCP23x17_INTCAPB);
    readRegister(MCP23x17_GPIOA); 
    readRegister(MCP23x17_GPIOB);

    initPinoutDataStructures();

    m_GpioIntA.EnableInterrupt(GPIOInterruptOnHighLevel);
    m_GpioIntB.EnableInterrupt(GPIOInterruptOnHighLevel);

	return TRUE;
}

void mcp23017::pinMode(uint8_t pin, uint8_t mode) {
    // Implementation for pinMode
}

void mcp23017::pullUpDnControl(uint8_t pin, uint8_t mode) {
    // Implementation for pullUpDnControl
}

bool mcp23017::writeRegister(uint8_t regAddr, uint8_t value) 
{
    assert (m_pI2CMaster != 0);

    const uint8_t Cmd[] = {regAddr, value};
	int nResult = m_pI2CMaster->Write (m_ucSlaveAddress, Cmd, sizeof Cmd);
	if (nResult != sizeof Cmd)
	{
		CLogger::Get ()->Write (FromMCP23017, LogWarning,
					"I2C write failed (err %d)", nResult);
		return false;
	}
    return true;
}

uint8_t mcp23017::readRegister(uint8_t regAddr) 
{
    assert (m_pI2CMaster != 0);

    m_SpinLock.Acquire();
    
    int nResult = m_pI2CMaster->Write (m_ucSlaveAddress, &regAddr, sizeof regAddr);
    if (nResult != sizeof regAddr)
	{
		CLogger::Get ()->Write (FromMCP23017, LogWarning,
					"I2C write failed (err %d)", nResult);
		return -1;
	}

    uint8_t pBuffer[1];
	nResult = m_pI2CMaster->Read (m_ucSlaveAddress, pBuffer, 1);

    m_SpinLock.Release();

	if (nResult != 1)
	{
		CLogger::Get ()->Write (FromMCP23017, LogWarning,
					"I2C read failed (err %d)", nResult);
		return -1;
	} 
	return pBuffer[0];
}


void mcp23017::RegisterEventHandler(TEventHandler *pHandler,  void *pParam) {
    assert(pHandler);
    assert(pParam);
	m_pEventHandler = pHandler;
	m_pCallerRef = pParam;
}

void mcp23017::InterruptHandlerA (void *pParam)
{
	mcp23017 *pThis = (mcp23017 *) pParam;
	assert (pThis != 0);
    uint8_t portVal = 0;
    portVal = pThis->readRegister(MCP23x17_GPIOA);
    pThis->decode(portVal, 0, pThis->lastA, pThis->rot16m, pThis);
    pThis->lastA = portVal;
}

void mcp23017::InterruptHandlerB (void *pParam)
{   
	mcp23017 *pThis = (mcp23017 *) pParam;
	assert (pThis != 0);
    uint8_t portVal = 0;
    portVal = pThis->readRegister(MCP23x17_GPIOB);
    pThis->decode(portVal, 2, pThis->lastB, pThis->rot12m, pThis);
    pThis->lastB = portVal;
}

void mcp23017::decode(uint8_t r, uint8_t encbase, uint8_t last, uint8_t rotm[][4], mcp23017 *pThis)
{
    assert(pThis);

    uint8_t e;
    uint8_t ret = 0;
    if ((r & 0x30) != (last & 0x30)) { //0x30 should really come from rotm
        e = encbase + 1;
        ret = rotaryDecoder(e, (r & rotm[e][1]) > 0, (r & rotm[e][2]) > 0, pThis);
        if (ret != 0) sendOffEvent(pThis, e, ret);
    }
    if ((r & 0x06) != (last & 0x06)) {
        e = encbase;
        ret = rotaryDecoder(e, (r & rotm[e][1]) > 0, (r & rotm[e][2]) > 0, pThis);
        if (ret != 0) sendOffEvent(pThis, e, ret);
    }

    uint8_t mask = 0x01;
    if ((r & mask) != (last & mask)) {
        e = encbase;
        ret = switchDecoder(e, r, mask);
        if (ret != 0) sendOffEvent(pThis, e, ret);
    }
    mask = 0x08;
    if ((r & mask) != (last & mask)) {
        e = encbase + 1;
        ret = switchDecoder(e, r, mask);
        if (ret != 0) sendOffEvent(pThis, e, ret);
    }
}


uint8_t mcp23017::rotaryDecoder(uint8_t e, uint8_t a, uint8_t b, mcp23017 *pThis)
{
        a = a ? 1 : 0;
        b = b ? 1 : 0;
        uint8_t pinstate = b << 1 | a;
        pThis->mEncoderState[e] = ttable[pThis->mEncoderState[e] & 0x0f][pinstate];
        uint8_t ret = pThis->mEncoderState[e] & 0x30;

        return ret;
}

uint8_t mcp23017::switchDecoder(uint8_t enc, uint8_t pin, uint8_t mask)
{
    uint8_t ret = (pin & mask) > 0 ? SW_UP : SW_DN;
    return ret;
}

void mcp23017::sendOffEvent(mcp23017 *pThis, uint8_t device, uint8_t event)
{
    assert(pThis);
    (*pThis->m_pEventHandler) (device << 6 | event , pThis->m_pCallerRef);
}


void mcp23017::initPinoutDataStructures() {

        for (int i = 0; i < 2; i++) {
            rot12m[i][0] = rot12[i][0];
            rot12m[i][1] = 1 << rot12[i][1];
            rot12m[i][2] = 1 << rot12[i][2];
            rot12m[i][3] = 1 << rot12[i][3];
        }
        
        for (int i = 0; i < 2; i++) {
            rot16m[i][0] = rot16[i][0];
            rot16m[i][1] = 1 << rot16[i][1];
            rot16m[i][2] = 1 << rot16[i][2];
            rot16m[i][3] = 1 << rot16[i][3];
        }
}

void mcp23017::dumpRegisters(){
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"\nPRTA=%.2X ---",readRegister(0x12));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"IODIR=%.2X",readRegister(0x00));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"PLLUP=%.2X",readRegister(0x0C));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTCH=%.2X",readRegister(0x04));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTFG=%.2X",readRegister(0x0E));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTCA=%.2X",readRegister(0x10));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"\nPRTB=%.2X---",readRegister(0x13));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"IODIR=%.2X",readRegister(0x01));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"PLLUP=%.2X",readRegister(0x0D));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTCG=%.2X",readRegister(0x05));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTFG=%.2X",readRegister(0x0F));
	CLogger::Get ()->Write (FromMCP23017, LogWarning,"INTCA=%.2X",readRegister(0x11));
}