#pragma once

#include "SDL.inl"


#include "external/stb_truetype.h"


namespace SDL
{
	class Renderer : public dku::model::Singleton<Renderer>
	{
	public:
		void ShowText(std::string_view a_text, SDL::Rect a_dst) noexcept;


		[[nodiscard]] constexpr auto* get_sdl_viewscreen(SDL::Surface* a_surface) const noexcept { return _viewscreen; }
		[[nodiscard]] constexpr void set_sdl_viewscreen(SDL::Surface* a_surface) noexcept { _viewscreen = a_surface; }

	private:
		friend dku::model::Singleton<Renderer>;

		Renderer() noexcept;
		

		SDL::TTF::Font* _ttf;
		SDL::Surface* _viewscreen;

		wchar_t _buffer[256];
	};
}  // namespace SDL

