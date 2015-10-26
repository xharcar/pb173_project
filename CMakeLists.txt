project(World-of-Tanks)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -pedantic -Wall -Wextra -ggdb3 -lncurses")

# aux_source_directory(. SRC_LIST)
# add_executable(${PROJECT_NAME} ${SRC_LIST})

add_executable(world
    src/config.h
    src/world.h
    src/world.c
)
add_executable(tank
    src/config.h
    src/tank.h
    src/tank.c
)
add_executable(tankclient
    src/config.h
    src/tankclient.h
    src/tankclient.c
)
