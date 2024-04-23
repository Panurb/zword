cmake_minimum_required(VERSION 3.18)

project(notk C)

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")  
    set(USE_FLAGS "-s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS=\"[\"\"png\"\"]\" -s USE_SDL_TTF=2 -s USE_SDL_MIXER=2")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

include_directories(cJSON/include)
include_directories(zword/include)

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")  
    include_directories(SDL/include)
    include_directories(SDL_image/include)
    include_directories(SDL_ttf/include)
    include_directories(SDL_mixer/include)
endif()

set(SOURCES
    cJSON/src/cJSON.c
    zword/src/animation.c
    zword/src/app.c
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

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")  
    add_executable(notk ${SOURCES})
    set_target_properties(notk PROPERTIES LINK_FLAGS "--preload-file=data -s ALLOW_MEMORY_GROWTH=1")

    file(COPY zword/data
        DESTINATION ${CMAKE_BINARY_DIR}
        FILES_MATCHING 
            PATTERN "*.png"
            PATTERN "*.ttf"
            PATTERN "*.wav"
            PATTERN "*.json"
            PATTERN "*.ogg"
    )   
else()
    # Enable console by removing WIN32 flag
    add_executable(notk WIN32 ${SOURCES})
endif()

install(TARGETS notk
    DESTINATION ${CMAKE_BINARY_DIR})

if (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
    set(LIBS
        ${CMAKE_SOURCE_DIR}/SDL/lib/x64/SDL2main.lib
        ${CMAKE_SOURCE_DIR}/SDL/lib/x64/SDL2.lib
        ${CMAKE_SOURCE_DIR}/SDL_image/lib/x64/SDL2_image.lib
        ${CMAKE_SOURCE_DIR}/SDL_ttf/lib/x64/SDL2_ttf.lib
        ${CMAKE_SOURCE_DIR}/SDL_mixer/lib/x64/SDL2_mixer.lib
    )

    target_link_libraries(notk ${LIBS})
endif()

set(DLLS
    ${CMAKE_SOURCE_DIR}/SDL/lib/x64/SDL2.dll
    ${CMAKE_SOURCE_DIR}/SDL_image/lib/x64/SDL2_image.dll
    ${CMAKE_SOURCE_DIR}/SDL_ttf/lib/x64/SDL2_ttf.dll
    ${CMAKE_SOURCE_DIR}/SDL_mixer/lib/x64/SDL2_mixer.dll
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

install(FILES ${EXTERNAL_DLLS}
    DESTINATION ${CMAKE_BINARY_DIR}
)

set(CONFIG 
    ${CMAKE_SOURCE_DIR}/zword/config.cfg
)

install(FILES ${CONFIG}
    DESTINATION ${CMAKE_BINARY_DIR}
)
