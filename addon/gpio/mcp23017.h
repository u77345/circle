/*

wiringPiI2CReadReg8(self.bus, mcp23x17_GPIOA)
mcp23017Setup(100, 0x20) //100 is pin base
pinMode(23, wp.GPIO.INPUT)
pullUpDnControl(23, wp.PUD_UP)
wiringPiI2CWriteReg8(self.bus, mcp23x17_IODIRA, reg)
*/

#ifndef mcp23017_H
#define mcp23017_H

#include <stdint.h>
#include <circle/types.h>
#include <circle/gpiomanager.h>
#include <circle/gpiopin.h>
#include <circle/i2cmaster.h>


#define R_START   0x0
#define DIR_NONE  0x0
#define DIR_CW    0x10
#define DIR_CCW   0x20
#define SW_DN     0x01
#define SW_UP     0x02 
/*#define HALF_STEP False*/


class mcp23017 {
public:

	typedef void TEventHandler (uint8_t event, void *pParam);

    mcp23017(CI2CMaster *pI2CMaster, CGPIOManager *pGPIOManager, uint8_t ucSlaveAddress = 0x20, unsigned intAPin = 0, unsigned intBPin = 0, unsigned I2CClockHz = 400000 );

    boolean Initialize(void);
    void    pinMode(uint8_t pin, uint8_t mode);
    void    pullUpDnControl(uint8_t pin, uint8_t mode);
    boolean writeRegister(uint8_t regAddr, uint8_t value);
    uint8_t readRegister(uint8_t regAddr);
    void    RegisterEventHandler(TEventHandler *pHandler,  void *pParam);
    void    dumpRegisters(void);

private:

    static void InterruptHandlerA (void *pParam);
    static void InterruptHandlerB (void *pParam);
    void decode(uint8_t r, uint8_t encbase, uint8_t last, uint8_t rotm[][4], mcp23017 *pThis);
    void initPinoutDataStructures();
    uint8_t rotaryDecoder(uint8_t e, uint8_t a, uint8_t b, mcp23017 *pThis);
    uint8_t switchDecoder(uint8_t enc, uint8_t pin, uint8_t mask);
    void sendOffEvent(mcp23017 *pThis, uint8_t device, uint8_t event);

    TEventHandler   *m_pEventHandler;
    void            *m_pCallerRef;   
    CI2CMaster      *m_pI2CMaster;
    CGPIOPin        m_GpioIntA;
    CGPIOPin        m_GpioIntB;
	uint8_t	        m_ucSlaveAddress;
    unsigned        m_nI2CClockHz;
    CSpinLock       m_SpinLock;
    uint8_t         lastA;
    uint8_t         lastB;

    // Rotary pin definitions
    uint8_t         rot12[2][4] = {
    //  Eid A  B  S - indicating bit position in the byte off of 2301
        {1, 1, 2, 0},
        {2, 4, 5, 3}
    };
    uint8_t         rot16[2][4] = {
        {3, 1, 2, 0},
        {4, 4, 5, 3}
    };

    // Rotary pin masks. Will be calculated
    uint8_t         rot12m[2][4];
    uint8_t         rot16m[2][4];

    // Encoder state tracking
    uint8_t mEncoderState[4] = { 1, 1, 1, 1};

    #ifdef HALF_STEP
    // Use the half-step state table (emits a code at 00 and 11)
    #define R_CCW_BEGIN 0x1
    #define R_CW_BEGIN 0x2
    #define R_START_M 0x3
    #define R_CW_BEGIN_M 0x4
    #define R_CCW_BEGIN_M 0x5
    const uint8_t ttable[6][4] = {
    // R_START (00)
    {R_START_M,            R_CW_BEGIN,     R_CCW_BEGIN,  R_START},
    // R_CCW_BEGIN
    {R_START_M | DIR_CCW, R_START,        R_CCW_BEGIN,  R_START},
    // R_CW_BEGIN
    {R_START_M | DIR_CW,  R_CW_BEGIN,     R_START,      R_START},
    // R_START_M (11)
    {R_START_M,            R_CCW_BEGIN_M,  R_CW_BEGIN_M, R_START},
    // R_CW_BEGIN_M
    {R_START_M,            R_START_M,      R_CW_BEGIN_M, R_START | DIR_CW},
    // R_CCW_BEGIN_M
    {R_START_M,            R_CCW_BEGIN_M,  R_START_M,    R_START | DIR_CCW},
    };
    #else
    // Use the full-step state table (emits a code at 00 only)
    #define R_CW_FINAL 0x1
    #define R_CW_BEGIN 0x2
    #define R_CW_NEXT 0x3
    #define R_CCW_BEGIN 0x4
    #define R_CCW_FINAL 0x5
    #define R_CCW_NEXT 0x6

    const uint8_t ttable[7][4] = {
    // R_START
    {R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START},
    // R_CW_FINAL
    {R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW},
    // R_CW_BEGIN
    {R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START},
    // R_CW_NEXT
    {R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START},
    // R_CCW_BEGIN
    {R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START},
    // R_CCW_FINAL
    {R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW},
    // R_CCW_NEXT
    {R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START},
    };
    #endif



};


#endif // mcp23017_H