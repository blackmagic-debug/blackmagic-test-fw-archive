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
#include <substrate/span>
#include <substrate/index_sequence>
#include <frozen/unordered_map.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include "syscalls.hxx"
#include "hostConsole.hxx"

using namespace std::literals::string_view_literals;
using semihosting::types::OpenMode;
using semihosting::types::HeapInfoBlock;
using semihosting::types::SemihostingResult;
using semihosting::types::FileIOErrno;
using semihosting::types::ExitReason;
using semihosting::host::console::host;

constexpr static int32_t stdinFD{1};
constexpr static int32_t stdoutFD{2};
constexpr static size_t featuresLength{5U};
constexpr static std::array<char, 4> featuresMagic{{'S', 'H', 'F', 'B'}};
constexpr static auto alphabet{"abcdefghijklmnopqrstuvwxyz\r\n"sv};

constexpr static auto testFileA{"semihosting-test.a"sv};
constexpr static auto testFileB{"semihosting-test.b"sv};
constexpr static auto testTempFileName{"tempAK.tmp"sv};

constexpr static auto fileIOErrno
{
	frozen::make_unordered_map<FileIOErrno, std::string_view>
	({
		{FileIOErrno::notPermitted, "FILEIO_EPERM (Operation not permitted)"sv},
		{FileIOErrno::noSuchEntity, "FILEIO_ENOENT (No such file or directory)"sv},
		{FileIOErrno::syscallInterrupted, "FILEIO_EINTR (Interrupted system call)"sv},
		{FileIOErrno::ioError, "FILEIO_EIO (I/O error)"sv},
		{FileIOErrno::badFD, "FILEIO_EBADF (Bad file number)"sv},
		{FileIOErrno::accessError, "FILEIO_EACCESS (Permission denied)"sv},
		{FileIOErrno::addressFault, "FILEIO_EFAULT (Bad address)"sv},
		{FileIOErrno::busy, "FILEIO_EBUSY (Device or resource busy)"sv},
		{FileIOErrno::alreadyExists, "FILEIO_EEXIST (File already exists)"sv},
		{FileIOErrno::noSuchDevice, "FILEIO_ENODEV (No such device)"sv},
		{FileIOErrno::notADir, "FILEIO_ENOTDIR (Not a directory)"sv},
		{FileIOErrno::isADir, "FILEIO_EISDIR (Is a directory)"sv},
		{FileIOErrno::argumentInvalid, "FILEIO_EINVAL (Invalid argument)"sv},
		{FileIOErrno::fileTableFull, "FILEIO_ENFILE (File table overflow)"sv},
		{FileIOErrno::tooManyOpenFiles, "FILEIO_EMFILE (Too many open files)"sv},
		{FileIOErrno::fileTooLarge, "FILENO_EFBIG (File too large)"sv},
		{FileIOErrno::outOfSpace, "FILENO_ENOSPC (No space left on device)"sv},
		{FileIOErrno::illegalSeek, "FILENO_ESPIPE (Illegal seek)"sv},
		{FileIOErrno::fsReadOnly, "FILENO_EROFS (Read-only file system"sv},
		{FileIOErrno::syscallInvalid, "FILENO_ENOSYS (Invalid system call number)"sv},
		{FileIOErrno::fileNameTooLong, "FILENO_ENAMETOOLONG (File name too long)"sv},
	})
};

// NB: This suite is incomplete in that it does *not* test SYS_READ and SYS_READC with stdin
// This is because we cannot write a reproducible easy to use test. We assume that SYS_READC
// works (this is the only syscall we're fully unable to reproducibly test).

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
	host.warn("-> "sv, __func__);
	host.info("Testing SYS_GET_CMDLINE"sv);
	std::array<char, 80> commandLineBuffer{};
	if (semihosting::readCommandLine({commandLineBuffer}) != SemihostingResult::success)
	{
		host.error("SYS_GET_CMDLINE failed"sv);
		return false;
	}
	const std::string_view commandLine{commandLineBuffer.data(), strlen(commandLineBuffer)};
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
	host.warn("-> "sv, __func__);
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
	// Check that the handles we got back point to an actual console
	host.info("Testing SYS_ISTTY on "sv, "':tt'"sv, " handles"sv);
	if (semihosting::isTTY(host.stdinFD()) != 1 || semihosting::isTTY(host.stdoutFD()) != 1)
	{
		host.error("SYS_ISTTY failed"sv);
		return false;
	}
	host.notice("SYS_ISTTY success");
	return host.stdinFD() == stdinFD && host.stdoutFD() == stdoutFD;
}

// Test to validate the special file `:semihosting-features` can be opened, read,
// and the contents make sense
[[nodiscard]] static bool testSemihostingFeatures() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Testing access to special name ':semihosting-features'"sv);
	host.info("Trying SYS_OPEN on "sv, "':semihosting-features'"sv);
	// Start by opening the special file
	const auto featuresFD{semihosting::open(":semihosting-features"sv, OpenMode::read)};
	if (featuresFD <= 0)
	{
		host.error("SYS_OPEN failed: errno = "sv, semihosting::lastErrno());
		return false;
	}
	host.notice("SYS_OPEN success"sv);
	// Now try to check how long the file is and make sure it's the correct length for BMD
	host.info("Trying SYS_FLEN on "sv, "':semihosting-features'"sv);
	const auto fileLength{semihosting::fileLength(featuresFD)};
	if (static_cast<size_t>(fileLength) != featuresLength)
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
	// Validate that the read features "file" magic number is valid
	if (magic != featuresMagic)
	{
		host.error("Invalid "sv, "':semihosting-features'"sv, " magic number"sv);
		if (semihosting::close(featuresFD) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("':semihosting-features'"sv, " file magic OK"sv);

	host.info("Checking feature byte"sv);
	// Read out the next byte, which should be the supported features byte
	uint8_t supportedFeatures{};
	if (semihosting::read(featuresFD, {&supportedFeatures, 1U}) != 0)
	{
		host.error("SYS_READ failed"sv);
		if (semihosting::close(featuresFD) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("SYS_READ success"sv);
	// Validate the value of the byte matches what we know BMD supports
	// BMD supports extended exit and stdout+stderr through `:tt`
	if (supportedFeatures != 3U)
	{
		host.error("Supported features byte has incorrect value "sv, supportedFeatures);
		return false;
	}
	host.notice("Supported features reported correctly"sv);

	host.info("Trying SYS_CLOSE on "sv, "':semihosting-features'"sv);
	// Now try to close the "file" handle to make sure nothing bad happens in the emulation
	if (semihosting::close(featuresFD) != SemihostingResult::success)
	{
		host.error("SYS_CLOSE failed"sv);
		return false;
	};
	host.notice("SYS_CLOSE success"sv);
	host.notice("Access to "sv, "':semihosting-features'"sv, " successful"sv);
	return true;
}

[[nodiscard]] static bool testConsoleWrite() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Trying to SYS_WRITE to stdout"sv);
	// Try writing the test string to stdout using a {ptr, length}
	if (semihosting::write(host.stdoutFD(), substrate::span{alphabet}) != 0)
	{
		host.error("SYS_WRITE failed"sv);
		return false;
	}
	host.notice("SYS_WRITE success"sv);

	host.info("Trying to SYS_WRITE0 to stdout"sv);
	// Try writing the test string to stdout as a nul-terminated string
	if (semihosting::write(alphabet.data()) != SemihostingResult::success)
	{
		host.error("SYS_WRITE0 failed"sv);
		return false;
	}
	host.notice("SYS_WRITE0 success"sv);

	host.info("Trying to SYS_WRITEC to stdout"sv);
	// Try writing the test string to stdout one character at a time
	for (const auto chr : alphabet)
	{
		if (semihosting::writeChar(chr) != SemihostingResult::success)
		{
			host.writeln();
			host.error("SYS_WRITEC failed"sv);
			return false;
		}
	}
	host.notice("SYS_WRITEC success"sv);
	return true;
}

[[nodiscard]] static bool testFileIO() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Trying SYS_OPEN on "sv, testFileA);
	// Try to open file A in write mode - use binary mode to ensure no translation of newlines
	int32_t fd{semihosting::open(testFileA, OpenMode::writeBinary)};
	if (fd <= 0)
	{
		host.error("SYS_OPEN failed: errno = "sv, semihosting::lastErrno());
		return false;
	}
	host.notice("SYS_OPEN success"sv);

	host.info("Trying to SYS_WRITE to file"sv);
	// Now try to write the test data to the resulting file descriptor
	if (semihosting::write(fd, substrate::span{alphabet}) != 0)
	{
		host.error("SYS_WRITE failed"sv);
		return false;
	}
	host.notice("SYS_WRITE success"sv);

	host.info("Trying to SYS_CLOSE file"sv);
	// Try to close the test file and write some more data to it (this write should fail!)
	if (semihosting::close(fd) != SemihostingResult::success ||
		semihosting::write(fd, substrate::span{alphabet}) == 0)
	{
		host.error("SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("SYS_CLOSE success"sv);

	host.info("Trying to SYS_RENAME the test file"sv);
	if (semihosting::rename(testFileA, testFileB) != SemihostingResult::success)
	{
		host.error("SYS_RENAME failed"sv);
		return false;
	}
	host.notice("SYS_RENAME success"sv);

	host.info("Trying SYS_OPEN on "sv, testFileB);
	// Try to open file B in read mode - use binary mode to ensure no translation of newlines
	fd = semihosting::open(testFileB, OpenMode::readBinary);
	if (fd <= 0)
	{
		host.error("SYS_OPEN failed: errno = "sv, semihosting::lastErrno());
		return false;
	}
	host.notice("SYS_OPEN success"sv);

	host.info("Trying SYS_FLEN on file"sv);
	if (static_cast<size_t>(semihosting::fileLength(fd)) != alphabet.length())
	{
		host.error("SYS_FLEN failed"sv);
		return false;
	}
	host.notice("SYS_FLEN success"sv);

	std::array<char, alphabet.length()> buffer{};
	host.info("Trying to SYS_READ from file"sv);
	// Now try to write the test data to the resulting file descriptor
	if (semihosting::read(fd, substrate::span{buffer}) != 0)
	{
		host.error("SYS_READ failed"sv);
		return false;
	}
	host.notice("SYS_READ success"sv);
	// Check that the data read back is the data we asked to have written
	if (std::string_view{buffer.data(), buffer.size()} != alphabet)
	{
		host.error("Data read fails to match data written"sv);
		return false;
	}
	host.notice("Data read back correctly"sv);

	host.info("Trying SYS_ISTTY on file"sv);
	// Check that the file is not a TTY, but is instead a real file on the filesystem
	if (semihosting::isTTY(fd) != 0)
	{
		host.error("SYS_ISTTY failed"sv);
		return false;
	}
	host.notice("SYS_ISTTY success"sv);

	host.info("Trying to SYS_CLOSE file"sv);
	// Try to close the test file and read some more data from it (this read should fail!)
	if (semihosting::close(fd) != SemihostingResult::success ||
		semihosting::read(fd, substrate::span{buffer}) == 0)
	{
		host.error("SYS_CLOSE failed"sv);
		return false;
	}
	host.notice("SYS_CLOSE success"sv);

	host.info("Trying to SYS_REMOVE the test file"sv);
	// Finally try and remove the file to clean up and to test the call works
	if (semihosting::remove(testFileB) != SemihostingResult::success)
	{
		host.error("SYS_REMOVE failed"sv);
		return false;
	}
	host.notice("SYS_REMOVE success"sv);
	return true;
}

[[nodiscard]] static bool testIsError() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Testing SYS_ISERROR on the first 100 possible error codes"sv);
	// Loop through the first 100 possible error codes from SYS_ERRNO
	for (const auto code : substrate::indexSequence_t{100U})
	{
		// Look the code up in the map
		const auto codeMapping{fileIOErrno.find(static_cast<FileIOErrno>(code))};
		const auto inMap{codeMapping != fileIOErrno.end()};
		// Make the semihosting request (our syscalls layer turns it into a bool for us)
		const auto isError{semihosting::isError(static_cast<int32_t>(code))};
		// Check that the error state matches what's expected
		if (isError != inMap)
		{
			host.error("SYS_ISERROR failed - host considers "sv, code, " to"sv, isError ? ""sv : " not"sv,
				" be an error when it should"sv, inMap ? ""sv : " not"sv);
			return false;
		}
		// If it matches and it is an error (in the map), display the description in the success notice
		if (inMap)
			host.notice("SYS_ISERROR success for "sv, codeMapping->second);
		// Otherwise display that it was the success "error" code, which is not considered an error
		else if (code == 0)
			host.notice("SYS_ISERROR success for "sv, "FILEIO_SUCCESS (no error)"sv);
	}
	host.notice("SYS_ISERROR success"sv);
	return true;
}

[[nodiscard]] static bool testHeapInfo() noexcept
{
	host.warn("-> "sv, __func__);
	// Initialise the HeapInfoBlock to something insane and obvious so we can clearly tell if the call worked
	HeapInfoBlock infoBlock
	{
		0xbada110cU,
		0xca7f00d5U,
		0xfeedaca7U,
		0xbeefcafeU,
	};
	// Make the syscall to get the actual block from the host
	host.info("Testing SYS_HEAPINFO"sv);
	semihosting::heapInfo(infoBlock);
	if (infoBlock.heapBase == 0xbada110cU || infoBlock.heapLimit == 0xca7f00d5U ||
		infoBlock.stackBase == 0xfeedaca7U || infoBlock.stackLimit == 0xbeefcafeU)
	{
		host.error("SYS_HEAPINFO failed"sv);
		return false;
	}
	host.notice("SYS_HEAPINFO success"sv);
	host.notice("Heap base: "sv, infoBlock.heapBase, ", limit: "sv, infoBlock.heapLimit);
	host.notice("Stack base: "sv, infoBlock.stackBase, ", limit: "sv, infoBlock.stackLimit);
	return true;
}

[[nodiscard]] static bool testErrno() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Setting up and testing SYS_ERRNO"sv);
	// Try to open file B in read mode - use binary mode to ensure no translation of newlines
	// NB: we want and expect this to fail as the file should not exist after the previous File I/O test step
	const auto fd{semihosting::open(testFileB, OpenMode::readBinary)};
	if (fd > 0)
	{
		host.error("Setup failed, file "sv, testFileB, " unexpectedly exists"sv);
		if (semihosting::close(fd) != SemihostingResult::success)
			host.error("Additionally SYS_CLOSE failed"sv);
		return false;
	}
	// Now try to read back the error associated with that open()
	if (const auto lastError{semihosting::lastErrno()}; lastError != FileIOErrno::noSuchEntity)
	{
		host.error("SYS_ERRNO failed giving "sv, lastError, " - expected "sv, FileIOErrno::noSuchEntity);
		return false;
	}
	host.notice("First SYS_ERRNO successful, trying a second"sv);
	// If all's gone well, errno should now be 0 (success) here, so let's try to get it back
	if (const auto lastError{semihosting::lastErrno()}; lastError != FileIOErrno::success)
	{
		host.error("SYS_ERRNO failed giving "sv, lastError, " - expected "sv, FileIOErrno::success);
		return false;
	}
	host.notice("SYS_ERRNO success"sv);
	return true;
}

[[nodiscard]] static bool testTiming() noexcept
{
	host.warn("-> "sv, __func__);
	// BMD does not implement either of these calls, so we expect both to fail
	host.info("Testing SYS_TICKFREQ"sv);
	if (semihosting::tickFrequency() != -1)
	{
		host.error("SYS_TICKFREQ "sv, "unexpectedly succeeded"sv);
		return false;
	}
	host.notice("SYS_TICKFREQ "sv, "failed (expected)"sv);
	host.info("Testing SYS_ELAPSED"sv);
	uint64_t ticksElapsed{};
	// If ticksElapsed is non-0 after this call, something bad happened
	if (semihosting::elapsedTime(ticksElapsed) != SemihostingResult::failure)
	{
		host.error("SYS_ELAPSED "sv, "unexpectedly succeeded"sv);
		return false;
	}
	if (ticksElapsed != UINT64_C(0))
	{
		host.error("SYS_ELAPSED "sv, "failed but wrong modified the result value"sv);
		return false;
	}
	host.notice("SYS_ELAPSED "sv, "failed (expected)"sv);
	return true;
}

[[nodiscard]] static bool testTempName() noexcept
{
	host.warn("-> "sv, __func__);
	host.info("Testing SYS_TMPNAM"sv);
	// Set up a buffer of at least 20 bytes (value of L_tmpnam) to receive the new file name
	std::array<char, 32> fileNameBuffer{};
	// Try to get a new temporary file name
	if (semihosting::tempName({fileNameBuffer}, 10) != SemihostingResult::success)
	{
		host.error("SYS_TMPNAM failed"sv);
		return false;
	}
	// Convert the buffer array into a real string (view) and check what it contains
	const std::string_view fileName{fileNameBuffer.data(), strlen(fileNameBuffer)};
	// BMD uses the for `tempXX.tmp` where XX is the target ID value split nibble-wise and
	// linearly mapped onto the first 16 characters of the alphabet in upper-case
	if (fileName != testTempFileName)
	{
		host.error("SYS_TMPNAM failed, generated "sv, fileName, ", expected "sv, testTempFileName);
		return false;
	}
	host.notice("SYS_TMPNAM success"sv);
	return true;
}

[[nodiscard]] static bool testTimekeeping() noexcept
{
	host.warn("-> "sv, __func__);
	// Check that we can retrieve the current time in seconds and that it goes up at aproximately the right rate
	host.info("Testing SYS_TIME"sv);
	// Start the counter counting
	timer_enable_counter(TIM1);
	timer_clear_flag(TIM1, TIM_SR_UIF);
	// Try to get a start time - if the system call takes too long, increase the time step and
	// try again for up to 8s of time step
	const auto startTime
	{
		[]()
		{
			auto period{timer_get_period(TIM1)};
			while (period < 17999U)
			{
				// Try and retrieve the time
				const auto time{semihosting::time()};
				host.info("Starting time: "sv, time);
				// If the timer did *not* expire, move onto the next step
				if (!timer_get_flag(TIM1, TIM_SR_UIF))
					return time;
				// Otherwise, make note and increase the period by a second
				host.info("Timer already expired, trying again"sv);
				period += 2000U;
				timer_set_period(TIM1, period);
				// Reset the timer
				timer_generate_event(TIM1, TIM_EGR_UG);
				timer_clear_flag(TIM1, TIM_SR_UIF);
			}
			// If all that fails, return UINT32_MAX so we have an easily recognised value on the outside
			return UINT32_MAX;
		}()
	};
	// Check that getting the start time succeeded
	if (startTime == UINT32_MAX)
	{
		host.error("Failed to configure the internal timer for this test"sv);
		return false;
	}
	const auto period{(timer_get_period(TIM1) + 1U) >> 1U};
	// Run 5 requests for the time in succession, checking that they land the right distance apart
	// and are differing values, indicating that the host is counting up properly in seconds
	for (const auto iteration : substrate::indexSequence_t{5U})
	{
		// Wait for the counter to expire and request the time again
		while (!timer_get_flag(TIM1, TIM_SR_UIF))
			continue;
		timer_clear_flag(TIM1, TIM_SR_UIF);
		const auto currentTime{semihosting::time()};
		const auto expectedTimestep{period * (iteration + 1U)};
		const auto actualTimestep{(currentTime - startTime) * 1000U};
		host.info("Timestep "sv, iteration + 1U, ": "sv, currentTime);
		// Check that the resulting time gap tallies with the timer
		if (expectedTimestep != actualTimestep)
		{
			timer_disable_counter(TIM1);
			host.error("Timestep of "sv, actualTimestep, " too "sv,
				actualTimestep < expectedTimestep ? "short"sv : "long"sv, ", expected "sv, expectedTimestep);
			return false;
		}
	}
	// Finish up by disabling the counter again
	timer_disable_counter(TIM1);
	host.notice("SYS_TIME success"sv);
	return true;
}

[[nodiscard]] static bool testIntervals() noexcept
{
	host.warn("-> "sv, __func__);
	// Check that we can retrieve the time elapsed since the start of execution in centiseconds
	// and that it progresses properly against our timer
	host.info("Testing SYS_CLOCK"sv);
	// Start the counter counting, resetting it back to zero before we go on
	timer_enable_counter(TIM1);
	timer_generate_event(TIM1, TIM_EGR_UG);
	timer_clear_flag(TIM1, TIM_SR_UIF);
	// We've already calibrated it to handle syscall latency, so get on and get the first time point
	auto wallTime{semihosting::clock()};
	// Check that getting the start time succeeded
	if (wallTime == -1)
	{
		timer_disable_counter(TIM1);
		host.error("SYS_CLOCK failed"sv);
		return false;
	}
	host.info("Starting time: "sv, wallTime);
	auto period{(timer_get_period(TIM1) + 1U) >> 1U};
	// Run 5 requests for the wall clock in succession, checking that they land the right distance
	// apart and are differing values, indicating that the host is counting up properly in centiseconds
	for (const auto iteration : substrate::indexSequence_t{5U})
	{
		// Wait for the counter to expire and request the wall clock again
		while (!timer_get_flag(TIM1, TIM_SR_UIF))
			continue;
		timer_clear_flag(TIM1, TIM_SR_UIF);
		const auto currentTime{semihosting::clock()};
		const auto timestep{(currentTime - wallTime) * 10U};
		host.info("Timestep "sv, iteration + 1U, ": "sv, currentTime);
		// Check that the resulting time gap tallies with the timer (±40ms)
		if (timestep < period - 40U || timestep > period + 40U)
		{
			timer_disable_counter(TIM1);
			host.error("Timestep of "sv, timestep, " too "sv,
				timestep < period ? "short"sv : "long"sv, ", expected "sv, period);
			return false;
		}
		wallTime = currentTime;
		// Increase the timestep interval by 150ms
		period += 150U;
		timer_set_period(TIM1, (period << 1U) - 1U);
	}
	// Finish up by disabling the counter again
	timer_disable_counter(TIM1);
	host.notice("SYS_CLOCK success"sv);
	return true;
}

[[nodiscard]] static bool testExits() noexcept
{
	host.warn("-> "sv, __func__);
	// Check that calling the two kinds of exit works [for some value of working]
	// Best we can do here is to call them each and then visually check on the
	// other end of the debug connection that the right output is generated from
	// in BMD - the first should result in `exit(0)` being printed, the second
	// in `exit(25)` as extended exit allows us to drive the status code.
	host.info("Testing SYS_EXIT"sv);
	semihosting::exit(ExitReason::applicationExit);
	host.info("Testing SYS_EXIT_EXTENDED"sv);
	semihosting::exit(ExitReason::applicationExit, 25U);
	return true;
}

[[nodiscard]] static bool testSemihosting() noexcept
{
	return
		testReadCommandLine() &&
		testConsoleHandles() &&
		testSemihostingFeatures() &&
		testConsoleWrite() &&
		testFileIO() &&
		testIsError() &&
		testHeapInfo() &&
		testErrno() &&
		testTiming() &&
		testTempName() &&
		testTimekeeping() &&
		testIntervals() &&
		testExits();
}

int main(int, char **)
{
	// Set up clocking for later when we want to run the timekeeping tests
	rcc_clock_setup_pll(&rcc_hsi_configs[RCC_CLOCK_3V3_84MHZ]);
	rcc_periph_clock_enable(RCC_TIM1);
	// Configure Timer 1 as an upcounting timer with a 2kHz tick frequency so we can count microseconds
	timer_set_mode(TIM1, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	// To get a 2kHz tick rate, we have to prescale the APB2 clock (84MHz) down on a ratio of 42000
	// (84MHz / 2kHz), which is encoded in the prescaler register less 1 to get the right ratio
	// We would use 1kHz here but the prescaler is only 16-bit so we don't have enough bits for that
	timer_set_prescaler(TIM1, 41999U);
	// Set that we want to count 1s at a time (1000ms in halves of a ms, which again, must be stored less 1)
	timer_set_period(TIM1, 1999U);
	// Disable the counter, resetting it to 0 ready for use
	timer_disable_counter(TIM1);
	timer_generate_event(TIM1, TIM_EGR_UG);
	timer_clear_flag(TIM1, TIM_SR_UIF);
	// Run the counter continuously once enabled
	timer_continuous_mode(TIM1);
	// Set that we only care about updates on counter overflow
	timer_update_on_overflow(TIM1);

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
		__asm__("bkpt #0");
	return 0;
}
