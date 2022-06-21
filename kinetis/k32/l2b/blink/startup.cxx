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

#include <cstdint>
#include <array>
#include "platform.hxx"
#include "k32l2b.hxx"

void irqReset() noexcept;
void irqNMI() noexcept;
void irqEmptyDef() noexcept;
[[gnu::naked]] void irqHardFault() noexcept;

extern const uint32_t stackTop;
extern const uint32_t endText;
extern uint32_t beginData;
extern const uint32_t endData;
extern uint32_t beginBSS;
extern const uint32_t endBSS;

using ctorFuncs_t = void (*)();
extern const ctorFuncs_t beginCtors, endCtors;

using irqFunction_t = void (*)();

struct nvicTable_t final
{
	const void *stackTop;
	std::array<irqFunction_t, 47> vectorTable;
};

struct flashConfig_t final
{
	uint64_t backdoorKey;
	uint32_t protection;
	uint8_t securityByte;
	uint8_t optionByte;
	uint16_t reserved{0};
};

static_assert(sizeof(flashConfig_t) == 16U);

[[gnu::section(".nvic_table"), gnu::used]] static const nvicTable_t nvicTable
{
	&stackTop,
	{
		irqReset, /* Reset handler */
		irqNMI, /* NMI handler */
		irqHardFault, /* Hard Fault handler */

		/* Configurable priority handlers */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		irqEmptyDef, /* SV Call */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		irqEmptyDef, /* Pending SV */
		irqEmptyDef, /* Sys Tick */

		/* Peripheral handlers */
		irqEmptyDef, /* DMA Channel 1 */
		irqEmptyDef, /* DMA Channel 2 */
		irqEmptyDef, /* DMA Channel 3 */
		irqEmptyDef, /* DMA Channel 4 */
		nullptr, /* Reserved */
		irqEmptyDef, /* FTFA */
		irqEmptyDef, /* PMC */
		irqEmptyDef, /* LLWU */
		irqEmptyDef, /* I2C0 */
		irqEmptyDef, /* I2C1 */
		irqEmptyDef, /* SPI0 */
		irqEmptyDef, /* SPI1 */
		irqEmptyDef, /* LPUART0 */
		irqEmptyDef, /* LPUART1 */
		irqEmptyDef, /* UART2 / FlexIO */
		irqEmptyDef, /* ADC0 */
		irqEmptyDef, /* CMP0 */
		irqEmptyDef, /* TPM0 */
		irqEmptyDef, /* TPM1 */
		irqEmptyDef, /* TPM2 */
		irqEmptyDef, /* RTC Alarm */
		irqEmptyDef, /* RTC Seconds */
		irqEmptyDef, /* PIT */
		nullptr, /* Reserved */
		irqEmptyDef, /* USB */
		irqEmptyDef, /* DAC0 */
		nullptr, /* Reserved */
		nullptr, /* Reserved */
		irqEmptyDef, /* LPTMR0 */
		irqEmptyDef, /* LCD */
		irqEmptyDef, /* Pin Detect Port A */
		irqEmptyDef, /* Pin Detect Port C & D */
	}
};

[[gnu::section(".flash_config"), gnu::used]] static const flashConfig_t flashConfig
{
	// "Backdoor" access key
	UINT64_C(0xFEED'ACA7'DEAD'BEEF),
	// Make sure all Flash regions are unprotected
	UINT32_C(0xFF'FF'FF'FF),
	// Flash security byte
	// Backdoor enabled, mass erase enabled, factory access granted, and Flash is unsecured
	0b10'11'11'10,
	// Flash non-volatile option byte
	// Boot from Flash, using Fast Init, leave nRST enabled, no NMI, boot configured by FOPT
	// and clock divider set to /1 from the start.
	0b11'0'0'1'0'00,
};

void irqReset() noexcept
{
	while (true)
	{
		auto *src{&endText};
		for (auto *dst{&beginData}; dst < &endData; ++dst, ++src)
			*dst = *src;
		for (auto *dst{&beginBSS}; dst < &endBSS; ++dst)
			*dst = 0;
		for (auto *ctor{&beginCtors}; ctor != &endCtors; ++ctor)
			(*ctor)();

		run();
	}
}

void irqNMI() noexcept
{
	while (true)
		continue;
}

void irqHardFault() noexcept
{
	/* Get some information about the fault for the debugger.. */
	__asm__(R"(
		mov     r0, #4
		mov     r1, lr
		tst     r0, r1
		beq     _MSP
		mrs     r0, psp
		b       _HALT
	_MSP:
		mrs     r0, msp
	_HALT:
		ldr     r1, [r0, #0x00] /* r0 */
		ldr     r2, [r0, #0x04] /* r1 */
		ldr     r3, [r0, #0x08] /* r2 */
		ldr     r4, [r0, #0x0C] /* r3 */
		ldr     r5, [r0, #0x10] /* r12 */
		ldr     r6, [r0, #0x14] /* lr */
		ldr     r7, [r0, #0x1C] /* xpsr */
		mov     r8, r7
		ldr     r7, [r0, #0x18] /* pc */
		bkpt    #0
	_DEADLOOP:
		b		_DEADLOOP
	)");
	/* The lowest 8 bits of r8 (xpsr) contain which handler triggered this, if there is a signal handler frame before this. */
}

void irqEmptyDef() noexcept
{
	while (true)
		continue;
}
