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
#include "syscalls.hxx"
#include "hostConsole.hxx"

using namespace std::literals::string_view_literals;
using semihosting::types::OpenMode;
using semihosting::types::SemihostingResult;
using semihosting::host::console::host;

constexpr static int32_t stdinFD{1};
constexpr static int32_t stdoutFD{3};
constexpr static size_t featuresMinLength{4U};
constexpr static std::array<char, 4> featuresMagic{{'S', 'H', 'F', 'B'}};

// Compute the length of a (hopefully) nul terminated string stored in `storage`, bounded on the storage size
template<size_t N> [[nodiscard]] static size_t strlen(const std::array<char, N> &storage) noexcept
{
	for (size_t index = 0; index < N; ++index)
	{
		if (storage[index] == '\0')
			return index;
	}
	return N;
}

// Test the retrieval of the command line from GDB
[[nodiscard]] static bool testReadCommandLine() noexcept
{
	host.info("Testing SYS_GET_CMDLINE"sv);
	std::array<char, 80> commandLineBuffer{};
	if (semihosting::readCommandLine({commandLineBuffer}) != SemihostingResult::success)
	{
		host.error("SYS_GET_CMDLINE failed"sv);
		return false;
	}
	std::string_view commandLine{commandLineBuffer.data(), strlen(commandLineBuffer)};
	const auto result{commandLine == " how meow brown cow"sv};
	if (result)
		host.notice("SYS_GET_CMDLINE success"sv);
	else
		host.error("Wrong command line string value: '"sv, commandLine, "'"sv);
	return result;
}

// Test to verify that we can close and open the host TTY handles properly
[[nodiscard]] static bool testConsoleHandles() noexcept
{
	host.info("Testing SYS_CLOSE on special name ", "':tt'"sv);
	if (!host.closeConsole())
	{
		host.error("SYS_CLOSE failed");
		return false;
	}
	// As per the comment in the fail case, use the fact we don't invalidate our handles to tell it what we're doing
	host.notice("SYS_CLOSE success"sv);
	host.info("Testing SYS_OPEN on special name "sv, "':tt'"sv);
	if (!host.openConsole())
	{
		// Nothing we can do here, as we just failed to re-open our only means of talking with the host
		// Given we don't invalidate our handles in .closeConsole() though, we can try to tell the host
		// that this failed
		host.error("SYS_OPEN failed"sv);
		return false;
	}
	host.notice("SYS_OPEN success"sv);
	host.info("SYS_OPEN returned FDs "sv, host.stdinFD(), ", and "sv, host.stdoutFD(), " for console I/O"sv);
	// Check and make sure that the handles returned are actually for the right console parts
	if (host.stdinFD() != stdinFD || host.stdoutFD() != stdoutFD)
		host.error("Improper I/O handles returned for special name "sv, "':tt'"sv);
	return host.stdinFD() == stdinFD && host.stdoutFD() == stdoutFD;
}

// Test to validate the special file `:semihosting-features` can be opened, read,
// and the contents make sense
[[nodiscard]] static bool testSemihostingFeatures() noexcept
{
	host.info("Testing access to special name ':semihosting-features'"sv);
	host.info("Trying SYS_OPEN on "sv, "':semihosting-features'"sv);
	// Start by opening the special file
	const auto featuresFD{semihosting::open(":semihosting-features"sv, OpenMode::read)};
	if (featuresFD <= 0)
	{
		host.error("SYS_OPEN failed"sv);
		return false;
	}
	host.notice("SYS_OPEN success"sv);
	// Now try to check how long the file is and make sure it's at least the minimum lenght
	host.info("Trying SYS_FLEN on "sv, "':semihosting-features'"sv);
	const auto fileLength{semihosting::fileLength(featuresFD)};
	if (fileLength < 0 || static_cast<size_t>(fileLength) < featuresMinLength)
	{
		host.error("SYS_FLEN failed, file "sv, fileLength == -1 ? "length couldn't be determined"sv : "too short"sv);
		if (semihosting::close(featuresFD) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("SYS_FLEN success"sv);
	// Try to read out the magic number for the file
	std::array<char, 4> magic{};
	host.info("Trying SYS_READ on "sv, "':semihosting-features'"sv);
	if (semihosting::read(featuresFD, substrate::span{magic}) != 0)
	{
		host.error("SYS_READ failed"sv);
		if (semihosting::close(featuresFD) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("SYS_READ success"sv);
	if (magic != featuresMagic)
	{
		host.error("Invalid "sv, "':semihosting-features'"sv, " magic number"sv);
		if (semihosting::close(featuresFD) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}

	host.info("Trying SYS_CLOSE on "sv, "':semihosting-features'"sv);
	if (semihosting::close(featuresFD) != SemihostingResult::success)
	{
		host.error("SYS_CLOSE failed");
		return false;
	};
	host.notice("SYS_CLOSE success"sv);
	host.notice("Access to "sv, "':semihosting-features'"sv, " successful"sv);
	return true;
}

[[nodiscard]] static bool testSemihosting() noexcept
{
	return testReadCommandLine() &&
		testConsoleHandles() &&
		testSemihostingFeatures();
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
