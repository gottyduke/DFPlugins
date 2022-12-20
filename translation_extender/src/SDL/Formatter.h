#pragma once

#include "DKUtil/Utility.hpp"

#include "SDL/SDL.inl"


namespace SDL
{
	class Formatter : public dku::model::Singleton<Formatter>
	{
		struct text_entry
		{
			const std::int16_t x;
			std::uint16_t w;
			std::uint16_t h;
			
			SDL::Color rgba;

			char buf[256];
			std::int16_t pos;

			text_entry* next;
			bool rendered = false;
		};


	public:

		void AddTextAtlas(SDL::df_packed_ascii* a_pack, SDL::Rect* a_rect) noexcept;
		void FormatAndFlushAll() noexcept;
		void FormatAndFlush(std::int16_t a_y) noexcept;

	private:
		friend dku::model::Singleton<Formatter>;

		Formatter() noexcept = default;

		void clear() noexcept { _indexes.clear(); }

		// y-axis indexed text entries
		std::unordered_map<std::int16_t, std::vector<text_entry>> _indexes;
	};
}  // namespace SDL