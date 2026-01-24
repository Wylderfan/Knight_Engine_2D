# Knight Engine 2D

A 2D game engine written in C using SDL2. Currently in early development.

## Features

- Hardware-accelerated rendering with VSYNC
- Sprite rendering with rotation and flipping
- Texture loading and caching (PNG support via SDL2_image)
- Camera system with world-to-screen coordinate conversion
- Input handling with edge detection (key pressed/released)
- Fixed timestep game loop for consistent physics
- Debug visualization (bounding boxes, FPS counter)
- Stress test mode for performance testing

## Controls

| Action | Keys |
|--------|------|
| Move | Arrow keys or WASD |
| Move camera | I, J, K, L |
| Toggle debug mode | P |
| Toggle stress test | T |
| Quit | ESC or Q |

## Building

### Requirements

- CMake 3.16 or higher
- C11 compatible compiler
- SDL2
- SDL2_image

### macOS

```bash
# Install dependencies via Homebrew
brew install sdl2 sdl2_image

# Build
mkdir build && cd build
cmake ..
make

# Run (from project root)
cd ..
./knight_engine_2d
```

### Linux (Ubuntu/Debian)

```bash
# Install dependencies
sudo apt-get install libsdl2-dev libsdl2-image-dev

# Build
mkdir build && cd build
cmake ..
make

# Run (from project root)
cd ..
./knight_engine_2d
```

### Linux (Fedora)

```bash
# Install dependencies
sudo dnf install SDL2-devel SDL2_image-devel

# Build
mkdir build && cd build
cmake ..
make

# Run (from project root)
cd ..
./knight_engine_2d
```

### Windows

**Option 1: vcpkg (Recommended)**
```powershell
# Install vcpkg and dependencies
vcpkg install sdl2 sdl2-image

# Build
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=[vcpkg root]/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

**Option 2: Manual Setup**

1. Download SDL2 and SDL2_image development libraries from:
   - https://github.com/libsdl-org/SDL/releases
   - https://github.com/libsdl-org/SDL_image/releases

2. Place in `vendor/` folder:
   ```
   vendor/
   ├── SDL2/
   │   ├── include/
   │   └── lib/x64/
   └── SDL2_image/
       ├── include/
       └── lib/x64/
   ```

3. Build:
   ```powershell
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

4. Copy SDL2.dll and SDL2_image.dll to the project root before running.

### Build Options

```bash
# Enable AddressSanitizer for memory leak detection
cmake -DENABLE_ASAN=ON ..

# Release build
cmake -DCMAKE_BUILD_TYPE=Release ..
```

## Project Structure

```
Knight_Engine_2D/
├── assets/                  # Game assets (textures, etc.)
│   ├── player.png
│   └── background.png
├── src/
│   ├── main.c              # Entry point
│   ├── core/
│   │   ├── config.h        # Engine configuration constants
│   │   ├── engine.c/h      # Engine init, cleanup, game loop
│   │   ├── game_logic.c/h  # Input processing, game updates
│   │   └── game_state.h    # Central game state structure
│   ├── graphics/
│   │   ├── camera.c/h      # Camera and coordinate conversion
│   │   ├── renderer.c/h    # SDL renderer wrapper
│   │   ├── sprite.c/h      # Sprite rendering
│   │   └── texture.c/h     # Texture loading and management
│   ├── input/
│   │   ├── input.c/h       # Input state and edge detection
│   │   └── input_config.h  # Key bindings
│   └── util/
│       ├── debug.c/h       # Debug drawing, stress test
│       └── timer.c/h       # FPS tracking utilities
├── CMakeLists.txt          # Build configuration
├── CLAUDE.md               # AI assistant instructions
└── README.md
```

## File Descriptions

### Core

| File | Description |
|------|-------------|
| `main.c` | Minimal entry point. Creates game state, calls engine_init, engine_run, engine_cleanup. |
| `core/config.h` | All engine configuration constants: window size, sprite settings, timing, colors, asset paths. |
| `core/engine.c/h` | Engine lifecycle: initialization (renderer, textures, sprites), cleanup, and main game loop with event handling and rendering. |
| `core/game_logic.c/h` | Game update logic: player input processing, camera movement, sprite updates, position clamping. |
| `core/game_state.h` | Central `game_state_t` structure holding all game data: renderer, textures, input, camera, sprites, debug state. |

### Graphics

| File | Description |
|------|-------------|
| `graphics/camera.c/h` | Camera position and `world_to_screen()` coordinate conversion. |
| `graphics/renderer.c/h` | SDL renderer wrapper: init, cleanup, clear, present, window title. Handles SDL and SDL_image initialization. |
| `graphics/sprite.c/h` | Sprite structure (position, velocity, size, rotation, texture) and rendering functions with camera support. |
| `graphics/texture.c/h` | Texture manager with caching. Loads PNG files, tracks dimensions, provides fallback colored textures. |

### Input

| File | Description |
|------|-------------|
| `input/input.c/h` | Input state tracking with edge detection: `input_key_down()` (held), `input_key_pressed()` (just pressed), `input_key_released()` (just released). |
| `input/input_config.h` | Key binding definitions for movement (arrows/WASD), camera (IJKL), and system keys (ESC, P, T). |

### Utilities

| File | Description |
|------|-------------|
| `util/debug.c/h` | Debug drawing functions (rectangles, rotated rectangles) and stress test toggle for spawning/despawning test sprites. |
| `util/timer.c/h` | FPS counter utilities for tracking frame rate over time. |

## Configuration

Edit `src/core/config.h` to customize:

- Window dimensions (`WINDOW_WIDTH`, `WINDOW_HEIGHT`)
- Sprite settings (`SPRITE_WIDTH`, `SPRITE_HEIGHT`, `SPRITE_SPEED`)
- Physics timing (`TARGET_FPS`, `FIXED_TIMESTEP`)
- Camera speed (`CAMERA_SPEED`)
- FPS display (`FPS_DISPLAY_ENABLED`, `FPS_DEBUG_LOG`)
- Colors (`COLOR_BG_*`, `COLOR_PLAYER_*`)
- Asset paths (`PLAYER_TEXTURE_PATH`, `BACKGROUND_TEXTURE_PATH`)

## License

[Add license information here]
