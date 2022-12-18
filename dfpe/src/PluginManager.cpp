#include "PluginManager.h"
#include "Interfaces.h"

#include "DKUtil/Utility.hpp"


namespace DFPE::API
{
	void Sanitize(PluginManager::PluginIndex* a_index) noexcept
	{
		a_index->info.name[sizeof(a_index->info.name) - 1] = 0;
		a_index->info.version[sizeof(a_index->info.version) - 1] = 0;
		a_index->info.author[sizeof(a_index->info.author) - 1] = 0;
	}

	auto PluginManager::Query() noexcept
		-> std::vector<PluginIndex>&
	{
		INFO("Querying plugins...");
		auto files = dku::Config::GetAllFiles("plugins"sv, ".dll"sv) | std::views::drop_while([](auto file) { return dku::string::istarts_with(file.filename().string(), "dfpe_"sv); });

		auto loadOrder{ 0 };
		for (auto& file : files) {
			auto log = fmt::format("Plugin ({}) : {}", ++loadOrder, file.filename().string());
			INFO("{}", log);

			auto plugin = ::LoadLibraryExA(file.string().c_str(), NULL, LOAD_LIBRARY_AS_IMAGE_RESOURCE);
			if (plugin) {
				DEBUG("Loaded as image resource for querying.");

				auto* info = std::bit_cast<const PluginInfo*>(GetLibExportAddress(plugin, PLUGIN_INFO_EXPORT_NAME));
				if (info) {
					DEBUG("Info structure acquired.");

					// dfpe distributable will always be compiled with the latest sdk
					if (info->infoVersion < PluginInfo::kPacked) {
						// outdated plugin regression stuff here
						WARN(
							"CAUTION; {} is compiled with an outdated DFPE SDK.\n"
							"This could lead to incompatible behavior!\n"
							"Plugin SDK version : {} | DFPE SDK version : {}",
							log, info->infoVersion, PluginInfo::kPacked);
					}

					if (info->supportedVersion >= GetInterfaceInternal()->runtime_version()) {
						DEBUG("packed version : {} | supported version : {}", info->infoVersion, dku::print_enum(info->supportedVersion));

						// save to global plugins storage
						_plugins.emplace_back(file.filename().string(), file.string(), *info);

						continue;
					} else {
						WARN(
							"Skipped; {} does not support this version of {}!\n"
							"Expected : {} | Running : {}",
							log, DF_PROC, dku::print_enum(info->supportedVersion),
							dku::print_enum(GetInterface()->runtime_version()));
					}
				} else {
					WARN("Skipped; {} does not export valid info structure.", log);
				}
			} else {
				WARN("Skipped; {} is not a valid dll plugin.", log);
			}

			::FreeLibrary(plugin);
			DEBUG("Unloaded image resource from mem.");
		}

		INFO("Query complete, {} / {} plugins reported as compatible.\n", _plugins.size(), files.size());

		return _plugins;
	}

	auto PluginManager::Install() noexcept
		-> std::vector<PluginIndex>&
	{
		INFO("Installing plugins...");

		_isLoading = true;

		auto loadOrder{ 0 };
		auto succeeded{ 0 };
		for (auto& plugin : _plugins) {
			auto log = fmt::format("plugin {} : {}", ++loadOrder, plugin.fileName);
			_currentLoadingPlugin = loadOrder;
			INFO("Loading {}", log);

			plugin.base = ::LoadLibraryA(plugin.filePath.data());
			if (plugin.base) {
				plugin.func = std::bit_cast<decltype(plugin.func)>(::GetProcAddress(plugin.base, LOAD_FUNC_EXPORT_NAME));
				if (plugin.func) {
					Sanitize(std::addressof(plugin));

					auto success = plugin.func(GetInterfaceInternal());
					if (success) {
						INFO("Succeeded; Base at 0x{:X}\n{}", AsAddress(plugin.base), PrintPluginInfo(plugin.info));

						++succeeded;
						continue;
					} else {
						WARN(
							"{} reported false during loading!\n"
							"Please check plugin log for details.\n"
							"This is not an error, plugin will be unloaded.",
							log);
					}
				} else {
					WARN("Disabled {}; Does not export valid load function.", log);
				}

				::FreeLibrary(plugin.base);
				plugin.base = nullptr;
			} else {
				WARN("Disabled {}; Cannot be loaded as DLL!", log);
			}
		}

		INFO("Install complete, {} / {} plugins loaded.\n", succeeded, _plugins.size());

		_isLoading = false;
		_currentLoadingPlugin = 0;

		return _plugins;
	}

	[[nodiscard]] PluginHandle PluginManager::LookupHandleFromName(std::string_view a_pluginName) noexcept
	{
		if (dku::string::iequals(a_pluginName, "dfpe"sv)) {
			return Interface::kDFPE;
		}

		auto& plugins = get_plugins();
		for (auto idx = 0; idx < plugins.size(); ++idx) {
			if (dku::string::iequals(a_pluginName, plugins[idx].info.name)) {
				return plugins[idx].base ? ++idx : Interface::kInvalid;
			}
		}

		return Interface::kInvalid;
	}

	[[nodiscard]] std::string_view PluginManager::LookupNameFromHandle(PluginHandle a_handle) noexcept
	{
		if (a_handle > 0 && a_handle <= _plugins.size()) {
			return _plugins[a_handle - 1].info.name;
		} else if (a_handle == 0) {
			return "dfpe"sv;
		} else {
			return {};
		}
	}

	std::string PrintPluginInfo(PluginInfo& a_info) noexcept
	{
		return fmt::format(
			"name    : {}\n"
			"version : {}\n"
			"author  : {}\n"
			"target  : {}",
			a_info.name, a_info.version, a_info.author,
			dku::print_enum(a_info.supportedVersion));
	}

	[[nodiscard]] const void* GetLibExportAddress(HMODULE a_module, const char* a_export) noexcept
	{
		auto& module = dku::Hook::Module::get(AsAddress(a_module));
		auto* dosHeader = AsPointer(module.dosHeader());
		auto* exportTable = dku::adjust_pointer<const ::IMAGE_EXPORT_DIRECTORY>(dosHeader, module.ntHeader()->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		auto* exportAddresses = dku::adjust_pointer<const std::uint32_t>(dosHeader, exportTable->AddressOfFunctions);        // RVA array
		auto* exportNameOrdinals = dku::adjust_pointer<const std::uint16_t>(dosHeader, exportTable->AddressOfNameOrdinals);  // index in to exportNames
		auto* exportNames = dku::adjust_pointer<const std::uint32_t>(dosHeader, exportTable->AddressOfNames);                // RVA array

		const void* result = nullptr;

		for (auto idx = 0; idx < exportTable->NumberOfFunctions; ++idx) {
			std::uint32_t nameOrdinal = exportNameOrdinals[idx];
			if (nameOrdinal < exportTable->NumberOfNames) {
				std::uint32_t nameRVA = exportNames[nameOrdinal];
				auto* name = dku::adjust_pointer<const char>(dosHeader, nameRVA);

				if (dku::string::iequals(a_export, name)) {
					std::uint32_t addrRVA = exportAddresses[idx];
					result = dku::adjust_pointer(dosHeader, addrRVA);

					break;
				}
			}
		}

		return result;
	}
}  // DFPE::API