#include "DFPE/Process.h"

#include "Inject.h"


extern "C" int APIENTRY main([[maybe_unused]] int, [[maybe_unused]] char**)
{
	std::locale::global(std::locale("en_US.UTF-8"));

	dku::Logger::Init();

	// create game process suspended for injection
	DFPE::CloseGameProcess();
	DFPE::CreateGameProcess(CREATE_SUSPENDED);
	if (!Inject::InjectDLLRemoteThread(DFPE::GetSessionInfo())) {
		DFPE::CloseGameProcess();

		ERROR("DFPE : Unable to inject DLL remote thread for game process.\nThis is fatal!\nLoader will now exit.");
	} else {
		if (!::ResumeThread(DFPE::GetSessionInfo()->pi.hThread)) {
			WARN("DFPE : something has started the runtime outside of dfpe_loader's control.\n"
				"DFPE will probably not function correctly.\n"
				"Try running dfpe_loader as an administrator, or check for debugging plugins.");
		}

		DFPE::CloseGameHandle();
	}

	// idle
	INFO("Loader will now exit!");
}
