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

#include <string_view>
#include "syscalls.hxx"
#include "hostConsole.hxx"

using namespace std::literals::string_view_literals;
using namespace semihosting::types;

namespace semihosting::host::console
{
	Console host{};

	bool Console::open() noexcept
	{
		fdFromHost = semihosting::open(":tt"sv, OpenMode::read);
		fdToHost = semihosting::open(":tt"sv, OpenMode::write);
		return fdFromHost != -1 && fdToHost != -1;
	}

	void Console::write(const std::string_view &value) const noexcept
		{ static_cast<void>(semihosting::write(value.data())); }

	// Output `[!]` in red
	void Console::errorPrefix() const noexcept
		{ write("\x1b[31m[!]\x1b[0m"sv); }

	// Output `[*]` in yellow/brown
	void Console::warningPrefix() const noexcept
		{ write("\x1b[33m[*]\x1b[0m"sv); }

	// Output `[~]` in green
	void Console::noticePrefix() const noexcept
		{ write("\x1b[32m[~]\x1b[0m"sv); }

	// Output `[~]` in cyan
	void Console::infoPrefix() const noexcept
		{ write("\x1b[36m[~]\x1b[0m"sv); }
} // namespace host
