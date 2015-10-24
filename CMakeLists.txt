project(World-of-Tanks)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99 -pedantic -Wall -Wextra -ggdb3")

# aux_source_directory(. SRC_LIST)
# add_executable(${PROJECT_NAME} ${SRC_LIST})

add_executable(world
    src/logger.h
    src/world.h
    src/world.c
)
add_executable(tank
    src/logger.h
    src/tank.h
    src/tank.c
)
add_executable(tankclient
    src/logger.h
    src/tankclient.h
    src/tankclient.c
)
