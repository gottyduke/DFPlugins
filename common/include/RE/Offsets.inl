#pragma once

#include "Relocation.h"


namespace RE
{
	namespace Game
	{
		using WinMain_func_t  = std::add_pointer_t<int WINAPI(HINSTANCE a_instance, HINSTANCE a_prevInstance, LPSTR a_commands, int a_numCommands)>;
		constexpr auto WinMain = RE::Offset(0x1032440);
	} // namespace Game

	namespace Renderer
	{
		using AtlasManager_t = void*;
		constexpr auto AtlasManager = RE::Offset(0x12AB950);

		using AddAtlasToBuffer_Func_t = std::add_pointer_t<std::uint64_t(AtlasManager_t a_sdl, std::int8_t a_atlas, std::int8_t a_offset)>;
		constexpr auto AddAtlasToBuffer_Func = RE::Offset(0x55C70);

		using RendererInstance_t = void*;
		using SDL_Surface = void;

		using CreateSurfaceFromASCII_Func_t = std::add_pointer_t<SDL_Surface*(RendererInstance_t a_renderer, std::int8_t a_ascii)>;
		constexpr auto CreateSurfaceFromASCII_Func = RE::Offset(0x5AE290);

		using BlitTextAtlasToViewscreen_VFunc_t = std::add_pointer_t<void(RendererInstance_t a_renderer, std::uint32_t a_fileID, std::uint32_t a_atlasID)>;
		constexpr auto BlitAtlasToViewscreen_VFunc = RE::Offset(0x5AEE00);
		constexpr std::ptrdiff_t BlitAtlasToViewscreen_VFunc_TextAtlasInvokeOffset = 0x407;
	} // namespace Renderer
} // namespace RE
