cmake_minimum_required(VERSION 3.8.0)
project(xjson VERSION 0.3.0 LANGUAGES C CXX)

include_directories(lib)

include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)

# effective cpp -W
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Weffc++")

# 循环添加 tests 文件夹下的所有文件, 编译
file(GLOB TEST_SOURCES tests/*.cpp)
foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_NAME} ${TEST_SOURCE})
  add_test(${TEST_NAME} ${TEST_NAME})
endforeach()

# addtest

# install to system
install(FILES lib/json.hpp DESTINATION include/xihale)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
