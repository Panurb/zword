cmake_minimum_required(VERSION 3.18)

project(zword)

include_directories(CSFML/include)

include_directories(zword/include)

set(SOURCES
    zword/src/camera.c
    zword/src/collider.c
    zword/src/image.c
    zword/src/level.c
    zword/src/physics.c
    zword/src/player.c
    zword/src/util.c
    zword/src/zword.c
)

add_executable(zword_exe ${SOURCES})

target_link_libraries(zword_exe ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-audio.a)
target_link_libraries(zword_exe ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-graphics.a)
target_link_libraries(zword_exe ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-network.a)
target_link_libraries(zword_exe ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-system.a)
target_link_libraries(zword_exe ${CMAKE_SOURCE_DIR}/CSFML/lib/gcc/libcsfml-window.a)

install(TARGETS zword_exe 
    DESTINATION ${CMAKE_SOURCE_DIR}/bin)

install(DIRECTORY CSFML/bin
    DESTINATION ${CMAKE_SOURCE_DIR})

install(DIRECTORY zword/data
    DESTINATION ${CMAKE_SOURCE_DIR}/bin)
