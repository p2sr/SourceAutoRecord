/*

	VMTHook - incredibly straight-forward virtual hooking class.
	Copyright (C) 2017, aixxe. <me@aixxe.net>
		
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with VMTHook. If not, see <http://www.gnu.org/licenses/>.

*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>

class VMTHook {
	private:
		std::uintptr_t** baseclass = nullptr;
		std::unique_ptr<std::uintptr_t[]> current_vft = nullptr;
		std::uintptr_t* original_vft = nullptr;
		std::size_t total_functions = 0;
	public:
		VMTHook(void) = default;

		VMTHook(void* baseclass) {
			this->baseclass = static_cast<std::uintptr_t**>(baseclass);

			while (static_cast<std::uintptr_t*>(*this->baseclass)[this->total_functions])
				++this->total_functions;

			const std::size_t table_size = this->total_functions * sizeof(std::uintptr_t);

			this->original_vft = *this->baseclass;
			this->current_vft = std::make_unique<std::uintptr_t[]>(this->total_functions);

			std::memcpy(this->current_vft.get(), this->original_vft, table_size);

			*this->baseclass = this->current_vft.get();
		};

		~VMTHook() {
			*this->baseclass = this->original_vft;
		};

		template <typename Fn> inline const Fn GetOriginalFunction(std::size_t function_index) {
			return (Fn)this->original_vft[function_index];
		}

		inline bool HookFunction(void* new_function, const std::size_t function_index) {
			if (function_index > this->total_functions)
				return false;

			this->current_vft[function_index] = reinterpret_cast<std::uintptr_t>(new_function);

			return true;
		}

		inline bool UnhookFunction(const std::size_t function_index) {
			if (function_index > this->total_functions)
				return false;

			this->current_vft[function_index] = this->original_vft[function_index];

			return true;
		}

		inline std::size_t GetTotalFunctions() {
			return this->total_functions;
		}

		inline void* GetThisPtr() {
			return reinterpret_cast<void*>(this->baseclass);
		}
};