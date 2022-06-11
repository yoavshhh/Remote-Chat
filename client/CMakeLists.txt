find_package(asio CONFIG REQUIRED)

add_executable(Client WIN32 "client.cpp")
target_compile_features(Client PRIVATE cxx_std_17)

target_link_libraries(Client PRIVATE net)
target_link_libraries(Client PRIVATE asio asio::asio)