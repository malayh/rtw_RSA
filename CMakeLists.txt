cmake_minimum_required(VERSION 3.1...3.16)
project(rtwRSA VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_FLAGS "-g")
set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_library(util src/util.cpp)
add_library(rsa src/rsa.cpp)

add_executable(rtwrsa src/main.cpp)
target_link_libraries( rtwrsa util rsa gmpxx gmp)

add_executable(tests src/test.cpp)
target_link_libraries(tests util rsa gmpxx gmp pthread)



