#include "Hooks.h"

#include "DFPE/Process.h"


namespace Hooks
{
	void Hook_SDL_UpperBlit(SDL::Surface* a_srcSurface, std::int8_t a_ascii, SDL::Surface* a_dstSurface, SDL::Rect* a_dstRect) noexcept
	{


		return SDL_UpperBlit(a_srcSurface, 0, a_dstSurface, a_dstRect);
	}


	void Commit() noexcept
	{
		const auto func = RE::Renderer::BlitAtlasToViewscreen_VFunc.full();
		const auto offset = RE::Renderer::BlitAtlasToViewscreen_VFunc_TextAtlasInvokeOffset;

		SDL_UpperBlit = dku::AsPun<std::uintptr_t>(dku::Hook::GetImportAddress(DFPE::DF_PROC, "sdl.dll"sv, "SDL_UpperBlit"sv));

		INFO("SDL_UpperBlit at 0x{:X} | hook entry at 0x{:X}", SDL_UpperBlit.address(), func);
		if (!func || !SDL_UpperBlit.address()) {
			ERROR("Failed to obtain valid function address for SDL_UpperBlit hook!");
		}

		auto handle = dku::Hook::AddCaveHook(
			func,
			{ offset, offset + sizeof(dku::Hook::Assembly::JmpRip<false>) },
			FUNC_INFO(Hook_SDL_UpperBlit));

		handle->Enable();

		INFO("SDL_UpperBlit hook installed.");
	}
} // namespace Hooks