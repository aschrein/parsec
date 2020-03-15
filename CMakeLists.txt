project(simple_parser)
# add_subdirectory(3rdparty/asmjit)
cmake_minimum_required(VERSION 3.16)
add_executable(simple_parser main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -fno-exceptions -fno-rtti")
include_directories(simple_parser
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/build
  )
# target_include_directories(simple_parser PRIVATE 3rdparty)
# target_link_libraries(simple_parser PRIVATE asmjit)