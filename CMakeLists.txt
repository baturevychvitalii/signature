cmake_minimum_required (VERSION 3.0)

project("signature generator" VERSION 1.0 LANGUAGES CXX)



set (CMAKE_CXX_STANDARD 17)

#release
#set (CMAKE_CXX_FLAGS "-O2 -Wall -Wpedantic -pedantic")
#debug
set (CMAKE_CXX_FLAGS "-g -Wall -Wpedantic -pedantic -DDEBUG")

include_directories(src/include)

file(GLOB SOURCES "src/*.cpp")

add_executable(signator ${SOURCES})

find_package(Boost 1.69.0 COMPONENTS program_options REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(signator ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

