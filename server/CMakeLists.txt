find_package(asio CONFIG REQUIRED)
find_package(Threads REQUIRED)

add_executable(Server "server.cpp")

target_compile_features(Server PRIVATE cxx_std_17)

target_link_libraries(Server PRIVATE net)
target_link_libraries(Server PRIVATE asio asio::asio)
target_link_libraries(Server PRIVATE Threads::Threads)