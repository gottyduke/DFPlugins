#include "Formatter.h"

#include "Renderer.h"


namespace SDL
{
	void Formatter::AddTextAtlas(SDL::df_packed_ascii* a_pack, SDL::Rect* a_rect) noexcept
	{
		constexpr auto allowed_kern = 1;

		const auto& [ascii, color] = *a_pack;
		auto& [x, y, w, h] = *a_rect;
		w = w >= 1 << 10 ? 8 : w;
		h = h ? h : 12;

		text_entry* entry = nullptr;
		for (auto& index : _indexes[y]) {
			if (x <= index.x + allowed_kern + index.w) {
				// 拓展当前字段渲染区
				index.w += w;
				entry = std::addressof(index);
				break;
			}
		}

		// 当前Y轴新字段
		if (!entry) {
			entry = std::addressof(_indexes[y].emplace_back(x, w, h, color));
		}

		// 储存并结尾
		if (entry->pos < std::extent_v<decltype(entry->buf)> - 1) {
			entry->buf[entry->pos++] = ascii[0];
			entry->buf[entry->pos] = 0;
		}
	}

	void Formatter::FormatAndFlushAll() noexcept
	{
		for (auto& [y, index] : _indexes) {
			for (auto& entry : index) {
				if (dku::string::is_only_space(entry.buf)) {
					continue;
				}
				INFO("{} {}", entry.buf, entry.pos);
				Renderer::GetSingleton()->ShowText(entry.buf, { entry.x, y, entry.w, entry.h });
			}
		}

		clear();
	}

	void Formatter::FormatAndFlush(std::int16_t a_y) noexcept
	{
		for (auto& entry : _indexes[a_y]) {
			INFO("{}", entry.buf);
		}

		_indexes.erase(a_y);
	}
}  // namespace SDL