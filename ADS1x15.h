#ifndef ADS1115_H
#define ADS1115_H

#include <mbed.h>

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
#define ADS1115_ADDRESS (0x48) // 0100 1000 (ADDR = GND)
/*=========================================================================*/


/*=========================================================================
    POINTER REGISTER
    -----------------------------------------------------------------------*/
#define ADS1115_REG_POINTER_MASK (0x03)
#define ADS1115_REG_POINTER_CONVERT (0x00)
#define ADS1115_REG_POINTER_CONFIG (0x01)
#define ADS1115_REG_POINTER_LOWTHRESH (0x02)
#define ADS1115_REG_POINTER_HITHRESH (0x03)
/*=========================================================================*/

/*=========================================================================
    CONFIG REGISTER
    -----------------------------------------------------------------------*/
#define ADS1115_REG_CONFIG_OS_MASK (0x8000)
#define ADS1115_REG_CONFIG_OS_SINGLE (0x8000)  // Write: Set to start a single-conversion
#define ADS1115_REG_CONFIG_OS_BUSY (0x0000)    // Read: Bit = 0 when conversion is in progress
#define ADS1115_REG_CONFIG_OS_NOTBUSY (0x8000) // Read: Bit = 1 when device is not performing a conversion

#define ADS1115_REG_CONFIG_MODE_MASK (0x0100)
#define ADS1115_REG_CONFIG_MODE_CONTIN (0x0000) // Continuous conversion mode
#define ADS1115_REG_CONFIG_MODE_SINGLE (0x0100) // Power-down single-shot mode (default)

#define ADS1115_REG_CONFIG_CMODE_MASK (0x0010)
#define ADS1115_REG_CONFIG_CMODE_TRAD (0x0000)   // Traditional comparator with hysteresis (default)
#define ADS1115_REG_CONFIG_CMODE_WINDOW (0x0010) // Window comparator

#define ADS1115_REG_CONFIG_CPOL_MASK (0x0008)
#define ADS1115_REG_CONFIG_CPOL_ACTVLOW (0x0000) // ALERT/RDY pin is low when active (default)
#define ADS1115_REG_CONFIG_CPOL_ACTVHI (0x0008)  // ALERT/RDY pin is high when active

#define ADS1115_REG_CONFIG_CLAT_MASK (0x0004)   // Determines if ALERT/RDY pin latches once asserted
#define ADS1115_REG_CONFIG_CLAT_NONLAT (0x0000) // Non-latching comparator (default)
#define ADS1115_REG_CONFIG_CLAT_LATCH (0x0004)  // Latching comparator

#define ADS1115_REG_CONFIG_CQUE_MASK (0x0003)
/*=========================================================================*/

typedef enum
{
    chan_0_1 = 0x0000, // Differential P = AIN0, N = AIN1 (default)
    chan_0_3 = 0x1000, // Differential P = AIN0, N = AIN3
    chan_1_3 = 0x2000, // Differential P = AIN1, N = AIN3
    chan_2_3 = 0x3000, // Differential P = AIN2, N = AIN3
    chan_0 = 0x4000,   // Single-ended AIN0
    chan_1 = 0x5000,   // Single-ended AIN1
    chan_2 = 0x6000,   // Single-ended AIN2
    chan_3 = 0x7000    // Single-ended AIN3
} chan_t;

// The +/-6.144V and +/-4.096V settings express the full-scale range of the ADC scaling.
// In no event should more than VDD + 0.3V be applied to this device
typedef enum
{                           //																	ADS1115		ADS1115
    VR_p_m_6_144V = 0x0000, // +/-6.144V range = Gain 2/3		1-bit = 3mV     	0.1875mV
    VR_p_m_4_096V = 0x0200, // +/-4.096V range = Gain 1		1-bit = 2mV      	0.125mV
    VR_p_m_2_048V = 0x0400, // +/-2.048V range = Gain 2	 	1-bit = 1mV      	0.0625mV
    VR_p_m_1_024V = 0x0600, // +/-1.024V range = Gain 4		1-bit = 0.5mV    	0.03125mV
    VR_p_m_0_512V = 0x0800, // +/-0.512V range = Gain 8		1-bit = 0.25mV   	0.015625mV
    VR_p_m_0_256V = 0x0A00  // +/-0.256V range = Gain 16		1-bit = 0.125mV  	0.0078125mV
} adsVR_t;                  // VR: Voltage Range

typedef enum
{
    ADS1115_DR_8SPS = 0x0000,   //    8 SPS
    ADS1115_DR_16SPS = 0x0020,  //   16 SPS
    ADS1115_DR_32SPS = 0x0040,  //   32 SPS
    ADS1115_DR_64SPS = 0x0060,  //   64 SPS
    ADS1115_DR_128SPS = 0x0080, //  128 SPS
    ADS1115_DR_250SPS = 0x00A0, //  250 SPS
    ADS1115_DR_475SPS = 0x00C0, //  475 SPS
    ADS1115_DR_860SPS = 0x00E0  //  860 SPS
} adsDR_t;                      // DR: Data Rate

typedef enum
{
    cque_1conv = 0x0000,// Assert ALERT/RDY after one conversions
    cque_2conv = 0x0001,// Assert ALERT/RDY after two conversions
    cque_4conv = 0x0002,// Assert ALERT/RDY after four conversions
    cque_none = 0x0003 // Disable the comparator and put ALERT/RDY in high state (default)
} conv_t;


class ADS1115
{
public:
    ADS1115(I2C *i2c, uint8_t i2cAddress = ADS1115_ADDRESS);
    int16_t readADC(chan_t chan);
    double readADC_V(chan_t chan);
    void startComparator_SingleEnded(chan_t chan, int16_t threshold);
    int16_t getLastConversionResults();
    double getLastConversionResults_V();
    void startConversation(chan_t chan);
    int getConversationDelay();
    double getVoltage(int16_t adc_val);

    void setVoltageRange(adsVR_t voltageRange);
    void setDataRate(adsDR_t dataRate);

    void enableConvReadyPin(uint8_t convCnt);

    adsVR_t setVoltageRangeByVal(double val);

private:
    // Instance-specific properties
    uint8_t m_i2cAddress;
    int m_conversionDelay;
    adsVR_t m_voltageRange{VR_p_m_4_096V};
    adsDR_t m_dataRate{ADS1115_DR_128SPS};
    conv_t m_convMask{cque_none};
    float m_bit_V{0.};
    I2C *m_i2c;

    uint16_t readRegister(uint8_t i2cAddress, uint8_t reg);
    void writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value);
};


#endif
