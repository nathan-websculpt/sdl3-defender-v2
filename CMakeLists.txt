cmake_minimum_required(VERSION 4.2)
project(SDL3Defender LANGUAGES CXX)

if (POLICY CMP0207)
    cmake_policy(SET CMP0207 NEW)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(SDL3DEFENDER_BUILD_TESTS "Build unit tests" ON)
option(SDL3DEFENDER_MSVC_DIAGNOSTICS "Enable opt-in MSVC diagnostics lane" OFF)

# Collect sources
file(GLOB_RECURSE GAME_SOURCES
    "${CMAKE_SOURCE_DIR}/core/*.cpp"
    "${CMAKE_SOURCE_DIR}/core/**/*.cpp"
    "${CMAKE_SOURCE_DIR}/entities/*.cpp"
    "${CMAKE_SOURCE_DIR}/entities/**/*.cpp"
)
add_library(${PROJECT_NAME}Core STATIC ${GAME_SOURCES})
target_include_directories(${PROJECT_NAME}Core
    PUBLIC
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/external"
)

add_executable(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}Core)

function(sdl3defender_enable_msvc_diagnostics target_name)
    if (NOT SDL3DEFENDER_MSVC_DIAGNOSTICS OR NOT MSVC)
        return()
    endif()

    target_compile_options(${target_name} PRIVATE
        /W4
        /analyze
        /external:W0
        "/external:I${CMAKE_SOURCE_DIR}/external"
    )
endfunction()

sdl3defender_enable_msvc_diagnostics(${PROJECT_NAME}Core)
sdl3defender_enable_msvc_diagnostics(${PROJECT_NAME})

# SDL3
find_package(SDL3 REQUIRED CONFIG)

# Fix for missing SDL3::Headers target (found in some distros)
if (NOT TARGET SDL3::Headers)
    add_library(SDL3::Headers INTERFACE IMPORTED)
    if (DEFINED SDL3_INCLUDE_DIRS)
        target_include_directories(SDL3::Headers INTERFACE ${SDL3_INCLUDE_DIRS})
    endif()
    message(WARNING "SDL3::Headers target not provided by your SDL3 package — created manually.")
endif()

# SDL3_image & SDL3_ttf
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)
find_package(SDL3_mixer REQUIRED CONFIG)

target_link_libraries(${PROJECT_NAME}Core
    PUBLIC SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    SDL3_mixer::SDL3_mixer
)

if (WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND_EXPAND_LISTS
        COMMENT "Copying runtime DLLs -> $<TARGET_FILE_DIR:${PROJECT_NAME}>"
    )
endif()

# Filesystem support for older GCC versions
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
    #include <filesystem>
    int main() { std::filesystem::path p{\".\"}; return 0; }
" HAS_FILESYSTEM)

if (NOT HAS_FILESYSTEM)
    message(STATUS "Adding stdc++fs library for filesystem support")
    target_link_libraries(${PROJECT_NAME}Core PUBLIC stdc++fs)
endif()

# Look for asset directories relative to project root
set(POSSIBLE_ASSET_DIRS
    "${CMAKE_SOURCE_DIR}/assets"
    "${CMAKE_SOURCE_DIR}/resources"
    "${CMAKE_SOURCE_DIR}/data"
)

set(FOUND_ASSET_DIRS "")
foreach(dir IN LISTS POSSIBLE_ASSET_DIRS)
    if (EXISTS "${dir}")
        list(APPEND FOUND_ASSET_DIRS "${dir}")
        message(STATUS "Asset dir found: ${dir}")
    endif()
endforeach()

if (FOUND_ASSET_DIRS)
    foreach(src_dir IN LISTS FOUND_ASSET_DIRS)
        get_filename_component(dir_name ${src_dir} NAME)
        set(dst_dir "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${dir_name}")

        # Copy AFTER the executable is built (no cyclic dependency)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${dst_dir}"
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${src_dir}" "${dst_dir}"
            COMMENT "Copying ${dir_name} -> ${dst_dir}"
        )
    endforeach()
else()
    message(WARNING "No asset directories (assets/, resources/, data/) were found in the project root (${CMAKE_SOURCE_DIR}).")
endif()

if (SDL3DEFENDER_BUILD_TESTS)
    enable_testing()
    find_package(GTest CONFIG REQUIRED)
    include(GoogleTest)

    add_executable(${PROJECT_NAME}_tests
        "${CMAKE_SOURCE_DIR}/tests/test_support.h"
        "${CMAKE_SOURCE_DIR}/tests/test_geometry_and_clipping.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_projectile_and_particle.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_health_item.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_colony_update_and_prune.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_collision_handler.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_high_scores_ranking.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_high_scores_persistence.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_game_input_transitions.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_opponents.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_rng.cpp"
        "${CMAKE_SOURCE_DIR}/tests/test_sound_manager_lifecycle.cpp"
    )

    target_link_libraries(${PROJECT_NAME}_tests
        PRIVATE
            ${PROJECT_NAME}Core
            GTest::gtest
            GTest::gtest_main
    )
    sdl3defender_enable_msvc_diagnostics(${PROJECT_NAME}_tests)
    if (MSVC AND SDL3DEFENDER_MSVC_DIAGNOSTICS)
        target_compile_options(${PROJECT_NAME}_tests PRIVATE /analyze-)
    endif()

    gtest_discover_tests(${PROJECT_NAME}_tests DISCOVERY_TIMEOUT 60)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_runtime_deps
    RUNTIME DESTINATION .
)

install(RUNTIME_DEPENDENCY_SET ${PROJECT_NAME}_runtime_deps
    DESTINATION .
    PRE_EXCLUDE_REGEXES
        "api-ms-.*"
        "ext-ms-.*"
    POST_EXCLUDE_REGEXES
        ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][\\\\/][Ss][Yy][Ss][Tt][Ee][Mm]32[\\\\/].*"
)

if (EXISTS "${CMAKE_SOURCE_DIR}/assets")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/assets" DESTINATION .)
endif()

if (EXISTS "${CMAKE_SOURCE_DIR}/resources")
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/resources" DESTINATION .)
endif()


# Platform tweaks
if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES)
endif()

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.sdl3defender"
    )
endif()
