#include "PluginManager.h"
#include "Hooks.h"


using namespace dku::Alias;

void ReportSDKVersion() noexcept
{
	INFO("dfpe packed PluginInfo version : {}", DFPE::PluginInfo::kPacked);
	INFO("dfpe packed Interface version : {}", DFPE::Interface::kPacked);
}


void Init() noexcept
{
	std::locale::global(std::locale("en_US.UTF-8"));

	dku::Logger::Init();
	ReportSDKVersion();

	INFO("Game image base : 0x{:X}\n", dku::Hook::Module::get().base());

	auto dfVer = DFPE::API::GetInterfaceInternal()->runtime_version();
	if (dfVer == RE::VERSION::kError) {
		ERROR(
			"DFPE failed to match any available version from game file!\n"
			"The game might have been updated recently or source file modified externally.\n"
			"Please contact author or check for updates. DFPE will now exit!");
	}

	// hooks
	auto* tram = dku::Hook::default_trampoline();
	tram->create(dku::numbers::kilobyte(64));

	DFPE::Hooks::Hook_ReleaseVersion::Commit();

	DFPE::WaitForDebugger();

	// plugins
	auto* manager = DFPE::API::PluginManager::GetSingleton();
	manager->Query();
	manager->Install();

	// end
	FlushInstructionCache(GetCurrentProcess(), NULL, 0);
	INFO("DFPE has finished initializing!");

	DFPE::API::DispatchInternal(DFPE::Interface::kDFPE, DFPE::Interface::Event::kPostLoad);
}


BOOL APIENTRY DllMain([[maybe_unused]] HMODULE, DWORD a_ul_reason_for_call, [[maybe_unused]] LPVOID)
{
	if (a_ul_reason_for_call == DLL_PROCESS_ATTACH) {
		DFPE::Hooks::Hook_WinMain::Commit();
	}
	return TRUE;
}
