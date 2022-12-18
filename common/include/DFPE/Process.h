#pragma once

#include "Prefix.h"

#include "RE/Versions.inl"

#include "DKUtil/Logger.hpp"
#include "DKUtil/Hook.hpp"
#include "DKUtil/Utility.hpp"


#pragma warning(push)
#pragma warning(disable: 6335)


namespace DFPE
{
	static constexpr auto DF_PROC = "Dwarf Fortress.exe"sv;

	struct SessionInfo
	{
		SessionInfo()
		{
			dku::memzero(&si);
			si.cb = sizeof(si);
			dku::memzero(&pi);
		}

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
	};

	[[nodiscard]] inline SessionInfo* GetSessionInfo() noexcept
	{
		static SessionInfo info;
		return std::addressof(info);
	}

	[[nodiscard]] inline bool CreateGameProcess(DWORD a_flag = NULL) noexcept
	{
		// check file path
		std::filesystem::path dfProcPath = std::filesystem::current_path() / DF_PROC;

		if (!std::filesystem::exists(dfProcPath)) {
			ERROR("Unable to find [{}]! Did you install dfpe in the game root folder?\n"
				"This could happen due to renamed game runtime exectuable!\n"
				"Current dfpe location: {}\n", DF_PROC, std::filesystem::current_path().string());
		}

		INFO("Game file path: {}", dfProcPath.string());

		// start game
		auto* info = GetSessionInfo();
		if (!::CreateProcess(
				dfProcPath.c_str(),
				NULL,    // no args
				NULL,    // default process security
				NULL,    // default thread security
				FALSE,   // don't inherit handles
				a_flag,  // CREATE_SUSPEND
				NULL,    // no new environment
				NULL,    // no new cwd
				&info->si, &info->pi)) {
			ERROR("Launching {} failed with ERRORCODE {}", dfProcPath.string().data(), ::GetLastError());
			return false;
		}

		INFO("Main thread id : {}", info->pi.dwThreadId);
	}

	// always within range of compiled dfpe sdk
	[[nodiscard]] inline RE::VERSION IdentifyVersion() noexcept
	{
		for (auto idx = 0; idx < RE::DFPE_VERSIONS.size(); ++idx) {
			auto& [version, reloc] = RE::DFPE_VERSIONS[idx];

#ifndef DFPE_NO_SDK_CHECK
			// check for sdk integrity
			DEBUG("Performing sdk integrity check...");

			auto sdkTblName = dku::print_enum(RE::VERSION(idx));
			auto sdkAryName = dku::string::replace_all(version, "."sv, "_"sv);

			DEBUG("TblName : {} | AryName : {}", sdkTblName, sdkAryName);
#endif

			if (!dku::string::iends_with(sdkTblName, sdkAryName)) {
				DEBUG("Mismatch\n");
				ERROR("Fatal error occurred identifying game version!\n"
				"Internal sdk version table and reloc table does not match!\n"
				"This is likely due to a wrongly modified SDK source file.\n"
				"If you are developing plugins, please check the file integrity of [common\\RE\\Versions.inl]\n"
				"DFPE will now exit!");
			}
			DEBUG("Match\n");

			if (dku::string::iequals(version, reloc.get())) {
				return RE::VERSION(idx);
			}
		}

		return RE::VERSION::kError;
	}

	// subset
	inline void CloseGameHandle() noexcept
	{
		auto& pi = GetSessionInfo()->pi;

		::CloseHandle(pi.hProcess);
		::CloseHandle(pi.hThread);
	}

	inline void CloseGameProcess() noexcept
	{
		auto snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
		PROCESSENTRY32 entry{};
		entry.dwSize = sizeof(entry);

		auto res = ::Process32First(snapshot, &entry);
		while (res) {
			if (dku::string::iequals(dku::string::to_string(entry.szExeFile), DF_PROC)) {
				auto process = ::OpenProcess(PROCESS_TERMINATE, 0, entry.th32ProcessID);
				if (process) {
					::TerminateProcess(process, 9);
					::CloseHandle(process);
				}
			}
			res = ::Process32Next(snapshot, &entry);
		}

		::CloseHandle(snapshot);
		CloseGameHandle();
	}
}  // namespace DFPE::Process

#pragma warning(pop)