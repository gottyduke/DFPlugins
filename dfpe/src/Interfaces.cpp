#include "Interfaces.h"


namespace DFPE::API
{
	// PluginIndex is the loadOrder of plugin + 1
	// 0 : reserved for DFPE
	// -1 : kInvalid
	// [sender, [listener, handler]]
	std::map<std::uint64_t, std::set<std::pair<PluginHandle, Interface::EventCallback>>> Broadcasts;

	[[nodiscard]] constexpr PluginHandle GetHandleInternal() noexcept
	{
		const auto* manager = PluginManager::GetSingleton();
		// this is only expected during plugin loading phase
		if (!manager->is_loading()) {
			return Interface::kInvalid;
		}

		return manager->current_plugin();
	}

	bool RegisterListenerInternal(PluginHandle a_listener, Interface::EventCallback a_handler, const char* a_sender) noexcept
	{
		if (!a_handler || !a_sender) {
			INFO("Failed to register listener for plugin ({}) : {}", a_listener, PluginManager::GetSingleton()->LookupNameFromHandle(a_listener));
			return false;
		}

		std::string sender{ a_sender };
		dku::string::tolower(sender);

		Broadcasts[dku::numbers::FNV_1A_64(sender)].emplace(a_listener, a_handler);
		INFO("Registered listener ({}) for {} with callback at 0x{:X}", a_listener, a_sender, AsAddress(a_handler));
		
		return true;
	}

	bool DispatchInternal(PluginHandle a_sender, std::uint32_t a_type, std::uint32_t a_size, const void* a_data) noexcept
	{
		auto* manager = PluginManager::GetSingleton();

		std::string sender{ manager->LookupNameFromHandle(a_sender).data() };
		if (sender.empty()) {  // this should never happen because plugin has no way of manually dispatching with a different local handle
			INFO("Failed to dispatch message from sender ({}) - invalid handle", a_sender);
			return false;
		}
		dku::string::tolower(sender);

		auto& listener = Broadcasts[dku::numbers::FNV_1A_64(sender)];
		for (auto& [name, handler] : listener) {
			handler({ sender.data(), a_type, a_size, a_data });
			INFO("{} is dispatching message type {} with size of {} to {}", sender, a_type, a_size, manager->LookupNameFromHandle(name));
		}

		return true;
	}

	[[nodiscard]] Interface* GetInterfaceInternal() noexcept
	{
		static Interface instance{
			Plugin::VERSION,
			IdentifyVersion(),
			GetHandleInternal,
			RegisterListenerInternal, 
			DispatchInternal
		};
		return std::addressof(instance);
	}

	void FinalizeBroadcasts() noexcept
	{

	}
}  // namespace DFPE::API
