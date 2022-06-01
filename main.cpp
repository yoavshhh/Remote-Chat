#include <iostream>

#include <olc_net.h>

enum class Hello : uint32_t {
    HH, BB
};

int main(int, char**) {
    std::cout << "Hello, world!!!!!!!!!!!\n";
    olc::net::connection<Hello> con;
    int a; std::cin >> a;
    return 0;
}
