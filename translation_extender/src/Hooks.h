#pragma once

#include "DKUtil/Hook.hpp"
#include <xbyak.h>

#include "RE/Offsets.inl"

#include "SDL/SDL.inl"


namespace Hooks
{
	struct Prolog : public Xbyak::CodeGenerator
	{
		Prolog()
		{
			mov(rdx, ptr[rbp - 0x4D]);
		}
	};


	struct Epilog : public Xbyak::CodeGenerator
	{
		Epilog()
		{

		}
	};


	void Hook_SDL_UpperBlit(SDL::Surface* a_srcSurface, std::int8_t a_ascii, SDL::Surface* a_dstSurface, SDL::Rect* a_dstRect) noexcept;

	static inline RE::Relocation<decltype(Hook_SDL_UpperBlit)> SDL_UpperBlit;

	void Commit() noexcept;
} // namespace Hooks