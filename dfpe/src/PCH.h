#pragma once

#include "DFPE/Prefix.h"

#include "Plugin.h"

#include "DFPE/Process.h"
#include "RE/Relocation.h"

#include "DKUtil/Logger.hpp"
#include "DKUtil/Utility.hpp"
#include "DKUtil/Config.hpp"
#include "DKUtil/Hook.hpp"

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)
