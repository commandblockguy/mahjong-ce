cmake_minimum_required(VERSION 3.15)
project(mahjong C)

set(CMAKE_C_STANDARD 90)

include_directories($ENV{CEDEV}/include)

add_custom_target(Make ALL $(MAKE) debug WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# A dummy target to make CLion do stuff
add_executable(dummy
        src/gfx/cursor.c
        src/gfx/gfx_group.c
        src/gfx/gfx_group.h
        src/gfx/tilemap.c
        src/gfx/tiles_gfx.c
        src/gfx/tiles_gfx.h
        src/commands.h
        src/game.h
        src/gfx.c
        src/gfx.h
        src/layouts.h
        src/logic.c
        src/logic.h
        src/main.c
        src/storage.c
        src/storage.h
        src/tile.c
        src/tile.h
        src/ui.c
        src/ui.h
        src/util.c
        src/util.h)
