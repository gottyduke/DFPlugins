#pragma once

#include "PluginManager.h"
#include "Interfaces.h"

#include "RE/Relocation.h"


// forward
void Init() noexcept;


namespace DFPE::Hooks
{
	struct Hook_ReleaseVersion
	{
		static constexpr auto ModReleaseText = "dfpe+"sv;

		static void Commit() noexcept
		{
			// release version info change
			auto dfVer = API::GetInterfaceInternal()->runtime_version();
			auto& [version, reloc] = RE::DFPE_VERSIONS[std::to_underlying(dfVer)];

			dku::Hook::WriteData(reloc.address(), ModReleaseText.data(), ModReleaseText.size(), false);

			INFO("ReleaseVersion {} modified at 0x{:X}\n", version, reloc.address());
		}
	};


	struct Hook_WinMain
	{
		static char* thunk()
		{
			Init();
			return func();
		}

		static inline RE::Relocation<decltype(thunk)> func;
		static constexpr auto ImportDLL = "api-ms-win-crt-runtime-l1-1-0.dll"sv;
		static constexpr auto ImportFunc = "_get_narrow_winmain_command_line"sv;

		static void Commit() noexcept
		{
			auto handle = dku::Hook::AddIATHook(DF_PROC, ImportDLL, ImportFunc, FUNC_INFO(thunk));
			func = handle->OldAddress;

			handle->Enable();
		}
	};




}  // namespace DFPE::Hooks


#undef HOOK_NAME
