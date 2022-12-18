#pragma once

#include "Relocation.h"


#define DFPE_MAKE_VERSION(V, O) std::make_pair(std::string_view(###V), Relocation<const char*>(Offset(O)))


namespace RE
{
	enum class VERSION : std::uint32_t
	{
		k50_03,

		kError,
	};


	static std::array DFPE_VERSIONS = { 
		DFPE_MAKE_VERSION(50.03, 0x10BE4D8), // december 12, 2022 https://store.steampowered.com/news/app/975370/view/3631620553397903591
	};
}  // namespace RE