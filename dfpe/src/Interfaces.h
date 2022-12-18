#pragma once

#include "PluginManager.h"

#include "DFPE/Interfaces.h"


namespace DFPE::API
{
	[[nodiscard]] Interface* GetInterfaceInternal() noexcept;

	[[nodiscard]] constexpr PluginHandle GetHandleInternal() noexcept;

	bool RegisterListenerInternal(PluginHandle a_listener, Interface::EventCallback a_handler, const char* a_sender) noexcept;

	bool DispatchInternal(PluginHandle a_sender, std::uint32_t a_type, std::uint32_t a_size = 0, const void* a_data = nullptr) noexcept;

	[[nodiscard]] Interface* GetInterfaceInternal() noexcept;

	void FinalizeBroadcasts() noexcept;
}  // namespace DFPE::API
