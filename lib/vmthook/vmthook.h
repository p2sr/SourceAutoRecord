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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>

class VMTHook {
private:
    uintptr_t** baseclass = nullptr;
    uintptr_t* original = nullptr;
    int total = 0;
    bool hooked = false;
    std::unique_ptr<uintptr_t[]> vtable = nullptr;

public:
    VMTHook(void) = default;

    VMTHook(void* baseclass, bool enable = true)
    {
        this->baseclass = reinterpret_cast<uintptr_t**>(baseclass);

        while (static_cast<uintptr_t*>(*this->baseclass)[this->total])
            ++this->total;

        this->original = *this->baseclass;
        this->vtable = std::make_unique<uintptr_t[]>(this->total);
        std::memcpy(this->vtable.get(), this->original, this->total * sizeof(uintptr_t));

        if (enable) {
            this->Enable();
        }
    }
    ~VMTHook()
    {
        this->Disable();
    }
    void Enable()
    {
        if (!this->hooked) {
            *this->baseclass = this->vtable.get();
            this->hooked = true;
        }
    }
    void Disable()
    {
        if (this->hooked) {
            *this->baseclass = this->original;
            this->hooked = false;
        }
    }
    template <typename T = uintptr_t>
    T GetOriginal(int index, bool readJmp = false)
    {
        if (readJmp) {
            auto source = this->original[index] + 1;
            auto rel = *reinterpret_cast<uintptr_t*>(source);
            return (T)(source + rel + sizeof(rel));
        }
        return (T)this->original[index];
    }
    bool Hook(void* detour, int index)
    {
        if (index > this->total)
            return false;

        this->vtable[index] = reinterpret_cast<uintptr_t>(detour);

        return true;
    }
    bool Unhook(int index)
    {
        if (index > this->total)
            return false;

        this->vtable[index] = this->original[index];

        return true;
    }
    int GetTotalFunctions()
    {
        return this->total;
    }
    void* GetThisPtr()
    {
        return reinterpret_cast<void*>(this->baseclass);
    }
    /* operator bool()
    {
        return this->GetThisPtr() != nullptr;
    } */
};