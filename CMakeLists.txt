cmake_minimum_required(VERSION 2.6)
project(recipes)


set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 
set(Boost_ALL_DYN_LINK ON)


find_package(Threads)

find_package(Boost REQUIRED filesystem system)

include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(asio/roundtrip)
add_subdirectory(asio/filetransfer)
