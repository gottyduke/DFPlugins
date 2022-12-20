#pragma once

#include "DKUtil/Hook.hpp"
#include <xbyak.h>

#include "RE/Offsets.inl"

#include "SDL/SDL.inl"


namespace Hooks
{
	namespace CreateSurfaceFromTextAtlas
	{
		SDL::Surface* thunk(const void* a_renderer, SDL::df_packed_ascii* a_attribute) noexcept;

		static inline RE::Relocation<decltype(thunk)> func;

		void Commit() noexcept;
	} // namespace CreateSurfaceFromTextAtlas


	namespace UpperBlit
	{
		void thunk(SDL::Surface* a_srcSurface, SDL::df_packed_ascii* a_pack, SDL::Surface* a_dstSurface, SDL::Rect* a_dstRect) noexcept;
		
		void Commit() noexcept;
	} // namespace UpperBlit


	namespace PostRenderUpdate
	{
		void thunk() noexcept;

		void Commit() noexcept;
	} // namespace PostRenderUpdate
} // namespace Hooks