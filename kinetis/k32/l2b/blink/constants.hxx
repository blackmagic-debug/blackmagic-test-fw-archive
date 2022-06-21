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

#ifndef CONSTANTS_HXX
#define CONSTANTS_HXX

#include <cstdint>

namespace vals
{
	namespace sim
	{
		constexpr static uint32_t clockGateCtrl1PortA{0x00000200U};
		constexpr static uint32_t clockGateCtrl1PortB{0x00000400U};
		constexpr static uint32_t clockGateCtrl1PortC{0x00000800U};
		constexpr static uint32_t clockGateCtrl1PortD{0x00001000U};
		constexpr static uint32_t clockGateCtrl1PortE{0x00002000U};

		constexpr static uint32_t copCtrlClkSelBus{0x000000C0U};
		constexpr static uint32_t copCtrlClkSelOscEr{0x00000080U};
		constexpr static uint32_t copCtrlClkSelMCGIR{0x00000040U};
		constexpr static uint32_t copCtrlClkSelLPO{0x00000000U};
		constexpr static uint32_t copCtrlDebugEn{0x00000020U};
		constexpr static uint32_t copCtrlStopEn{0x00000010U};
		constexpr static uint32_t copCtrlTimeoutHigh{0x0000000CU};
		constexpr static uint32_t copCtrlTimeoutMedium{0x00000008U};
		constexpr static uint32_t copCtrlTimeoutLow{0x00000004U};
		constexpr static uint32_t copCtrlDisabled{0x00000000U};
		constexpr static uint32_t copCtrlTimeoutLong{0x00000002U};
		constexpr static uint32_t copCtrlTimeoutShort{0x00000000U};
		constexpr static uint32_t copCtrlModeWindowed{0x00000001U};
		constexpr static uint32_t copCtrlModeNormal{0x00000000U};
	} // namespace sim
} // namespace vals

#endif /*CONSTANTS_HXX*/
