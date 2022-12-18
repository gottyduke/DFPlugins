#include "DFPE/PluginAPI.h"
#include "DFPE/Interfaces.h"


DFPE_EXPORT_PLUGIN_INFO
{
	DFPE::PluginInfo info{};

	info.SetName(Plugin::NAME);
	info.SetVersion(Plugin::VERSION);
	info.SetAuthor("ModAuthor"sv);
	info.SupportLatest();

	return info;
}();


DLLEXPORT bool DFPE_Load(DFPE::Interface* a_dfpe) noexcept
{
	std::locale::global(std::locale("en_US.UTF-8"));

	dku::Logger::Init();

	// do stuff

	return false;
}
