// SPDX-License-Identifier: BSD-3-Clause
/* This file is part of the black magic probe test firmware archive.
 *
 * Copyright (C) 2022 Rachel Mant <git@dragonmux.network>
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

#include <cstddef>
#include "platform.hxx"
#include "constants.hxx"
#include "k32l2b.hxx"

static void clockSetup()
{
	// Disable the WDT
	sim.copCtrl = vals::sim::copCtrlDisabled;
	// Switch into HIRC mode as we want to go LIRC 8MHz -> 2MHz and this must be done indirectly.
	mcg.miscCtrl = vals::mcg::miscCtrlHIRClkEnabled | vals::mcg::miscCtrlLIRClkDiv1;
	while ((mcg.status & vals::mcg::statusClkModeMask) != vals::mcg::statusClkModeHIRC)
		mcg.ctrl1 = vals::mcg::ctrl1ClkModeHIRC | vals::mcg::ctrl1IRefClkEnabled;
	// Configure the clock as the 2MHz low-precision internal reference
	mcg.ctrl2 = vals::mcg::ctrl2LIRC2MHz;
	mcg.statusCtrl = vals::mcg::statusCtrlLIRClkDiv1;
	mcg.miscCtrl = vals::mcg::miscCtrlHIRClkEnabled | vals::mcg::miscCtrlLIRClkDiv1;
	while ((mcg.status & vals::mcg::statusClkModeMask) != vals::mcg::statusClkModeLIRC)
		mcg.ctrl1 = vals::mcg::ctrl1ClkModeLIRC | vals::mcg::ctrl1IRefClkEnabled |
			vals::mcg::ctrl1IRefStopEnabled;
	// And clean up.
	mcg.miscCtrl = vals::mcg::miscCtrlHIRClkDisabled | vals::mcg::miscCtrlLIRClkDiv1;
	// Enable PortC clocking
	sim.clockGateCtrl[1] |= vals::sim::clockGateCtrl1PortC;
}

static void gpioSetup()
{
	// Make sure PortC[1..3] are set high to keep the LEDs off
	fgpioC.bitSet = 7;
	// Set PortC[1..3] as outputs
	fgpioC.dir |= 7;
}

void run()
{
	clockSetup();
	gpioSetup();

	while (true) {
		// Toggle PC1 on and off which makes the red part of the RGB LED blink.
		fgpioC.bitToggle = 1;
		for (volatile size_t i = 0; i < 1000000U; ++i)
			continue;
	}
}
