// SPDX-License-Identifier: BSD-3-Clause
/* This file is part of the Black Magic Probe test firmware archive.
 *
 * Copyright (C) 2023 Rachel Mant <git@dragonmux.network>
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

#include <array>
#include <string_view>
#include <arm_acle.h>
#include "syscalls.hxx"
#include "hostConsole.hxx"

using namespace std::literals::string_view_literals;
using semihosting::types::SemihostingResult;
using semihosting::host::console::host;

[[nodiscard]] static bool testReadCommandLine() noexcept
{
	host.info("Testing SYS_GET_CMDLINE"sv);
	std::array<char, 80> commandLineBuffer{};
	if (semihosting::readCommandLine({commandLineBuffer}) != SemihostingResult::success)
	{
		host.error("SYS_GET_CMDLINE failed"sv);
		return false;
	}
	std::string_view commandLine{commandLineBuffer.data(), commandLineBuffer.size()};
	const auto result{commandLine == "how meow brown cow"sv};
	if (result)
		host.notice("SYS_GET_CMDLINE success"sv);
	else
		host.error("Wrong command line string value: "sv, commandLine);
	return result;
}

[[nodiscard]] static bool testSemihosting() noexcept
{
	return testReadCommandLine();
}

int main(int, char **)
{
	// Try to open the host's console interface, and if that fails, return as there's nothing more can be done
	if (!host.openConsole())
		return 1;
	host.notice("Testing semihosting support"sv);
	if (testSemihosting())
		host.notice("Test complete (success)"sv);
	else
		host.error("Test failed"sv);

	// Try to close the host's console interface, and if that fails return so the test restarts
	if (!host.closeConsole())
	{
		// NB: we close first the stdin side then the stdout, so in this case the stdout side should still
		// be valid and we can try to write some sort of error string
		host.error("Failed to shut down host console interface");
		return 1;
	}

	while (true)
		__asm__("wfi");
	return 0;
}
