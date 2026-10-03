# Third-party dependencies, downloaded at configure time with FetchContent.
# Sources land in <build dir>/_deps and are only downloaded once per build dir.
#
# To add a library:
#   1. FetchContent_Declare(...) it below
#   2. add its name to FetchContent_MakeAvailable(...)
#   3. add its target to target_link_libraries(...) in the top-level CMakeLists.txt

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)              # show download progress
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# --- raylib: window, input, 2D/3D drawing, audio ------------------------------
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)

# --- fmt: formatting / logging ------------------------------------------------
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG        11.1.4
    GIT_SHALLOW    TRUE
)

# --- Box2D v3 (C API): 2D physics ---------------------------------------------
set(BOX2D_SAMPLES    OFF CACHE BOOL "" FORCE)
set(BOX2D_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(BOX2D_DOCS       OFF CACHE BOOL "" FORCE)
set(BOX2D_UNIT_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    box2d
    GIT_REPOSITORY https://github.com/erincatto/box2d.git
    GIT_TAG        v3.1.0
    GIT_SHALLOW    TRUE
)

# --- EnTT: entity-component-system --------------------------------------------
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY https://github.com/skypjack/entt.git
    GIT_TAG        v3.14.0
    GIT_SHALLOW    TRUE
)

# --- glm: vector / matrix math ------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
    GIT_SHALLOW    TRUE
)

# --- nlohmann/json: save files, configs, level data ---------------------------
set(JSON_BuildTests OFF CACHE INTERNAL "")
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)

# --- raygui: immediate-mode GUI (single header) -------------------------------
# SOURCE_SUBDIR points at a folder that doesn't exist, so CMake downloads the
# repo but doesn't try to build it. We expose the header ourselves below.
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG        4.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  _no_cmake_here
)

FetchContent_MakeAvailable(raylib fmt box2d EnTT glm nlohmann_json raygui)

add_library(raygui INTERFACE)
target_include_directories(raygui SYSTEM INTERFACE "${raygui_SOURCE_DIR}/src")
