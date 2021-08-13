#include "ADS1x15.h"

ADS1115::ADS1115(I2C *i2c, uint8_t i2cAddress)
{
	// shift 7 bit address 1 left: read expects 8 bit address, see mbed's I2C.h
	m_i2cAddress = i2cAddress << 1;
	m_i2c = i2c;
}

void ADS1115::writeRegister(uint8_t i2cAddress, uint8_t reg, uint16_t value)
{
	char cmd[3];
	cmd[0] = (char)reg;
	cmd[1] = (char)(value >> 8);
	cmd[2] = (char)(value & 0xFF);
	int I2Cres = m_i2c->write(i2cAddress, cmd, 3);
	if (I2Cres)
	{
		printf("ADS1x15 i2c error\r\n");
	}
}

uint16_t ADS1115::readRegister(uint8_t i2cAddress, uint8_t reg)
{
	char data[2];
	data[0] = reg; // temporary use this to send address to conversion register
	m_i2c->write(i2cAddress, data, 1);
	m_i2c->read(i2cAddress, data, 2);
	return (data[0] << 8 | data[1]);
}

void ADS1115::startConversation(chan_t chan)
{
	// Prepare Config Register
	uint16_t config = ADS1115_REG_CONFIG_OS_SINGLE |	// Begin a single conversion (when in power-down mode)
														//   ADS1115_REG_CONFIG_CQUE_4CONV |//ADS1115_REG_CONFIG_CQUE_NONE |	// Disable the comparator (default val)
					  ADS1115_REG_CONFIG_CLAT_NONLAT |	// Non-latching (default val)
					  ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active high   (default val)
					  ADS1115_REG_CONFIG_CMODE_TRAD |	// Traditional comparator (default val)
					  ADS1115_REG_CONFIG_MODE_SINGLE;	// Single-shot mode (default) ADS1115_REG_CONFIG_MODE_CONTIN;//

	// Set channel(s)
	config |= chan;

	// Set PGA/voltage range
	config |= m_voltageRange;

	// Set Data rate
	config |= m_dataRate;

	//conv ALRT/RDY Pin
	config |= m_convMask;

	// printf("start conv i2c %02x chan %04x vr %04x\r\n", m_i2cAddress >> 1, chan, m_voltageRange);

	// Write config register to the ADC
	writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);
}

adsVR_t ADS1115::setVoltageRangeByVal(float val)
{
	if (val < 0)
	{
		val *= -1.0;
	}

	if (val < 0.9 * 0.256) //0.2304
	{
		m_voltageRange = VR_p_m_0_256V;
	}
	else if (val < 0.9 * 0.512) //0.4608
	{
		m_voltageRange = VR_p_m_0_512V;
	}
	else if (val < 0.9 * 1.024) //0.9216
	{
		m_voltageRange = VR_p_m_1_024V;
	}
	else if (val < 0.9 * 2.048) //1.8432
	{
		m_voltageRange = VR_p_m_2_048V;
	}////////->>>
	else if (val < 0.9 * 4.096) //3.6864
	{
		m_voltageRange = VR_p_m_4_096V;
	}
	else
	{
		m_voltageRange = VR_p_m_6_144V;
	}
	this->setVoltageRange(m_voltageRange);
	return m_voltageRange;
}

void ADS1115::setVoltageRange(adsVR_t voltageRange)
{
	//												ADS1115
	// +/-6.144V range = Gain 2/3		1-bit = 0.1875mV
	// +/-4.096V range = Gain 1			1-bit = 0.125mV
	// +/-2.048V range = Gain 2		 	1-bit = 0.0625mV
	// +/-1.024V range = Gain 4			1-bit = 0.03125mV
	// +/-0.512V range = Gain 8			1-bit = 0.015625mV
	// +/-0.256V range = Gain 16		1-bit = 0.0078125mV
	m_voltageRange = voltageRange;
	// switch (m_voltageRange)
	// {
	// case VR_p_m_6_144V: //6.144/32767
	// 	m_bit_V = 0.1875e-3f;
	// 	break;
	// case VR_p_m_4_096V: //4.096/32767
	// 	m_bit_V = 0.125e-3f;
	// 	break;
	// case VR_p_m_2_048V: //2.048/32767
	// 	m_bit_V = 0.0625e-3f;
	// 	break;
	// case VR_p_m_1_024V: //1.024/32767
	// 	m_bit_V = 0.03125e-3f;
	// 	break;
	// case VR_p_m_0_512V: //0.512/32767
	// 	m_bit_V = 0.015625e-3f;
	// 	break;
	// case VR_p_m_0_256V: //0.256/32767
	// 	m_bit_V = 0.0078125e-3f;
	// 	break;
	// }
}

float ADS1115::getVoltage(int16_t adc_val)
{
	switch (m_voltageRange)
	{
	case VR_p_m_6_144V:
		return (float)adc_val * 6.144 / 32767.0;
		break;
	case VR_p_m_4_096V:
		return (float)adc_val * 4.096 / 32767.0;
		break;
	case VR_p_m_2_048V:
		return (float)adc_val * 2.048 / 32767.0;
		break;
	case VR_p_m_1_024V:
		// case VR_p_m_0_512V:
		// case VR_p_m_0_256V:
		return (float)adc_val * 1.024 / 32767.0;
		break;
	case VR_p_m_0_512V:
		return (float)adc_val * 0.512 / 32767.0;
		break;
	case VR_p_m_0_256V:
		return (float)adc_val * 0.256 / 32767.0;
		break;
	}
}

void ADS1115::setDataRate(adsDR_t dataRate)
{
	m_dataRate = dataRate;
	switch (dataRate)
	{
	case ADS1115_DR_8SPS:
		m_conversionDelay = 1000000 / 8;
		break;
	case ADS1115_DR_16SPS:
		m_conversionDelay = 1000000 / 16;
		break;
	case ADS1115_DR_32SPS:
		m_conversionDelay = 1000000 / 32;
		break;
	case ADS1115_DR_64SPS:
		m_conversionDelay = 1000000 / 64;
		break;
	case ADS1115_DR_128SPS:
		m_conversionDelay = 1000000 / 128;
		break;
	case ADS1115_DR_250SPS:
		m_conversionDelay = 1000000 / 250;
		break;
	case ADS1115_DR_475SPS:
		m_conversionDelay = 1000000 / 475;
		break;
	case ADS1115_DR_860SPS:
		m_conversionDelay = 1000000 / 860;
		break;
	}
}

int ADS1115::getConversationDelay()
{
	return m_conversionDelay;
}

int16_t ADS1115::readADC(chan_t chan)
{
	this->startConversation(chan);
	wait_us(m_conversionDelay + 2000);

	return this->getLastConversionResults();
}

float ADS1115::getLastConversionResults_V()
{
	return this->getVoltage(this->getLastConversionResults());
}

float ADS1115::readADC_V(chan_t chan)
{
	this->startConversation(chan);
	// // Wait for the conversion to complete
	wait_us(m_conversionDelay + 2000);

	return this->getLastConversionResults_V();
}

void ADS1115::enableConvReadyPin(uint8_t convCnt)
{

	switch (convCnt)
	{
	case 0:
		m_convMask = cque_none;
		break;
	case 1:
		m_convMask = cque_1conv;
		break;
	case 2:
		m_convMask = cque_2conv;
		break;
	case 4:
		m_convMask = cque_4conv;
		break;
	default:
		m_convMask = cque_1conv;
		break;
	}

	writeRegister(m_i2cAddress, ADS1115_REG_POINTER_HITHRESH, 0xFFFF);
	writeRegister(m_i2cAddress, ADS1115_REG_POINTER_LOWTHRESH, 0x0000);
}

void ADS1115::startComparator_SingleEnded(chan_t chan, int16_t threshold)
{
	// Prepare Config Register
	uint16_t config = ADS1115_REG_CONFIG_CLAT_LATCH |	// Latching mode
					  ADS1115_REG_CONFIG_CPOL_ACTVLOW | // Alert/Rdy active low   (default val)
					  ADS1115_REG_CONFIG_CMODE_TRAD |	// Traditional comparator (default val)
					  ADS1115_REG_CONFIG_MODE_SINGLE;	// Single-shot mode (default)

	// Set channel(s)
	config |= chan;

	// Set PGA/voltage range
	config |= m_voltageRange;

	// Set Data rate
	config |= m_dataRate;

	//conv ALRT/RDY Pin
	config |= m_convMask;

	// Set the high threshold register
	// Shift 12-bit results left 4 bits for the ADS1115
	writeRegister(m_i2cAddress, ADS1115_REG_POINTER_HITHRESH, threshold);

	// Write config register to the ADC
	writeRegister(m_i2cAddress, ADS1115_REG_POINTER_CONFIG, config);
}

int16_t ADS1115::getLastConversionResults()
{
	return (int16_t)readRegister(m_i2cAddress, ADS1115_REG_POINTER_CONVERT);
}
