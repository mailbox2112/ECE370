/**
 * LIS23DH.cpp
 *
 *  Created on: <DATE>
 *      Author: <NAME>
 */

#include "LIS3DH.h"
#define USART_CTRL_CONFIG (0x1 << 0) + (0x1 << 12) + (0x3 << 8) + (0x1 << 10)
#define USART_CLK_CONFIG 0x280
#define USART_CMD_CONFIG (0x1) + (0x1 << 2) + (0x1 << 4)
#define USART_ROUTING_CONFIG (0x1 << 8) + (0xB)
#define USART_FRAMING_CONFIG 0x5
#define WHO_AM_I 0x0Fu
#define DUMMY_DATA 0x25u
#define READ 0b10000000u
#define WRITE 0b00000000u
#define TXBL_MASK 0b1000000u
#define TXC_MASK 0b100000u
#define CTRL_REG1 0x20u
#define CTRL_REG4 0x23u
#define CTRL1_CONFIG 0x47u
#define CTRL4_CONFIG 0x88u
#define OUT_X_L 0x28u
#define OUT_X_H 0x29u
#define OUT_Y_L 0x2Au
#define OUT_Y_H 0x2Bu
#define OUT_Z_L 0x2Cu
#define OUT_Z_H 0x2Du

/**
 * Default constructor. Sets up default values
 */
LIS3DH::LIS3DH() {
	/**
	 * Default bus speed of 2MHz
	 */
	busSpeed=2000000;

	/**
	 * Default Pin Location #1
	 */
	pinLocation=1;

	/**
	 * Default USART1
	 */
	usart=USART1;

	/**
	 * Default scale range of +/- 2G
	 */
	scaleRange=2;

	/**
	 * Default sensitivity of 1mg/digit
	 */
	sensitivity=1;
}

/**
 * Destructor. Does nothing.
 */
LIS3DH::~LIS3DH() {

}

/**
 * Helper function to setup the associated clocks
 */
void LIS3DH::setupClocks() {
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART1, true);
}

/**
 * Helper function to setup the associated GPIO
 */
void LIS3DH::setupGPIO() {
	// Set MOSI pin to output
	GPIO_PinModeSet(gpioPortD, 0, gpioModePushPull, 1);

	// Set the MISO pin to input
	GPIO_PinModeSet(gpioPortD, 1, gpioModeInput, 0);

	// Set CLK pin to output
	GPIO_PinModeSet(gpioPortD, 2, gpioModePushPull, 1);

	// Set CS pin to output
	GPIO_PinModeSet(gpioPortD, 3, gpioModePushPull, 1);
}

/**
 * Helper function to setup the associated GPIO
 */
void LIS3DH::setupUSART() {
	// Enable synchronous mode, TX buffer level, clock phase and polarity, MSBF
	usart->CTRL |= USART_CTRL_CONFIG;

	// 2 MHz clock rate
	usart->CLKDIV = USART_CLK_CONFIG;

	// Set up CMD register
	usart->CMD = USART_CMD_CONFIG;

	// Set up packet framing
	usart->FRAME = USART_FRAMING_CONFIG;

	// Set up routing
	// Location 1, TXEN, RXEN, CLKEN
	usart->ROUTE |= USART_ROUTING_CONFIG;
}

/**
 * Setup accelerometer to default parameters
 * defined in project specification
 */
void LIS3DH::setupLIS3DH() {
	// Write the CTRL_REG1 to high resolution mode, xyz axis enabled
	writeRegister(CTRL_REG1, CTRL1_CONFIG);

	// Write the CTRL_REG4 config
	writeRegister(CTRL_REG4, CTRL4_CONFIG);
}

/**
 * Read a single register and return its value
 * @param address 8-bit address of internal register to read
 * @return	the value that was contained in that register
 */
uint8_t LIS3DH::readRegister(uint8_t address) {
	// Lower CS line
	GPIO_PinOutClear(gpioPortD, 3);

	// SPI_Transfer sends the read address and returns the junk data sent back
	SPI_Transfer(READ + address);

	// SPI_Transfer sends junk data in return and gets the results we want
	uint8_t result = SPI_Transfer(DUMMY_DATA);

	// Raise the CS line
	GPIO_PinOutSet(gpioPortD, 3);

	// Return the data we got back
	return result;
}

/**
 * Conduct transfer on SPI bus with included data
 * @param data 8-bit data that should be transmitted over the SPI bus
 * @return received 8-bits as result of transmission
 */
uint8_t LIS3DH::SPI_Transfer(uint8_t data) {
	// Read the TX buffer level flag
	// If the buffer is full, wait until it's empty
	while(((usart->STATUS & TXBL_MASK) >> 6) != 1) {}

	// Send the data
	usart->TXDATA = data;

	// Wait for the data to finish sending
	while(((usart->STATUS & TXC_MASK) >> 5) != 1) {}

	// Return whatever we received
	uint8_t received = usart->RXDATA;
	return received;
}

/**
 * Write a single register
 * @param address 8-bit address of internal register to read
 * @param data 8-bit data value that should be written to the register
 */
void LIS3DH::writeRegister(uint8_t address , uint8_t data) {
	// Lower CS line
	GPIO_PinOutClear(gpioPortD, 3);

	// SPI_Transfer sends the read address and returns the junk data sent back
	SPI_Transfer(WRITE + address);

	// SPI_Transfer sends data and returns the junk data sent back
	SPI_Transfer(data);

	// Raise the CS line
	GPIO_PinOutSet(gpioPortD, 3);
}

/**
 * Initialize SPI device to communicate 2 MHz, sample at 50Hz, enable
 * all axis measurements, full power on. Set the FIFO to Bypass Mode (default)
 *
 *@return result returns true if initialization completed successfully, otherwise false
 */
bool LIS3DH::initialize()
{
	/**
	 * ensure device has sufficient time to power on
	 * and load configuration information. See datasheet.
	 */
	delay(5);

	//configure the required clocks
	setupClocks();

	//configure the required GPIO
	setupGPIO();

	//configure the USART
	setupUSART();

	//configure the LIS3DH
	setupLIS3DH();

	return true;
}

/**
 * Perform a self-test by reading the WHO_AM_I
 * register.
 * @return returns true is self-test was completed successfully, otherwise false
 */
bool LIS3DH::selfTest()
{
	/**
	 * Perform a SPI read from the WHO_AM_I register.
	 * The returned value should be 0x33 if successful.
	 * Return true if successful, false otherwise.
	 */
	uint8_t result = readRegister(WHO_AM_I);

	if(result == 0x33) {
		return true;
	} else {
		return false;
	}
}

/**
 * Get X-axis acceleration and convert to a floating point
 * @return x-axis acceleration expressed as a floating point
 */
float LIS3DH::getXAcceleration() {
	// Read the low and high 8 bits of the acceleration value
	uint8_t lowByte = readRegister(OUT_X_L);
	uint8_t highByte = readRegister(OUT_X_H);

	// Get the acceleration in integer form, then convert to floating point normalized to 1g
	int16_t accel = (int16_t)((uint16_t)highByte << 8) + ((uint16_t)lowByte);
	float finalAccel = convertReadingToAccel(accel);
	return finalAccel;
}

/**
 * Get Y-axis acceleration and convert to a floating point
 * @return y-axis acceleration expressed as a floating point
 */
float LIS3DH::getYAcceleration() {
	// Read the low and high 8 bits of the acceleration value
	uint8_t lowByte = readRegister(OUT_Y_L);
	uint8_t highByte = readRegister(OUT_Y_H);

	// Get the acceleration in integer form, then convert to floating point normalized to 1g
	int16_t accel = (int16_t)((uint16_t)highByte << 8) + ((uint16_t)lowByte);
	float finalAccel = convertReadingToAccel(accel);
	return finalAccel;
}


/**
 * Get Z-axis acceleration and convert to a floating point
 * @return z-axis acceleration expressed as a floating point
 */
float LIS3DH::getZAcceleration() {
	// Read the low and high 8 bits of the acceleration value
	uint8_t lowByte = readRegister(OUT_Z_L);
	uint8_t highByte = readRegister(OUT_Z_H);

	// Get the acceleration in integer form, then convert to floating point normalized to 1g
	int16_t accel = (int16_t)((uint16_t)highByte << 8) + ((uint16_t)lowByte);
	float finalAccel = convertReadingToAccel(accel);
	return finalAccel;
}

/**
 * Convert int16_t reading into floating point acceleration value
 * @param  2's complement signed value to be converted in a floating point acceleration
 * @return floating point value expressing an acceleration
 */
float LIS3DH::convertReadingToAccel(int16_t reading) {
	// Take the 16 bit integer we read, cast it as a float, normalized it to half the max reading
	float finalAccel = ((float)(reading)) / ((float)32767) * ((float)(2));
	return finalAccel;
}

/**
 * A simple delay function
 * @param num number of milliseconds to delay (approximately)
 */
void LIS3DH::delay(int num)
{
	int counter=0;
	for(int i=0;i<num*1000;i++)
	{
		counter++;
	}
}
