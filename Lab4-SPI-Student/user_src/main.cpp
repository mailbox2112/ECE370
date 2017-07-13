#include "em_device.h"
#include "em_chip.h"

#include <stdint.h>
#include <stdbool.h>
#include <cmath>

#include "LIS3DH.h"

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	/**
	 * Chip errata
	 */
	CHIP_Init();

	/**
	 * Instantiate a copy of the accelerometer
	 * driver. You CANNOT call NEW! This is not
	 * a desktop PC.
	 */
	LIS3DH accel;

	/**
	 * Setup for default operation.
	 * 2MHz operation SPI bus;
	 * Use USART1 and Pin Location #1
	 */
	accel.initialize();

	/**
	 * Perform a self-test. This will attempt to
	 * read a fixed register on the LIS3DhH. If the
	 * self-test fails, then your program should not proceed.
	 */
	bool selfTest=accel.selfTest();
	if(!selfTest)
	{
		while(1){};
	}

	/**
	 * Once you've passed the self test. Attempt to read
	 * the acceleration from each axis. The magnitude of
	 * all accelerations should be close to 1g. Your result
	 * should not differ by more than 5%.
	 */
	float x=accel.getXAcceleration();
	float y=accel.getYAcceleration();
	float z=accel.getZAcceleration();

	/**
	 * Use illegal math functions to calculate a square root.
	 * This is possibly OK since this is a high-performance
	 * microprocessor and we only perform this operation once.
	 */
	float mag = sqrt(x*x+y*y+z*z);

	while(true)
	{
		/**
		 * Dummy instruction so the compiler
		 * doesn't optimize this loop away and
		 * we restart at main()
		 */
		int counter=0;

	}
}

