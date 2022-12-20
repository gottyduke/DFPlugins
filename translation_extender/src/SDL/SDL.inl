#pragma once

#include "RE/Relocation.h"


// 一些必要的定义, 不必依赖SDL库
namespace SDL
{
	namespace Structures
	{

		struct Rect
		{
			std::int16_t x;
			std::int16_t y;
			std::uint16_t w;
			std::uint16_t h;
		};
		static_assert(sizeof(Rect) == 0x8);

		struct Surface
		{
			using PixelFormat = void;
			using BlitMap = void;

			const std::uint32_t flags;
			const PixelFormat* format;
			const std::int32_t x;
			const std::int32_t y;
			const std::uint16_t pitch;
			void* pixels;
			Rect clip;
			int refCount;
		};

		enum SurfaceFlag : std::uint32_t
		{
			SWSURFACE = 0,
		};

		struct Color
		{
			float red;
			float green;
			float blue;
			float alpha;
		};
		static_assert(sizeof(Color) == 0x10);

		enum PaletteFlag : std::uint32_t
		{
			LOGPAL = 1 << 0,
			PHYSPAL = 1 << 1,
		};

		consteval float Scalar(std::uint32_t a_rgb) noexcept
		{
			return static_cast<float>(a_rgb / 255);
		}
	} // namespace Structures
	using namespace Structures;


	namespace Functions
	{
		using FreeSurface_func_t = std::add_pointer_t<void(Surface* a_surface)>;
		static inline RE::Relocation<FreeSurface_func_t> FreeSurface;

		using CreateRGBSurface_func_t = std::add_pointer_t<Surface*(std::uint32_t a_flag, int a_width, int a_height, int a_depth, float a_r, float a_g, float a_b, float a_a)>;
		static inline RE::Relocation<CreateRGBSurface_func_t> CreateRGBSurface;

		using SetPalette_func_t = std::add_pointer_t<int(Surface* a_surface, std::uint32_t a_flag, Color* a_palette, int a_first, int a_total)>;
		static inline RE::Relocation<SetPalette_func_t> SetPalette;

		using UpperBlit_func_t = std::add_pointer_t<void(Surface* a_src, Rect* a_srcRect, Surface* a_dst, Rect* a_dstRect)>;
		static inline RE::Relocation<UpperBlit_func_t> UpperBlit;
	} // namespace Functions
	using namespace Functions;


	struct df_packed_ascii
	{
		enum codepoint : std::uint8_t
		{
			kUnset = 0x00,   // 空
			kEnd = 0x01,     // codepoint结束
			kAtlas = 0x04,   // 意义不明
			kConcat = 0x62,  // 拼接表情
			kAlt = 0x64,     // 8bit贴图
		};

		codepoint ascii[4];
		Color rbga;
	};
	static_assert(sizeof(df_packed_ascii) == 0x14);


	namespace TTF
	{
		using Font = void;

		using OpenFont_Func_t = std::add_pointer_t<Font*(const char* a_file, float a_size)>;
		static inline RE::Relocation<OpenFont_Func_t> OpenFont;

		using RenderText_Solid_Func_t = std::add_pointer_t<Surface*(Font* a_font, const char* a_text, Color)>;
		static inline RE::Relocation<RenderText_Solid_Func_t> RenderText_Solid;
	} // namespace TTF
}  // namespace SDL