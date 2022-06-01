cmake_minimum_required(VERSION 3.0.0)
project(RemoteChat VERSION 0.1.0)

# include(CTest)
# enable_testing()

add_subdirectory(net)

find_package(asio CONFIG REQUIRED)

add_executable(RemoteChat "main.cpp")

target_link_libraries(RemoteChat PRIVATE net)
target_link_libraries(RemoteChat PRIVATE asio asio::asio)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
