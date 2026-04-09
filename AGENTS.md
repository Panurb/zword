# AGENTS

Top-down 2D zombie survival game written in pure C using SDL2. Internal project name is "zword", published as "NotK". Targets Windows (MSVC x64) and Web (Emscripten/WASM).

## Build

Windows (MSVC):
```
cmake . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --target install --config Release
```

Emscripten:
```
emcmake cmake . -B build
cmake --build build
```

The LSP will report errors like `'SDL.h' file not found` because it doesn't know about CMake's include paths. These cascade into many false positives. Ignore them — if the build succeeds, the code is correct.

## Directory layout

```
CMakeLists.txt          Root build file
zword/src/              43 source files (.c)
zword/include/          42 headers (.h), 1:1 with src (except zword.c)
zword/data/             Assets: images/, maps/, prefabs/, sfx/, music/
zword/config.cfg        Runtime settings (resolution, keybinds, audio)
cJSON/                  Vendored cJSON library
SDL/ SDL_image/         Vendored SDL2 libs (prebuilt x64)
SDL_mixer/ SDL_ttf/
```

Prefabs are JSON files under `data/prefabs/` in subdirectories: `creatures/`, `objects/`, `tiles/`, `paths/`, `weapons/`. Maps are JSON under `data/maps/`.

## Architecture

Custom Entity-Component System. Entities are plain ints (max 4000, `NULL_ENTITY = -1`). Components are heap-allocated structs stored in pointer arrays (`ComponentData.physics[entity]`, etc.). NULL means the entity lacks that component. 22 component types defined in `component.h`.

Entity hierarchy via `CoordinateComponent.parent` / `.children`. Position, angle, and scale resolve recursively through the parent chain.

Fixed 60 Hz update timestep with interpolation for rendering. `CoordinateComponent` stores a `previous` transform (position, angle, scale) for lerping.

Key globals:
- `app` -- SDL window, controllers, delta time
- `game_data` -- components, camera, entity data
- `game_state` -- current state (menu, game, editor, etc.)
- `resources` -- loaded textures and sounds
- `game_settings` -- config.cfg values
- `network` -- multiplayer state (see AGENTS-networking.md)

System update order (in `game.c:update_game()`): coordinates, lifetimes, health, physics, collisions, waypoints, doors, players, weapons, enemies, energy, particles, lights, camera, weather, animation.

## Conventions

- Types: `PascalCase` (`CoordinateComponent`, `ColliderGrid`)
- Type methods: `PascalCase` (`ComponentData_create()`, `List_add()`)
- System/utility functions: `snake_case` (`update_physics()`, `create_entity()`)
- Constants/enums: `UPPER_SNAKE_CASE` (`MAX_ENTITIES`, `PLAYER_ON_FOOT`)
- Project includes use `"quotes"`, SDL/system use `<angle brackets>`
- Platform guards: only use `#ifdef` guards when a library is unavailable on a platform (e.g. Winsock2 vs POSIX sockets via `#ifdef _WIN32`). Do not use `__EMSCRIPTEN__` guards for networking code — POSIX sockets compile on Emscripten. `__EMSCRIPTEN__` guards are only for platform-specific APIs (IDBFS, canvas resize, `emscripten_set_main_loop`, etc.) and UI differences (hiding buttons that don't work on web). Do not guard code just to reduce web executable size. Format `#ifdef` blocks inline like normal `if` statements (not outdented to column 0).
- Always use braces with `if` statements. Exceptions: short `continue` conditions in `for` loops, and functions with lots of short return paths.
- No plain scopes — don't create blocks with braces unless preceded by `if`, `for`, `while`, `switch`, etc.
- Always `typedef` the `struct` keyword away (e.g. `typedef struct { ... } Foo;`, not bare `struct Foo { ... };`)
- Manual memory management (`malloc`/`free`), no tests or CI
- Logging via macros: `LOG_DEBUG`, `LOG_INFO`, `LOG_WARNING`, `LOG_ERROR`

## Key files

| File | Role |
|------|------|
| `zword.c` | Entry point, main loop, fixed timestep |
| `app.c` | SDL init, state machine, game loop orchestration |
| `game.c` | System update order, game mode logic (survival, campaign) |
| `component.c` | Entity creation/destruction, component add/get/remove |
| `input.c` | Controller/keyboard/mouse input, player state machines |
| `player.c` | Player system: states, inventory, attack, reload |
| `weapon.c` | Weapon firing, reloading, recoil, ammo |
| `enemy.c` | Enemy AI state machine, pathfinding, spawning |
| `collider.c` | Collision detection (circle, rect), collision matrix |
| `grid.c` | Spatial partitioning (416x416 uniform grid) |
| `serialize.c` | JSON serialization for maps, prefabs, saves |
| `camera.c` | World-to-screen transforms, all draw primitives |

## Networking

Multiplayer is experimental. See [AGENTS-networking.md](AGENTS-networking.md).

## Agent instructions

- After implementing a feature, stop and let the user test the change before continuing
- Keep `AGENTS.md` and `AGENTS-networking.md` up-to-date when making changes that affect their content
- Never edit data files under `zword/data/`, only source code
- Prefer plain `int`/`float` over more specific types like `int32_t`, `uint16_t`, etc.
