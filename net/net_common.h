#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <deque>
#include <experimental/optional>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstdint>

// #ifdef _WIN32

// #ifdef _WIN32_WINNT
//     #undef _WIN32_WINNT
// #endif

// #define _WIN32_WINNT 0x0A00

// #endif

#define	ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
#include <asio/detail/scoped_lock.hpp>
#include <asio/detail/mutex.hpp>