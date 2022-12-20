#include "Hooks.h"

#include "DFPE/Process.h"
#include "SDL/Renderer.h"
#include "SDL/Formatter.h"


namespace Hooks
{
	namespace CreateSurfaceFromTextAtlas
	{
		SDL::Surface* thunk(const void* a_renderer, SDL::df_packed_ascii* a_attribute) noexcept
		{
			return func(a_renderer, a_attribute);
		}

		void Commit() noexcept
		{
			const auto base = RE::Renderer::BlitTextAtlasToViewscreen_VFunc.full();
			const auto offset = RE::Renderer::Invoke::BTAtV_CreateSurfaceFromASCIIOffset;

			func = RE::Renderer::CreateSurfaceFromASCII_Func.full();

			INFO("CreateSurfaceFromTextAtlas at 0x{:X} | hook entry at 0x{:X}", func.address(), base);
			if (!base || !func.address()) {
				ERROR("Failed to obtain valid function address for CreateSurfaceFromTextAtlas hook!");
			}

			dku::Hook::default_trampoline()->write_call<5>(base + offset, thunk);

			INFO("CreateSurfaceFromTextAtlas hook installed.");
		}
	} // namespace CreateSurfaceFromTextAtlas


	namespace UpperBlit
	{
		// 游戏此处blit使用空rect, 可以安全代替为df_packed_ascii*
		constexpr dku::Hook::Patch RelocateAscii{
			"\x48\x8D\x55\xB3",  // lea rdx, [ rbp - 0x4D ]
			4
		};

		void thunk(SDL::Surface* a_srcSurface, SDL::df_packed_ascii* a_pack, SDL::Surface* a_dstSurface, SDL::Rect* a_dstRect) noexcept
		{
			auto* renderer = SDL::Renderer::GetSingleton();
			renderer->set_sdl_viewscreen(a_dstSurface);

			// 只处理文本, 跳过拼接表情和交替贴图
			if (a_pack->ascii[1] == SDL::df_packed_ascii::kUnset ||
				a_pack->ascii[1] == SDL::df_packed_ascii::kEnd) {
				return SDL::Formatter::GetSingleton()->AddTextAtlas(a_pack, a_dstRect);
			} else {
				return SDL::UpperBlit(a_srcSurface, nullptr, a_dstSurface, a_dstRect);
			}
		}

		void Commit() noexcept
		{
			const auto base = RE::Renderer::BlitTextAtlasToViewscreen_VFunc.full();
			const auto offset = RE::Renderer::Invoke::BTAtV_UpperBlitOffset;

			SDL::UpperBlit = AsAddress(::GetProcAddress(::GetModuleHandleA("sdl.dll"), "SDL_UpperBlit"));

			INFO("SDL_UpperBlit at 0x{:X} | hook entry at 0x{:X}", SDL::UpperBlit.address(), base);
			if (!base || !SDL::UpperBlit.address()) {
				ERROR("Failed to obtain valid function address for SDL_UpperBlit hook!");
			}

			auto handle = dku::Hook::AddCaveHook_Patch(
				base,
				{ offset, offset + sizeof(dku::Hook::Assembly::JmpRip<false>) },
				FUNC_INFO(thunk),
				&RelocateAscii);

			handle->Enable();

			INFO("SDL_UpperBlit hook installed.");
		}
	} // namespace UpperBlit


	namespace PostRenderUpdate
	{
		void thunk() noexcept
		{
			// TODO: cache indexes
			return SDL::Formatter::GetSingleton()->FormatAndFlushAll();
		}

		void Commit() noexcept
		{
			const auto base = RE::Renderer::Invoke::MainThread_PostRenderUpdateOffset.full();
			if (!base) {
				ERROR("Failed to obtain valid function address for MainThread_PostRenderUpdateOffset");
			}

			dku::Hook::default_trampoline()->write_call<5>(base, thunk);

			INFO("MainThread_PostRenderUpdateOffset hook installed.");
		}
	} // namespace PostRenderUpdate
} // namespace Hooks