cmake_minimum_required(VERSION 3.0.0)
project(RemoteChat VERSION 0.1.0)

# include(CTest)
# enable_testing()

add_subdirectory(net)
add_subdirectory(client)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
