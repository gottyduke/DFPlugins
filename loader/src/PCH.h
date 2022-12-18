#pragma once

#include "DFPE/Prefix.h"

#include "Plugin.h"

#include "RE/Relocation.h"

#include "DKUtil/Logger.hpp"
#include "DKUtil/Config.hpp"
#include "DKUtil/Hook.hpp"

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)
