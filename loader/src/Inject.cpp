#include "Inject.h"

#pragma warning(push)
#pragma warning(disable: 6387)


namespace Inject
{
	bool InjectDLLRemoteThread(DFPE::SessionInfo* a_info) noexcept
	{
		// grab handle of created game process
		auto process = ::OpenProcess(
			PROCESS_ALL_ACCESS,
			TRUE,
			a_info->pi.dwProcessId);
		if (!process) {
			return false;
		}

		// safe because kernel32 is loaded at the same address in all processes
		// (can change across restarts)
		auto loadLibraryA = std::bit_cast<std::uintptr_t>(::GetProcAddress(::GetModuleHandleA("kernel32.dll"), "LoadLibraryA"));
		INFO("Process opened : {} with former id {}\n"
			"LoadLibraryA acquired : 0x{:X}",
			AsAddress(process), a_info->pi.dwProcessId, loadLibraryA);

		// allocate in game process space
		auto* mem = ::VirtualAllocEx(process, NULL, static_cast<std::size_t>(1) << 10, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
		if (!mem) {
			return false;
		}
		DEBUG("VirtualAlloc committed 1024 bytes for remote thread injection at 0x{:X}", AsAddress(mem));

		// build path
		std::size_t written{};
		auto path = GetDllPath().string();

		// write LoadLibraryA into allocated space
		if (!::WriteProcessMemory(process, mem, path.data(), path.size(), &written)) {
			return false;
		}
		DEBUG("Remote buffer written [{}] bytes: {}", written, path);

		auto thread = ::CreateRemoteThread(process, NULL, 0, LPTHREAD_START_ROUTINE(loadLibraryA), mem, NULL, NULL);
		if (!thread) {
			return false;
		}

		// hold for process injection & attach debugger
		switch (::WaitForSingleObject(thread, 1000 * 60))  // timeout = one minute
		{
		case WAIT_OBJECT_0:
			{
				INFO("Injection complete with thread set to {}", AsAddress(thread));
				break;
			}
		case WAIT_ABANDONED:
			{
				ERROR("DFPE : waiting for thread = WAIT_ABANDONED\n"
				"This is fatal! dfpe will not function correctly without injection.");
				break;
			}
		case WAIT_TIMEOUT:
			{
				ERROR("DFPE : waiting for thread = WAIT_TIMEOUT\n"
				"This could be caused by a plugin waiting for debugger to attach and has reached the timeout.\n"
				"Current timeout : {} seconds\n"
				"Default timeout could be changed in the dfpe_loader.json file.", 1000 * 60 / 1000);
				break;
			}
		}

		::CloseHandle(thread);

		::VirtualFreeEx(process, mem, 0, MEM_RELEASE);
		::CloseHandle(process);

		return true;
	}


	// very loose scanning lol
	std::filesystem::path GetDllPath() noexcept
	{
		auto dlls = dku::Config::GetAllFiles("plugins"sv, ".dll"sv, "dfpe_"sv);

		if (dlls.empty()) {
			ERROR("DFPE : Unable to find any dfpe dll inside [\\plugins] directory."
				"\nDid you delete it by accident?"
				"\nThis is fatal.\nLoader will now exit.");
		}

		return dlls.back();
	}
}  // namespace Inject

#pragma warning(pop)
