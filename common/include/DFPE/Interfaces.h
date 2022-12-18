#pragma once

#include "Prefix.h"

#include "DFPE/PluginAPI.h"
#include "RE/Versions.inl"

#include "DKUtil/Utility.hpp"

namespace DFPE
{
	static constexpr auto LOAD_FUNC_EXPORT_NAME = "DFPE_Load";

	struct Interface;

	// local for every plugin project, given at runtime by dfpe instance
	namespace storage
	{
		static inline Interface* LocalInterface{ nullptr };
		static inline PluginHandle LocalHandle{ static_cast<PluginHandle>(-1) };
	}


	struct Interface
	{
		enum
		{
			kInvalid = static_cast<PluginHandle>(-1),
			kDFPE = 0,
			kPacked = 1,
		};

		// built in dfpe message types
		enum Event
		{
			kPostLoad,    // all plugins loaded
			kDataLoaded,  // game resource loaded
			kNewGame,     // start new game
			kSaveGame,    // save game and return to menu
			kLoadGame,    // load game
			kDeleteGame,  // delete save
			kExitGame,    // game exited
		};

		struct Message
		{
			const char* sender;
			std::uint32_t type;
			std::uint32_t size;
			const void* data;
		};
		using GetHandle_func = std::add_pointer_t<PluginHandle(void)>;
		using EventCallback = std::add_pointer_t<void(Message)>;
		using RegisterListener_func = std::add_pointer_t<bool(PluginHandle, EventCallback, const char*)>;
		using Dispatch_func = std::add_pointer_t<bool(PluginHandle, std::uint32_t, std::uint32_t, const void*)>;

		constexpr Interface() = delete;
		explicit Interface(std::string_view a_dfpeVer, RE::VERSION a_runtime, GetHandle_func a_ghFunc, RegisterListener_func a_rlFunc, Dispatch_func a_dFunc) noexcept :
			_dfpeVersion(dku::string::lexical_cast<std::uint32_t>(dku::string::replace_all(a_dfpeVer, "-"sv, "."sv))),
			_runtimeVersion(a_runtime),
			_getHandle(a_ghFunc),
			_registerListener(a_rlFunc),
			_dispatch(a_dFunc)
		{}
		constexpr Interface(const Interface&) = delete;
		constexpr Interface(Interface&&) = delete;
		constexpr ~Interface() = default;

		[[nodiscard]] constexpr auto GetHandle() const noexcept
		{
			auto handle = _getHandle();
			return handle == kInvalid ? storage::LocalHandle : handle;
		}

		[[nodiscard]] constexpr auto RegisterListener(EventCallback a_handler, const char* a_sender = "dfpe") const noexcept
		{
			return _registerListener(storage::LocalHandle, a_handler, a_sender);
		}

		[[nodiscard]] constexpr auto Dispatch(std::uint32_t a_type, std::uint32_t a_size, const void* a_data) const noexcept
		{
			return _dispatch(storage::LocalHandle, a_type, a_size, a_data);
		}

		[[nodiscard]] constexpr std::uint32_t intfc_version() const noexcept { return _intfcVersion; }
		[[nodiscard]] constexpr std::uint32_t dfpe_version() const noexcept { return _dfpeVersion; }
		[[nodiscard]] constexpr RE::VERSION runtime_version() const noexcept { return _runtimeVersion; }

	protected:
		const std::uint32_t _intfcVersion = kPacked;
		const std::uint32_t _dfpeVersion;
		const RE::VERSION _runtimeVersion;  // plugin should attempt to match this idx within compiled sdk

		GetHandle_func _getHandle = nullptr;
		RegisterListener_func _registerListener = nullptr;
		Dispatch_func _dispatch = nullptr;
	};

	[[nodiscard]] static constexpr auto* GetInterface() noexcept
	{
		return storage::LocalInterface;
	}

	[[maybe_unused]] static constexpr auto* InitInterface(Interface* a_dfpe) noexcept
	{
		storage::LocalInterface = a_dfpe;
		storage::LocalHandle = a_dfpe->GetHandle();

		return storage::LocalInterface;
	}

	using DFPE_Load_func = std::add_pointer_t<bool(const Interface*)>;
}  // namespace DFPE
