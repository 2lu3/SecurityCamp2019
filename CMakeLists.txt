cmake_minimum_required(VERSION 3.0.0)
project(SecurityCamp2019 VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(SecurityCamp2019 main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
