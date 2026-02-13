cmake_minimum_required(VERSION 3.13)
project(shell)
file(GLOB SRC_FILES "src/*.c" "src/*.h")
add_executable(shell ${SRC_FILES})
