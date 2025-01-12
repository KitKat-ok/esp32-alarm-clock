# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

message(STATUS "get_pio_env output: ${PIO_ENV_OUTPUT}") # SEND_ERROR, WARNING
file(GLOB_RECURSE APP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.c
)

message(STATUS "App sources: ${APP_SOURCES}")

idf_component_register(SRCS ${APP_SOURCES})