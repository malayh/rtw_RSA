cmake_minimum_required(VERSION 3.1...3.16)
project(rtwRSA VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "-g")

add_library(util src/util.cpp)
add_library(rsa src/rsa.cpp)

add_executable(main src/main.cpp)
target_link_libraries( main util rsa gmpxx gmp)

add_executable(tests src/test.cpp)
target_link_libraries( tests util rsa gmpxx gmp)



