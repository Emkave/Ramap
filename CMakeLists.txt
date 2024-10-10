cmake_minimum_required(VERSION 3.29)
project(Ramap)

set(CMAKE_CXX_STANDARD 23)

set(WDK_INCLUDE_DIR "C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.26100.0\\km")
set(WDK_LIB_DIR "C:\\Program Files (x86)\\Windows Kits\\10\\Lib\\10.0.26100.0\\km\\x64")

include_directories(${WDK_INCLUDE_DIR})
link_directories(${WDK_LIB_DIR})

add_executable(Ramap main.cpp)

target_link_libraries(Ramap ntoskrnl hal)