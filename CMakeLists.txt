# Knight Engine 2D - CMake Build Configuration
# Supports macOS and Windows

cmake_minimum_required(VERSION 3.16)
project(knight_engine_2d
    VERSION 0.1.0
    LANGUAGES C
)

# Use C11 standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Output binary to project root for easy access to assets folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Source files - will grow as we modularize
set(SOURCES
    src/main.c
    src/core/game_logic.c
    src/graphics/camera.c
    src/graphics/renderer.c
    src/graphics/sprite.c
    src/graphics/texture.c
    src/input/input.c
    src/util/debug.c
    src/util/timer.c
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Include src directory for module headers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Platform-specific SDL2 configuration
if(APPLE)
    # macOS - Use Homebrew SDL2 installation
    # Install with: brew install sdl2 sdl2_image

    # Find SDL2 using pkg-config or CMake modules
    find_package(PkgConfig QUIET)

    if(PkgConfig_FOUND)
        pkg_check_modules(SDL2 REQUIRED sdl2)
        pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

        target_include_directories(${PROJECT_NAME} PRIVATE
            ${SDL2_INCLUDE_DIRS}
            ${SDL2_IMAGE_INCLUDE_DIRS}
        )

        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${SDL2_LIBRARIES}
            ${SDL2_IMAGE_LIBRARIES}
        )

        target_link_directories(${PROJECT_NAME} PRIVATE
            ${SDL2_LIBRARY_DIRS}
            ${SDL2_IMAGE_LIBRARY_DIRS}
        )
    else()
        # Fallback to framework-based linking
        find_library(SDL2_LIBRARY SDL2 REQUIRED)
        find_library(SDL2_IMAGE_LIBRARY SDL2_image REQUIRED)

        target_include_directories(${PROJECT_NAME} PRIVATE
            /usr/local/include
            /opt/homebrew/include
        )

        target_link_libraries(${PROJECT_NAME} PRIVATE
            ${SDL2_LIBRARY}
            ${SDL2_IMAGE_LIBRARY}
        )
    endif()

elseif(WIN32)
    # Windows - Expects SDL2 in a vendor folder or system path
    # Download SDL2 development libraries from:
    # https://github.com/libsdl-org/SDL/releases
    # https://github.com/libsdl-org/SDL_image/releases

    # Option 1: Set SDL2_DIR to your SDL2 CMake folder
    # Option 2: Place SDL2 in a vendor/ folder

    # Try to find SDL2 using CMake config files
    find_package(SDL2 CONFIG QUIET)
    find_package(SDL2_image CONFIG QUIET)

    if(SDL2_FOUND AND SDL2_image_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE
            SDL2::SDL2
            SDL2::SDL2main
            SDL2_image::SDL2_image
        )
    else()
        # Manual configuration for vendor folder setup
        set(SDL2_VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor/SDL2")
        set(SDL2_IMAGE_VENDOR_DIR "${CMAKE_SOURCE_DIR}/vendor/SDL2_image")

        if(EXISTS ${SDL2_VENDOR_DIR})
            target_include_directories(${PROJECT_NAME} PRIVATE
                ${SDL2_VENDOR_DIR}/include
                ${SDL2_IMAGE_VENDOR_DIR}/include
            )

            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(SDL2_LIB_DIR ${SDL2_VENDOR_DIR}/lib/x64)
                set(SDL2_IMAGE_LIB_DIR ${SDL2_IMAGE_VENDOR_DIR}/lib/x64)
            else()
                set(SDL2_LIB_DIR ${SDL2_VENDOR_DIR}/lib/x86)
                set(SDL2_IMAGE_LIB_DIR ${SDL2_IMAGE_VENDOR_DIR}/lib/x86)
            endif()

            target_link_directories(${PROJECT_NAME} PRIVATE
                ${SDL2_LIB_DIR}
                ${SDL2_IMAGE_LIB_DIR}
            )

            target_link_libraries(${PROJECT_NAME} PRIVATE
                SDL2
                SDL2main
                SDL2_image
            )
        else()
            message(FATAL_ERROR
                "SDL2 not found. Please either:\n"
                "  1. Install SDL2 and set SDL2_DIR/SDL2_image_DIR CMake variables\n"
                "  2. Place SDL2 dev libraries in vendor/SDL2 and vendor/SDL2_image"
            )
        endif()
    endif()

    # Windows-specific: Use WinMain entry point
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )

else()
    # Linux and other Unix-like systems
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL2 REQUIRED sdl2)
    pkg_check_modules(SDL2_IMAGE REQUIRED SDL2_image)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
    )
endif()

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# AddressSanitizer option for memory leak detection
# Usage: cmake -DENABLE_ASAN=ON ..
option(ENABLE_ASAN "Enable AddressSanitizer for memory leak detection" OFF)
if(ENABLE_ASAN)
    if(MSVC)
        message(WARNING "AddressSanitizer on MSVC requires VS 2019+ and /fsanitize=address")
        target_compile_options(${PROJECT_NAME} PRIVATE /fsanitize=address)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    endif()
    message(STATUS "AddressSanitizer: ENABLED")
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Knight Engine 2D Configuration:")
message(STATUS "  Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C Compiler: ${CMAKE_C_COMPILER_ID}")
message(STATUS "")
