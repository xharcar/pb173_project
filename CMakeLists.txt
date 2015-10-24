cmake_minimum_required(VERSION 2.8)
project(World-of-Tanks)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb3 -std=c11 -Wall -Wextra -pedantic -Werror")

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
