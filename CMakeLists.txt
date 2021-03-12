cmake_minimum_required(VERSION 3.18)

set(CMAKE_C_FLAGS "-g -Wall -Wextra --pedantic -Wno-deprecated-declarations")

project(zword)

include_directories(CSFML/include)

include_directories(zword/include)

set(SOURCES
    zword/src/camera.c
    zword/src/collider.c
    zword/src/component.c
    zword/src/enemy.c
    zword/src/grid.c
    zword/src/heap.c
    zword/src/image.c
    zword/src/item.c
    zword/src/interface.c
    zword/src/level.c
    zword/src/light.c
    zword/src/navigation.c
    zword/src/particle.c
    zword/src/perlin.c
    zword/src/physics.c
    zword/src/player.c
    zword/src/raycast.c
    zword/src/road.c
    zword/src/sound.c
    zword/src/util.c
    zword/src/zword.c
    zword/src/vehicle.c
    zword/src/weapon.c
)

add_executable(zword ${SOURCES})

install(TARGETS zword
    DESTINATION ${CMAKE_SOURCE_DIR}/build)

set(DLLS
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-audio-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-graphics-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-network-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-system-2.dll
	${CMAKE_SOURCE_DIR}/CSFML/bin/csfml-window-2.dll
)

target_link_libraries(zword ${DLLS})

install(FILES ${DLLS}
    DESTINATION ${CMAKE_SOURCE_DIR}/build)

install(DIRECTORY zword/data
    DESTINATION ${CMAKE_SOURCE_DIR}/build
    FILES_MATCHING 
        PATTERN "*.png"
        PATTERN "*.ttf"
        PATTERN "*.wav"
)
