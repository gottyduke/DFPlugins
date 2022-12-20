#include "DFPE/PluginAPI.h"
#include "DFPE/Interfaces.h"
#include "RE/Versions.inl"

#include "Hooks.h"
#include "SDL/Renderer.h"


namespace
{
	void MessageHandler(DFPE::Interface::Message a_msg) {
		if (a_msg.type == DFPE::Interface::Event::kPostLoad) {
			dku::Hook::default_trampoline()->create(dku::numbers::kilobyte(1));

			SDL::Renderer::GetSingleton();

			//Hooks::CreateSurfaceFromTextAtlas::Commit();
			Hooks::UpperBlit::Commit();
			Hooks::PostRenderUpdate::Commit();
		}
	}
}


DFPE_EXPORT_PLUGIN_INFO
{
	DFPE::PluginInfo info{};

	info.SetName(Plugin::NAME);
	info.SetVersion(Plugin::VERSION);
	info.SetAuthor("Dropkicker"sv);
	info.Support(RE::VERSION::k50_03);

	return info;
}();


DLLEXPORT bool DFPE_Load(DFPE::Interface* a_dfpe) noexcept
{
	std::locale::global(std::locale("en_US.UTF-8"));

	dku::Logger::Init();

	const auto* dfpe = DFPE::InitInterface(a_dfpe);
	if (dfpe->RegisterListener(MessageHandler)) {
		INFO("MessageHandler registered successfully.");
		return true;
	}

	WARN("Failed to initialize messaing interface, this is fatal. Plugin will now unload!");
	return false;
}
