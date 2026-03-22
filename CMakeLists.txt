cmake_minimum_required(VERSION 3.16)

project(ByteDeck VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BYTEDECK_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
set(BYTEDECK_SDL2_ROOT "" CACHE PATH "Optional SDL2 installation root")

include(FetchContent)

if(BYTEDECK_SDL2_ROOT)
  list(PREPEND CMAKE_PREFIX_PATH "${BYTEDECK_SDL2_ROOT}")
endif()

find_package(SDL2 QUIET CONFIG)
if(NOT SDL2_FOUND)
  find_package(SDL2 REQUIRED)
endif()

find_package(tinyxml2 QUIET CONFIG)
if(NOT tinyxml2_FOUND)
  FetchContent_Declare(
    tinyxml2
    GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
    GIT_TAG 10.0.0
  )
  FetchContent_MakeAvailable(tinyxml2)
endif()

find_package(nlohmann_json QUIET CONFIG)
if(NOT nlohmann_json_FOUND)
  FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
  )
  FetchContent_MakeAvailable(nlohmann_json)
endif()

FetchContent_Declare(
  stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)
FetchContent_MakeAvailable(stb)

set(BYTEDECK_SOURCES
  src/main.cpp
  src/app/application.cpp
  src/core/gamelist_parser.cpp
  src/core/library_scanner.cpp
  src/data/models.cpp
  src/launch/launch_service.cpp
  src/platform/logger.cpp
  src/platform/paths.cpp
  src/platform/status_service.cpp
  src/platform/translation_catalog.cpp
  src/platform/user_settings.cpp
  src/ui/fixed_ui_renderer.cpp
  src/ui/image_texture.cpp
  src/ui/theme_font_renderer.cpp
  src/ui/theme_manager.cpp
  src/ui/text_renderer.cpp
  src/ui/screen_manager.cpp
  src/ui/screens/apps_screen.cpp
  src/ui/screens/game_browser_screen.cpp
  src/ui/screens/games_screen.cpp
  src/ui/screens/main_menu_screen.cpp
  src/ui/screens/placeholder_screen.cpp
  src/ui/screens/settings_screen.cpp
)

add_executable(bytedeck ${BYTEDECK_SOURCES})

target_include_directories(bytedeck
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${stb_SOURCE_DIR}
)

if(TARGET SDL2::SDL2)
  target_link_libraries(bytedeck PRIVATE SDL2::SDL2)
elseif(TARGET SDL2::SDL2-static)
  target_link_libraries(bytedeck PRIVATE SDL2::SDL2-static)
else()
  target_include_directories(bytedeck PRIVATE ${SDL2_INCLUDE_DIRS})
  target_link_libraries(bytedeck PRIVATE ${SDL2_LIBRARIES})
endif()

if(TARGET SDL2::SDL2main)
  target_link_libraries(bytedeck PRIVATE SDL2::SDL2main)
endif()

if(TARGET tinyxml2::tinyxml2)
  target_link_libraries(bytedeck PRIVATE tinyxml2::tinyxml2)
else()
  target_link_libraries(bytedeck PRIVATE tinyxml2)
endif()

if(TARGET nlohmann_json::nlohmann_json)
  target_link_libraries(bytedeck PRIVATE nlohmann_json::nlohmann_json)
endif()

target_compile_definitions(bytedeck
  PRIVATE
    SDL_MAIN_HANDLED
    $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

if(MSVC)
  target_compile_options(bytedeck PRIVATE /W4)
  if(BYTEDECK_WARNINGS_AS_ERRORS)
    target_compile_options(bytedeck PRIVATE /WX)
  endif()
else()
  target_compile_options(bytedeck PRIVATE -Wall -Wextra -Wpedantic)
  if(BYTEDECK_WARNINGS_AS_ERRORS)
    target_compile_options(bytedeck PRIVATE -Werror)
  endif()
endif()
