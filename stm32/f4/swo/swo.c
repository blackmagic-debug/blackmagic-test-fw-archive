// SPDX-License-Identifier: BSD-3-Clause
/* This file is part of the Black Magic Probe test firmware archive.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/scs.h>
#include <libopencm3/cm3/tpiu.h>
#include <libopencm3/cm3/dwt.h>
#include <libopencm3/cm3/itm.h>

#define SWO_BAUDRATE 115200U
#define ARM_LAR_ACCESS_ENABLE 0xc5acce55U

static void clock_setup(void)
{
	/* Set processor to use the HSI at 84MHz */
	rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);

	/* Enable GPIOA clock so we can show activity */
	rcc_periph_clock_enable(RCC_GPIOA);
}

static void gpio_setup(void)
{
	/* Set PA5 to 'output push-pull'. */
	gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
}

static void swo_setup(void)
{
	/* Enable tracing in DEMCR */
	SCS_DEMCR |= SCS_DEMCR_TRCENA;

	/* Get the active clock frequency of the core and use that to calculate a divisor */
	const uint32_t clock_frequency = rcc_ahb_frequency;
	const uint32_t divisor = (clock_frequency / SWO_BAUDRATE) - 1U;
	/* And configure the TPIU for 1-bit async trace (SWO) in Manchester coding */
	TPIU_LAR = ARM_LAR_ACCESS_ENABLE;
	TPIU_CSPSR = 1U; /* 1-bit mode */
	TPIU_ACPR = divisor;
	TPIU_SPPR = TPIU_SPPR_ASYNC_MANCHESTER;
	/* Ensure that TPIU framing is off */
	TPIU_FFCR &= ~TPIU_FFCR_ENFCONT;

	/* Configure the DWT to provide the sync source for the ITM */
	DWT_LAR = ARM_LAR_ACCESS_ENABLE;
	DWT_CTRL |= 0x000003feU;
	/* Enable access to the ITM registers and configure tracing output from the first stimulus port */
	ITM_LAR = ARM_LAR_ACCESS_ENABLE;
	/* User-level access to the first 8 ports */
	ITM_TPR = 0x0000000fU;
	ITM_TCR = ITM_TCR_ITMENA | ITM_TCR_SYNCENA | ITM_TCR_TXENA | ITM_TCR_SWOENA | (1U << 16U);
	ITM_TER[0] = 1U;

	/* Now tell the DBGMCU that we want trace enabled and mapped as SWO */
	DBGMCU_CR &= ~DBGMCU_CR_TRACE_MODE_MASK;
	DBGMCU_CR |= DBGMCU_CR_TRACE_IOEN | DBGMCU_CR_TRACE_MODE_ASYNC;
}

static void itm_write(const uint8_t port, const char value)
{
	/* Wait for the port to become ready */
	while ((ITM_STIM8(port) & 1U) == 0U)
		continue;
	/* Write the new character to the port */
	ITM_STIM8(port) = (uint8_t)value;
}

int main(void)
{
	/* Bring the clocks and peripherals needed up */
	clock_setup();
	gpio_setup();
	swo_setup();

	while (true)
	{
		/* Write A-Z out via the ITM, followed by \r\n */
		for (char value = 'A'; value <= 'Z'; ++value)
			itm_write(0U, value);
		itm_write(0U, '\r');
		itm_write(0U, '\n');
		gpio_toggle(GPIOA, GPIO5);
	}

	return 0;
}
