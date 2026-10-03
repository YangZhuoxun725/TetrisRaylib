// Precompiled header.
//
// Put headers here that are (a) included in many files and (b) rarely change:
// the standard library and third-party libraries. Do NOT put your own game
// headers here, because every change would force a full rebuild.
//
// Every .cpp file should start with:  #include "pch.hpp"
// That keeps the project compiling whether USE_PCH is ON or OFF.
#pragma once

// ---- Standard library --------------------------------------------------------
#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// ---- Third-party -------------------------------------------------------------
#include <raylib.h>
// raygui is intentionally NOT here: its implementation (RAYGUI_IMPLEMENTATION)
// must be compiled in exactly one .cpp file. See main.cpp.

#include <box2d/box2d.h>
#include <entt/entt.hpp>
#include <fmt/core.h>
#include <fmt/format.h>
#include <glm/glm.hpp>
#include <nlohmann/json.hpp>