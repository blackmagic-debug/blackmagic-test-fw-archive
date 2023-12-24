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

#ifndef HOST_CONSOLE_HXX
#define HOST_CONSOLE_HXX

#include <cstdint>
#include <string_view>

namespace semihosting::host::console
{
	using namespace std::literals::string_view_literals;

	struct Console final
	{
	private:
		int32_t fdFromHost{-1};
		int32_t fdToHost{-1};

		void write(const std::string_view &value) const noexcept;
		void errorPrefix() const noexcept;
		void warningPrefix() const noexcept;
		void noticePrefix() const noexcept;
		void infoPrefix() const noexcept;

	public:
		Console() noexcept = default;
		[[nodiscard]] bool openConsole() noexcept;
		[[nodiscard]] bool closeConsole() noexcept;

		void writeln() const noexcept
			{ write("\n"sv); }

		template<typename Value, typename... Values> void writeln(Value && value, Values &&...values) const noexcept
		{
			write(value);
			writeln(values...);
		}

		template<typename... Values> void error(Values &&...values) const noexcept
		{
			errorPrefix();
			writeln(std::forward<Values>(values)...);
		}

		template<typename... Values> void warning(Values &&...values) const noexcept
		{
			warningPrefix();
			writeln(std::forward<Values>(values)...);
		}

		template<typename... Values> void warn(Values &&...values) const noexcept
			{ warning(std::forward<Values>(values)...); }

		template<typename... Values> void notice(Values &&...values) const noexcept
		{
			noticePrefix();
			writeln(std::forward<Values>(values)...);
		}

		template<typename... Values> void info(Values &&...values) const noexcept
		{
			infoPrefix();
			writeln(std::forward<Values>(values)...);
		}
	};

	extern Console host;
} // namespace semihosting::host

#endif /*HOST_CONSOLE_HXX*/
