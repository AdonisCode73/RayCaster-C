cmake_minimum_required(VERSION 3.10)
project(RayCaster-C C)

file(GLOB SRC src/*.c)
add_executable(RayCaster_C ${SRC})

target_include_directories(RayCaster_C PRIVATE
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/libs/SDL2/include/
)

target_link_directories(RayCaster_C PRIVATE
    ${CMAKE_SOURCE_DIR}/libs/SDL2/lib/x64
)

target_link_libraries(RayCaster_C SDL2 SDL2main)

add_custom_command(TARGET RayCaster_C POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/libs/SDL2/lib/x64/SDL2.dll"
        $<TARGET_FILE_DIR:RayCaster_C>)