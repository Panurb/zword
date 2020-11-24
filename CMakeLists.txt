cmake_minimum_required(VERSION 3.18)

project(zword)

include_directories(CSFML/include)

include_directories(zword/include)

set(SOURCES
    zword/src/camera.c
    zword/src/collider.c
    zword/src/component.c
    zword/src/enemy.c
    zword/src/grid.c
    zword/src/image.c
    zword/src/level.c
    zword/src/light.c
    zword/src/particle.c
    zword/src/physics.c
    zword/src/player.c
    zword/src/util.c
    zword/src/zword.c
)

add_executable(zword ${SOURCES})

target_link_libraries(zword ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-audio.a)
target_link_libraries(zword ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-graphics.a)
target_link_libraries(zword ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-network.a)
target_link_libraries(zword ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-system.a)
target_link_libraries(zword ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-window.a)

install(TARGETS zword
    DESTINATION ${CMAKE_SOURCE_DIR}/build)
	
set(DLLS
	CSFML/bin/csfml-audio-2.dll
	CSFML/bin/csfml-graphics-2.dll
	CSFML/bin/csfml-network-2.dll
	CSFML/bin/csfml-system-2.dll
	CSFML/bin/csfml-window-2.dll
)

install(FILES ${DLLS}
    DESTINATION ${CMAKE_SOURCE_DIR}/build)

install(DIRECTORY zword/data
    DESTINATION ${CMAKE_SOURCE_DIR}/build)
