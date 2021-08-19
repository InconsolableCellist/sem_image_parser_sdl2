cmake_minimum_required(VERSION 3.20)
project(sem_image_parser_sdl2 C)

set(CMAKE_C_STANDARD 99)

add_compile_options(-Ofast)
find_package(SDL2 REQUIRED)
find_file(SDL2_INCLUDE_DIR NAME SDL.h HINTS SDL2)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(sem_image_parser_sdl2 main.c)
target_link_libraries(sem_image_parser_sdl2 ${SDL2_LIBRARIES})
