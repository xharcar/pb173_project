project(Internet-of-Tanks)
cmake_minimum_required(VERSION 2.8)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pthread -lncurses")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wall -Wextra -ggdb3")

# aux_source_directory(. SRC_LIST)
# add_executable(${PROJECT_NAME} ${SRC_LIST})

add_executable(world
    src/world_shared.h
    src/world_shared.cpp
    src/tank.h
    src/tank.cpp
    src/world.h
    src/world.cpp
    src/daemonworld.h
    src/daemonworld.cpp
)

add_executable(worldclient
    src/worldclient.h
    src/worldclient.cpp
)

add_executable(tank
    src/tankclient.h
    src/tankclient.cpp
)

# add_executable(tankclient
#     src/config.h
#     src/tankclient.h
#     src/tankclient.cpp
# )
