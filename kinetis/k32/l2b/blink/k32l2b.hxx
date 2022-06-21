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

#ifndef K32L2B_HXX
#define K32L2B_HXX

#include <cstdint>
#include <array>

namespace k32l2b
{
	struct sim_t final
	{
		volatile uint32_t options1;
		volatile uint32_t options1Cfg;
		std::array<const volatile uint32_t, 1023> reserved1;
		volatile uint32_t options2;
		const volatile uint32_t reserved2;
		volatile uint32_t options4;
		volatile uint32_t options5;
		const volatile uint32_t reserved3;
		volatile uint32_t options7;
		std::array<const volatile uint32_t, 2> reserved4;
		const volatile uint32_t deviceIdent;
		std::array<const volatile uint32_t, 3> reserved5;
		std::array<volatile uint32_t, 4> clockGateCtrl;
		volatile uint32_t clockDiv1;
		const volatile uint32_t reserved6;
		std::array<volatile uint32_t, 2> flashCfg;
		const volatile uint32_t reserved7;
		std::array<const volatile uint32_t, 3> uniqueID;
		std::array<const volatile uint32_t, 39> reserved8;
		volatile uint32_t copCtrl;
		volatile uint32_t serviceCOP;
	};

	struct gpio_t final
	{
		volatile uint32_t dataOut;
		volatile uint32_t bitSet;
		volatile uint32_t bitClear;
		volatile uint32_t bitToggle;
		const volatile uint32_t dataIn;
		volatile uint32_t dir;
	};

	constexpr static uintptr_t simBase{0x40047000U};

	constexpr static uintptr_t gpioABase{0x400FF000U};
	constexpr static uintptr_t gpioBBase{0x400FF040U};
	constexpr static uintptr_t gpioCBase{0x400FF080U};
	constexpr static uintptr_t gpioDBase{0x400FF0C0U};
	constexpr static uintptr_t gpioEBase{0x400FF100U};

	constexpr static uintptr_t fgpioABase{0xF8000000U};
	constexpr static uintptr_t fgpioBBase{0xF8000040U};
	constexpr static uintptr_t fgpioCBase{0xF8000080U};
	constexpr static uintptr_t fgpioDBase{0xF80000C0U};
	constexpr static uintptr_t fgpioEBase{0xF8000100U};
} // namespace k32l2b

// NOLINTBEGIN(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static auto &sim{*reinterpret_cast<k32l2b::sim_t *>(k32l2b::simBase)};

static auto &gpioA{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::gpioABase)};
static auto &gpioB{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::gpioBBase)};
static auto &gpioC{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::gpioCBase)};
static auto &gpioD{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::gpioDBase)};
static auto &gpioE{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::gpioEBase)};

static auto &fgpioA{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::fgpioABase)};
static auto &fgpioB{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::fgpioBBase)};
static auto &fgpioC{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::fgpioCBase)};
static auto &fgpioD{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::fgpioDBase)};
static auto &fgpioE{*reinterpret_cast<k32l2b::gpio_t *>(k32l2b::fgpioEBase)};
// NOLINTEND(cppcoreguidelines-pro-type-reinterpret-cast)
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

#endif /*K32L2B_HXX*/
