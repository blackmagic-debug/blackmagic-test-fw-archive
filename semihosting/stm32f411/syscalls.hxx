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

#ifndef SYSCALLS_HXX
#define SYSCALLS_HXX

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <string_view>
#include <substrate/span>

#include "syscallTypes.hxx"

namespace semihosting
{
	[[nodiscard]] int32_t open(const std::string_view &path, types::OpenMode mode) noexcept;
	[[nodiscard]] types::SemihostingResult close(int32_t fd) noexcept;
	[[nodiscard]] types::SemihostingResult writeChar(char chr) noexcept;
	[[nodiscard]] types::SemihostingResult write(const char *string) noexcept;
	[[nodiscard]] int32_t write(int32_t fd, const substrate::span<const uint8_t> &data) noexcept;
	[[nodiscard]] int32_t read(int32_t fd, substrate::span<uint8_t> data) noexcept;
	[[nodiscard]] int32_t readChar() noexcept;
	[[nodiscard]] bool isError(int32_t status) noexcept;
	[[nodiscard]] int32_t isTTY(int32_t fd) noexcept;
	[[nodiscard]] int32_t seek(int32_t fd, uint32_t offset) noexcept;
	[[nodiscard]] int32_t fileLength(int32_t fd) noexcept;
	[[nodiscard]] int32_t tempName(substrate::span<char> &fileName, int32_t targetID) noexcept;
	[[nodiscard]] types::SemihostingResult remove(const std::string_view &path) noexcept;
	[[nodiscard]] types::SemihostingResult rename(const std::string_view &oldName,
		const std::string_view &newName) noexcept;
	[[nodiscard]] int32_t clock() noexcept;
	[[nodiscard]] int32_t time() noexcept;
	[[nodiscard]] int32_t system(const std::string_view &command) noexcept;
	[[nodiscard]] types::FileIOErrno lastErrno() noexcept;
	[[nodiscard]] types::SemihostingResult readCommandLine(substrate::span<char> commandLine) noexcept;
	void heapInfo(types::HeapInfoBlock &infoBlock) noexcept;
	[[noreturn]] void exit(uint32_t code) noexcept;
	[[noreturn]] void exit(uint64_t code) noexcept;
	[[nodiscard]] types::SemihostingResult elapsedTime(uint64_t &ticks) noexcept;
	[[nodiscard]] int32_t tickFrequency() noexcept;

	[[nodiscard]] static inline int32_t read(const int32_t fd, void *const dataPointer,
		const size_t dataLength) noexcept
	{
		substrate::span data{static_cast<uint8_t *>(dataPointer), dataLength};
		return read(fd, data);
	}

	template<typename T, size_t N> [[nodiscard]] static inline std::enable_if_t<!std::is_same_v<T, uint8_t>, int32_t>
		read(const int32_t fd, substrate::span<T, N> data) noexcept
	{
		const substrate::span dataBytes{reinterpret_cast<uint8_t *>(data.data()), data.size_bytes()};
		return read(fd, dataBytes);
	}

	[[nodiscard]] static inline int32_t write(const int32_t fd, const void *const dataPointer,
		const size_t dataLength) noexcept
	{
		const substrate::span data{static_cast<const uint8_t *>(dataPointer), dataLength};
		return write(fd, data);
	}

	template<typename T, size_t N> [[nodiscard]] static inline std::enable_if_t<!std::is_same_v<T, uint8_t>, int32_t>
		write(const int32_t fd, const substrate::span<T, N> &data) noexcept
	{
		const substrate::span dataBytes{reinterpret_cast<const uint8_t *>(data.data()), data.size_bytes()};
		return write(fd, dataBytes);
	}
} // namespace semihosting

#endif /*SYSCALLS_HXX*/
