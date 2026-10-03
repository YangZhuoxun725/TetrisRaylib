cmake_minimum_required(VERSION 3.24)

# To rename the project, change the name here. Nothing else needs to change.
project(MyGame VERSION 0.1.0 LANGUAGES C CXX)

# ------------------------------------------------------------------------------
# Language settings
# ------------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for clangd / IDE tooling
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(USE_PCH "Use a precompiled header (src/pch.hpp) to speed up compilation" ON)

# ------------------------------------------------------------------------------
# Build configurations
#   Debug   - no optimisation, debug info, GAME_DEBUG defined
#   Release - optimised, console window kept (handy for logging)
#   Dist    - optimised, no console window, GAME_DIST defined. Ship this one.
# ------------------------------------------------------------------------------
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(IS_MULTI_CONFIG)
    set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Dist" CACHE STRING "" FORCE)
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# Compiler flags for the custom Dist config (set before the dependencies are
# added so raylib, Box2D, etc. are built with them too)
if(MSVC)
    set(CMAKE_C_FLAGS_DIST   "/O2 /Ob2 /DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DIST "/O2 /Ob2 /DNDEBUG" CACHE STRING "" FORCE)
else()
    set(CMAKE_C_FLAGS_DIST   "-O3 -DNDEBUG" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DIST "-O3 -DNDEBUG" CACHE STRING "" FORCE)
endif()

# ------------------------------------------------------------------------------
# Dependencies (raylib, fmt, Box2D, EnTT, glm, nlohmann_json, raygui)
# See cmake/Dependencies.cmake to add or change libraries.
# ------------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Dependencies)

# ------------------------------------------------------------------------------
# Executable: every .cpp/.c under src/ is picked up automatically
# ------------------------------------------------------------------------------
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

# Precompiled header (C++ files only). Toggle with -DUSE_PCH=ON/OFF.
# Source files include "pch.hpp" themselves, so the project builds either way.
if(USE_PCH)
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:${CMAKE_CURRENT_SOURCE_DIR}/src/pch.hpp>")
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    raylib
    raygui
    fmt::fmt
    box2d
    EnTT::EnTT
    glm::glm
    nlohmann_json::nlohmann_json
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLM_ENABLE_EXPERIMENTAL
    $<$<CONFIG:Debug>:GAME_DEBUG>
    $<$<CONFIG:Dist>:GAME_DIST>
)

# Warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /MP /utf-8)
    # Dist builds run without a console window
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Dist>:/SUBSYSTEM:WINDOWS>
        $<$<CONFIG:Dist>:/ENTRY:mainCRTStartup>)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Visual Studio: F5 starts this project, working directory = folder with the exe
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
)

# macOS system frameworks needed by raylib
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework IOKit" "-framework Cocoa" "-framework OpenGL")
endif()

# ------------------------------------------------------------------------------
# Assets: copy assets/ next to the executable after each build
# ------------------------------------------------------------------------------
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets"
)