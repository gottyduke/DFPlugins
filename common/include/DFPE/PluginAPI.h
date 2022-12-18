#pragma once

#include "Prefix.h"

#include "RE/Versions.inl"


namespace DFPE
{
	static constexpr auto PLUGIN_INFO_EXPORT_NAME = "DFPE_PluginInfo";

	using PluginHandle = std::uint32_t;

	/* export your plugin's info for DFPE querying

	DFPE_EXPORT_PLUGIN_INFO
	{
		DFPE::PluginInfo info{};

		info.SetName(Plugin::NAME);
		info.SetVersion(Plugin::VERSION);
		info.SetAuthor("ModAuthor"sv);
		info.SupportLatest(); // or specific version from RE::VERSION

		return info;
	}();

	*/

#pragma pack( push, 1)
	struct PluginInfo
	{
		enum
		{
			kPacked = 1,
		};

		const std::uint32_t infoVersion = kPacked;

		[[maybe_unused]] void constexpr SetName(std::string_view a_name) noexcept { SetCharBuffer(a_name, name); }
		[[maybe_unused]] void constexpr SetVersion(std::string_view a_version) noexcept { SetCharBuffer(a_version, version); }
		[[maybe_unused]] void constexpr SetAuthor(std::string_view a_author) noexcept { SetCharBuffer(a_author, author); }
		[[maybe_unused]] void constexpr Support(RE::VERSION a_target) noexcept { supportedVersion = a_target; }
		[[maybe_unused]] void constexpr SupportLatest() noexcept { supportedVersion = static_cast<RE::VERSION>(std::to_underlying(RE::VERSION::kError) - 1); }

		char name[128] = {};
		char version[128] = {};
		char author[128] = {};
		RE::VERSION supportedVersion{ RE::VERSION(0) };

	private:
		static constexpr void SetCharBuffer(std::string_view a_src, std::span<char> a_dst) noexcept
		{
			assert(a_src.size() < a_dst.size());
			std::ranges::fill(a_dst, '\0');
			std::ranges::copy(a_src, a_dst.begin());
		}
	};
	static_assert(sizeof(PluginInfo) == 0x188);
#pragma pack(pop)

	
	inline void WaitForDebugger(void)
	{
#ifndef NDEBUG
		while (!IsDebuggerPresent()) {
			Sleep(100);
		}

		// hold 2 seconds for debugger to attach fully
		Sleep(1000 * 2);
#endif
	}
} // namespace DFPE::PluginAPI


#define DFPE_EXPORT_PLUGIN_INFO DLLEXPORT constinit auto DFPE_PluginInfo = []() noexcept
