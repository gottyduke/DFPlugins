#pragma once

#include "DFPE/Prefix.h"

#include "Plugin.h"

#include "RE/Relocation.h"

#include "DKUtil/Logger.hpp"

#include <imgui_impl_sdl.h>
#include <imgui_impl_win32.h>

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)
