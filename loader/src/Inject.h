#pragma once

#include "DFPE/Process.h"

namespace Inject
{
	bool InjectDLLRemoteThread(DFPE::SessionInfo* a_info) noexcept;

	std::filesystem::path GetDllPath() noexcept;
}  // namespace Inject