cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_FLAGS "-O3")

project(notk C)

include_directories(CSFML/include)

include_directories(cJSON/include)

include_directories(zword/include)

include_directories(SDL/include)

include_directories(SDL_image/include)

include_directories(SDL_ttf/include)

include_directories(SDL_mixer/include)

set(SOURCES
    cJSON/src/cJSON.c
    zword/src/animation.c
    zword/src/building.c
    zword/src/camera.c
    zword/src/collider.c
    zword/src/component.c
    zword/src/door.c
    zword/src/editor.c
    zword/src/enemy.c
    zword/src/game.c
    zword/src/grid.c
    zword/src/health.c
    zword/src/heap.c
    zword/src/hud.c
    zword/src/image.c
    zword/src/item.c
    zword/src/input.c
    zword/src/interface.c
    zword/src/level.c
    zword/src/light.c
    zword/src/list.c
    zword/src/menu.c
    zword/src/navigation.c
    zword/src/object.c
    zword/src/particle.c
    zword/src/perlin.c
    zword/src/physics.c
    zword/src/player.c
    zword/src/raycast.c
    zword/src/road.c
    zword/src/serialize.c
    zword/src/settings.c
    zword/src/sound.c
    zword/src/tile.c
    zword/src/util.c
    zword/src/zword.c
    zword/src/vehicle.c
    zword/src/weapon.c
    zword/src/widget.c
    zword/src/outline.frag
)

# Enable console by removing WIN32 flag
add_executable(notk ${SOURCES})

install(TARGETS notk
    DESTINATION ${CMAKE_BINARY_DIR})

set(LIBS
    ${CMAKE_SOURCE_DIR}/CSFML/lib/msvc/csfml-audio.lib
	${CMAKE_SOURCE_DIR}/CSFML/lib/msvc/csfml-graphics.lib
	${CMAKE_SOURCE_DIR}/CSFML/lib/msvc/csfml-system.lib
	${CMAKE_SOURCE_DIR}/CSFML/lib/msvc/csfml-window.lib
    ${CMAKE_SOURCE_DIR}/SDL/lib/x86/SDL2.lib
    ${CMAKE_SOURCE_DIR}/SDL/lib/x86/SDL2main.lib
    ${CMAKE_SOURCE_DIR}/SDL_image/lib/x86/SDL2_image.lib
    ${CMAKE_SOURCE_DIR}/SDL_ttf/lib/x86/SDL2_ttf.lib
    ${CMAKE_SOURCE_DIR}/SDL_mixer/lib/x86/SDL2_mixer.lib
)

target_link_libraries(notk ${LIBS})

set(DLLS
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-audio-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-graphics-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-system-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-window-2.dll
    ${CMAKE_SOURCE_DIR}/SDL/lib/x86/SDL2.dll
    ${CMAKE_SOURCE_DIR}/SDL_image/lib/x86/SDL2_image.dll
    ${CMAKE_SOURCE_DIR}/SDL_ttf/lib/x86/SDL2_ttf.dll
    ${CMAKE_SOURCE_DIR}/SDL_mixer/lib/x86/SDL2_mixer.dll
)

install(FILES ${DLLS}
    DESTINATION ${CMAKE_BINARY_DIR}
)

install(DIRECTORY zword/data
    DESTINATION ${CMAKE_BINARY_DIR}
    FILES_MATCHING 
        PATTERN "*.png"
        PATTERN "*.ttf"
        PATTERN "*.wav"
		PATTERN "*.json"
		PATTERN "*.ogg"
)

set(SHADERS
    ${CMAKE_SOURCE_DIR}/zword/src/outline.frag
)

install(FILES ${SHADERS}
    DESTINATION ${CMAKE_BINARY_DIR}
)

set(EXTERNAL_DLLS ${CMAKE_SOURCE_DIR}/OpenAL32.dll)

install(FILES ${EXTERNAL_DLLS}
    DESTINATION ${CMAKE_BINARY_DIR}
)

set(CONFIG 
    ${CMAKE_SOURCE_DIR}/zword/config.cfg
)

# install(FILES ${CONFIG}
#     DESTINATION ${CMAKE_BINARY_DIR}
# )
