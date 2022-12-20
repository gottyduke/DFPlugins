#include "Renderer.h"


#include "DKUtil/Config.hpp"


namespace SDL
{
	Renderer::Renderer() noexcept
	{
		const auto freetype = ::LoadLibraryA("plugins\\translation_extender\\font\\libfreetype-6.dll");
		const auto sdl_ttf = ::LoadLibraryA("plugins\\translation_extender\\font\\SDL_ttf.dll");
		if (sdl_ttf) {
			SDL::TTF::OpenFont = AsAddress(::GetProcAddress(sdl_ttf, "TTF_OpenFont"));
			SDL::TTF::RenderText_Solid = AsAddress(::GetProcAddress(sdl_ttf, "TTF_RenderText_Solid"));
		} else {
			ERROR("Failed to load SDL_ttf!");
		}

		if (!SDL::TTF::OpenFont.address() ||
			!SDL::TTF::RenderText_Solid.address()) {
			ERROR("Failed to obtain valid function address for SDL_ttf hook!");
		}

		const auto sdl = ::GetModuleHandleA("sdl.dll");
		if (sdl) {
			SDL::CreateRGBSurface = AsAddress(::GetProcAddress(sdl, "SDL_CreateRGBSurface"));
			SDL::SetPalette = AsAddress(::GetProcAddress(sdl, "SDL_SetPalette"));
			SDL::FreeSurface = AsAddress(::GetProcAddress(sdl, "SDL_FreeSurface"));
			SDL::UpperBlit = AsAddress(::GetProcAddress(sdl, "SDL_UpperBlit"));
		} else {
			ERROR("Failed to obtain sdl dll address!");
		}

		_ttf = SDL::TTF::OpenFont("plugins\\translation_extender\font\\YeZiGongChangShanHaiMingChao-2.ttf", 24);
		if (!_ttf) {
			ERROR("Failed to open ttf file!");
		}

		INFO("SDL Renderer has been initialized!");
	}

	void Renderer::ShowText(std::string_view a_text, SDL::Rect a_dst) noexcept
	{
		dku::memzero(_buffer, sizeof(_buffer));

		SDL::Color White = {
			SDL::Scalar(255),
			SDL::Scalar(255),
			SDL::Scalar(255)
		};

		auto* textSurface = SDL::TTF::RenderText_Solid(_ttf, a_text.data(), White);
		if (!textSurface) {
			WARN(
				"Failed creating text surface!\n"
				"string : {}", a_text);
			return;
		}

		SDL::UpperBlit(textSurface, nullptr, _viewscreen, &a_dst);

		SDL::FreeSurface(textSurface);
	}
}  // namespace SDL
