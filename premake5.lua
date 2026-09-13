workspace "TetrisRaylib"
    architecture "x64"
    startproject "Tetris"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["raylib"] = "Tetris/vendor/raylib/src"

include "Tetris/vendor/raylib"

project "Tetris"
    location "Tetris"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.cpp",
    }

    includedirs
    {
        "%{prj.name}/src",
        "%{IncludeDir.raylib}"
    }

    links
    {
        "raylib"
    }

    filter "system:windows"
        systemversion "latest"
        buildoptions { "/utf-8" }

        defines
        {

        }

    filter "configurations:Debug"
        symbols "On"
        optimize "Debug"

    filter "configurations:Release"
        symbols "Off"
        optimize "On"

    filter "configurations:Dist"
        symbols "Off"
        optimize "Speed"
    