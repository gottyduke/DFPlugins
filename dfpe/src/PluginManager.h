#pragma once

#include "DFPE/PluginAPI.h"
#include "DFPE/Interfaces.h"


namespace DFPE::API
{
	class PluginManager : public dku::model::Singleton<PluginManager>
	{
	public:
		struct PluginIndex
		{
			std::string fileName;
			std::string filePath;

			PluginInfo info;

			HMODULE base{ nullptr };
			DFPE_Load_func func{ nullptr };
		};

		[[nodiscard]] constexpr auto& get_plugins() const noexcept { return _plugins; }
		[[nodiscard]] constexpr auto get_loaded() const noexcept
		{
			return _plugins | std::views::take_while([](auto& p) { return p.base; });
		}
		[[nodiscard]] constexpr auto is_loading() const noexcept { return _isLoading; }
		[[nodiscard]] constexpr auto current_plugin() const noexcept { return _currentLoadingPlugin; }

		[[maybe_unused]] std::vector<PluginIndex>& Query() noexcept;
		[[maybe_unused]] std::vector<PluginIndex>& Install() noexcept;
		[[nodiscard]] PluginHandle LookupHandleFromName(std::string_view) noexcept;
		[[nodiscard]] std::string_view LookupNameFromHandle(PluginHandle) noexcept;

	private:
		friend dku::model::Singleton<PluginManager>;

		constexpr PluginManager() = default;

		std::vector<PluginIndex> _plugins{};
		bool _isLoading{ false };
		PluginHandle _currentLoadingPlugin{ 0 };

	};

	[[nodiscard]] std::string PrintPluginInfo(PluginInfo&) noexcept;

	[[nodiscard]] const void* GetLibExportAddress(const HMODULE a_module, const char* a_export) noexcept;
}  // namespace DFPE::API