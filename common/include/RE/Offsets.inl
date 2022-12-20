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

		using ascii_atlas_attribute_t = void;

		using BlitTextAtlasToViewscreen_VFunc_t = std::add_pointer_t<void(RendererInstance_t a_renderer, std::uint32_t a_fileID, std::uint32_t a_atlasID)>;
		constexpr auto BlitTextAtlasToViewscreen_VFunc = RE::Offset(0x5AEE00);

		using CreateSurfaceFromASCII_Func_t = std::add_pointer_t<SDL_Surface*(RendererInstance_t a_renderer, ascii_atlas_attribute_t* a_attribute)>;
		constexpr auto CreateSurfaceFromASCII_Func = RE::Offset(0x5AE290);

		using RenderUpdate_VFunc_t = std::add_pointer_t<void(RendererInstance_t a_renderer)>;
		constexpr auto RenderUpdate_VFunc = RE::Offset(0x5B2B60);

		namespace Invoke
		{
			constexpr auto BTAtV_CreateSurfaceFromASCIIOffset = std::ptrdiff_t(0x3D9);
			constexpr auto BTAtV_UpperBlitOffset = std::ptrdiff_t(0x407);
			constexpr auto MainThread_PostRenderUpdateOffset = RE::Offset(0x5B53FE);
		}
	} // namespace Renderer
} // namespace RE
