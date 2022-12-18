#pragma once


// 只定义一些用的的结构, 不必依赖SDL库
namespace SDL
{
	struct Rect
	{
		std::uint16_t x;
		std::uint16_t y;
		std::uint16_t w;
		std::uint16_t h;
	};


	struct Surface
	{
		using PixelFormat = void;
		using BlitMap = void;
		
		const std::uint64_t flags;
		const PixelFormat* format;
		const std::uint32_t x;
		const std::uint32_t y;
		const int pitch;
		void* pixels;
		void* userdata;
		const int locked;
		const void* lock_data;
		const std::uint32_t pad38;
		const std::uint16_t clipX;
		const std::uint16_t clipY;
		const BlitMap* map;
		int refCount;
	};
}  // namespace SDL