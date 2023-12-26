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
#include "syscalls.hxx"
#include "syscall_types.hxx"

using namespace semihosting::types;

/*
 * syscall and paramsPtr are both forced into registers by the function call itself,
 * so we take advantage of the fact that syscall will be in r0 and paramsPtr in r1
 * and that ARM expects the result in r0 to make the implementation super minimal.
 * By marking the function as never inlined, and naked, everything's set up ready for
 * the breakpoint instruction and we just have to return to wherever the program counter
 * was after from the link register value.
 */
[[gnu::naked, gnu::noinline]] static int32_t semihostingSyscall([[maybe_unused]] const Syscall syscall,
	[[maybe_unused]] const void *const paramsPtr) noexcept
{
	__asm__ volatile(R"(
		bkpt #0xab
		bx lr
	)");
}

template<typename T, size_t N> static int32_t semihostingSyscall(const Syscall syscall,
	const std::array<T, N> &params) noexcept
		{ return semihostingSyscall(syscall, params.data()); }

namespace semihosting
{
	int32_t open(const std::string_view &path, const OpenMode mode) noexcept
	{
		const std::array<uintptr_t, 3> params
		{{
			reinterpret_cast<uintptr_t>(path.data()),
			static_cast<uintptr_t>(mode),
			path.length(),
		}};
		return semihostingSyscall(Syscall::open, params);
	}

	types::SemihostingResult close(const int32_t fd) noexcept
	{
		const auto result{semihostingSyscall(Syscall::close, &fd)};
		return static_cast<SemihostingResult>(result);
	}

	types::SemihostingResult writeChar(const char chr) noexcept
	{
		const auto result{semihostingSyscall(Syscall::writeChar, &chr)};
		return static_cast<SemihostingResult>(result);
	}

	types::SemihostingResult write(const char *const string) noexcept
	{
		const auto result{semihostingSyscall(Syscall::writeNulStr, string)};
		return static_cast<SemihostingResult>(result);
	}

	int32_t write(const int32_t fd, const substrate::span<const uint8_t> &data) noexcept
	{
		const std::array<uintptr_t, 3> params
		{{
			static_cast<uintptr_t>(fd),
			reinterpret_cast<uintptr_t>(data.data()),
			data.size_bytes(),
		}};
		return semihostingSyscall(Syscall::write, params);
	}

	int32_t read(const int32_t fd, substrate::span<uint8_t> data) noexcept
	{
		const std::array<uintptr_t, 3> params
		{{
			static_cast<uintptr_t>(fd),
			reinterpret_cast<uintptr_t>(data.data()),
			data.size_bytes(),
		}};
		return semihostingSyscall(Syscall::read, params);
	}

	int32_t readChar() noexcept
		{ return semihostingSyscall(Syscall::readChar, nullptr); }

	int32_t isError(const int32_t status) noexcept
		{ return semihostingSyscall(Syscall::isError, &status); }

	int32_t isTTY(const int32_t fd) noexcept
		{ return semihostingSyscall(Syscall::isTTY, &fd); }

	int32_t seek(const int32_t fd, const uint32_t offset) noexcept
	{
		std::array<uint32_t, 2> params
		{{
			static_cast<uint32_t>(fd),
			offset,
		}};
		return semihostingSyscall(Syscall::seek, params);
	}

	int32_t fileLength(const int32_t fd) noexcept
		{ return semihostingSyscall(Syscall::fileLength, &fd); }

	int32_t tempName(substrate::span<char> &fileName, int32_t targetID) noexcept
	{
		const std::array<uintptr_t, 3> params
		{{
			reinterpret_cast<uintptr_t>(fileName.data()),
			static_cast<uintptr_t>(targetID),
			fileName.size_bytes()
		}};
		return semihostingSyscall(Syscall::tempName, params);
	}

	types::SemihostingResult remove(const std::string_view &path) noexcept
	{
		const std::array<uintptr_t, 2> params
		{{
			reinterpret_cast<uintptr_t>(path.data()),
			path.length(),
		}};
		const auto result{semihostingSyscall(Syscall::remove, params)};
		return static_cast<SemihostingResult>(result);
	}

	types::SemihostingResult rename(const std::string_view &oldName, const std::string_view &newName) noexcept
	{
		const std::array<uintptr_t, 4> params
		{{
			reinterpret_cast<uintptr_t>(oldName.data()),
			oldName.length(),
			reinterpret_cast<uintptr_t>(newName.data()),
			newName.length(),
		}};
		const auto result{semihostingSyscall(Syscall::rename, params)};
		return static_cast<SemihostingResult>(result);
	}

	int32_t clock() noexcept
		{ return semihostingSyscall(Syscall::clock, nullptr); }

	int32_t time() noexcept
		{ return semihostingSyscall(Syscall::time, nullptr); }

	int32_t system(const std::string_view &command) noexcept
	{
		const std::array<uintptr_t, 2> params
		{{
			reinterpret_cast<uintptr_t>(command.data()),
			command.length(),
		}};
		return semihostingSyscall(Syscall::system, params);
	}

	int32_t lastErrno() noexcept
		{ return semihostingSyscall(Syscall::lastErrno, nullptr); }

	SemihostingResult readCommandLine(substrate::span<char> commandLine) noexcept
	{
		const std::array<uintptr_t, 2> params
		{{
			reinterpret_cast<uintptr_t>(commandLine.data()),
			commandLine.size_bytes(),
		}};
		const auto result{semihostingSyscall(Syscall::readCommandLine, params)};
		return static_cast<SemihostingResult>(result);
	}

	int32_t heapInfo(HeapInfoBlock &infoBlock) noexcept
		{ return semihostingSyscall(Syscall::heapInfo, &infoBlock); }

	void exit(const uint32_t code) noexcept
	{
		semihostingSyscall(Syscall::exit32, reinterpret_cast<void *>(code));
		while (true)
			continue;
	}

	void exit(const uint64_t code) noexcept
	{
		const std::array<uint32_t, 2> params
		{{
			static_cast<uint32_t>(code),
			static_cast<uint32_t>(code >> 32U),
		}};
		semihostingSyscall(Syscall::exit64, params);
		while (true)
			continue;
	}

	int32_t elapsedTime(uint64_t &ticks) noexcept
		{ return semihostingSyscall(Syscall::elapsed, &ticks); }

	int32_t tickFrequency() noexcept
		{ return semihostingSyscall(Syscall::tickFrequency, nullptr); }
} // namespace semihosting
