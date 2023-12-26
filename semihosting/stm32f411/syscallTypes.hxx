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

#ifndef SYSCALL_TYPES_HXX
#define SYSCALL_TYPES_HXX

#include <cstdint>

namespace semihosting::types
{
	enum class Syscall : uint32_t
	{
		open = 0x01U,
		close = 0x02U,
		writeChar = 0x03U,
		writeNulStr = 0x04U,
		write = 0x05U,
		read = 0x06U,
		readChar = 0x07U,
		isError = 0x08U,
		isTTY = 0x09U,
		seek = 0x0aU,
		fileLength = 0x0cU,
		tempName = 0x0dU,
		remove = 0x0eU,
		rename = 0x0fU,
		clock = 0x10U,
		time = 0x11U,
		system = 0x12U,
		lastErrno = 0x13U,
		readCommandLine = 0x15U,
		heapInfo = 0x16U,
		exit32 = 0x18U,
		exit64 = 0x20U,
		elapsed = 0x30U,
		tickFrequency = 0x31U,
	};

	enum class OpenMode : uint8_t
	{
		read = 0,
		readBinary = 1,
		readPlus = 2,
		readBinaryPlus = 3,
		write = 4,
		writeBinary = 5,
		writePlus = 6,
		writeBinaryPlus = 7,
		append = 8,
		appendBinary = 9,
		appendPlus = 10,
		appendBinaryPlus = 11,
	};

	struct HeapInfoBlock
	{
		uintptr_t heapBase;
		uint32_t heapLimit;
		uintptr_t stackBase;
		uint32_t stackLimit;
	};

	enum class SemihostingResult : int32_t
	{
		success = 0,
		failure = -1,
	};

	enum class FileIOErrno : int32_t
	{
		/* EPERM - Operation not permitted */
		notPermitted = 1,
		/* ENOENT - No such file or directory */
		noSuchEntity = 2,
		/* EINTR - Interrupted system call */
		syscallInterrupted = 4,
		/* EIO - I/O error */
		ioError = 5,
		/* EBADF - Bad file number */
		badFD = 9,
		/* EACCESS - Permission denied */
		accessError = 13,
		/* EFAULT - Bad address */
		addressFault = 14,
		/* EBUSY - Device or resource busy */
		busy = 16,
		/* EEXIST - File exists */
		alreadyExists = 17,
		/* ENODEV - No such device */
		noSuchDevice = 19,
		/* ENOTDIR - Not a directory */
		notADir = 20,
		/* EISDIR - Is a directory */
		isADir = 21,
		/* EINVAL - Invalid argument */
		argumentInvalid = 22,
		/* ENFILE - File table overflow */
		fileTableFull = 23,
		/* EMFILE - Too many open files */
		tooManyOpenFiles = 24,
		/* EFBIG - File too large */
		fileTooLarge = 27,
		/* ENOSPC - No space left on device */
		outOfSpace = 28,
		/* ESPIPE - Illegal seek */
		illegalSeek = 29,
		/* EROFS - Read-only file system */
		fsReadOnly = 30,
		/* ENOSYS - Invalid system call number */
		syscallInvalid = 88,
		/* ENAMETOOLONG - File name too long */
		fileNameTooLong = 91,
	};
} // namespace semihosting::types

#endif /*SYSCALL_TYPES_HXX*/
