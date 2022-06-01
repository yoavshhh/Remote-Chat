#pragma once

#include "net_common.h"

namespace olc
{
	namespace net
	{
		template <typename T>
		class tsqueue {

		public:
			tsqueue() = default;
			tsqueue(const tsqueue<T>&) = delete;
			virtual ~tsqueue() { clear(); }

		public:
			// Returns and maintains item at front of the queue
			const T& front() {

				asio::detail::scoped_lock lock(muxQueue);
				return deqQueue.front();
			}

			const T& back() {

				asio::detail::scoped_lock lock(muxQueue);
				return deqQueue.back();
			}

			void push_front(const T& item) {

				asio::detail::scoped_lock lock(muxQueue);
				deqQueue.push_front(std::move(item));

				std::unique_lock<asio::detail::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			void push_back(const T& item) {

				asio::detail::scoped_lock lock(muxQueue);
				deqQueue.push_back(std::move(item));

				std::unique_lock<asio::detail::mutex> ul(muxBlocking);
				cvBlocking.notify_one();
			}

			bool empty() {

				asio::detail::scoped_lock lock(muxQueue);
				return deqQueue.empty();
			}

			size_t count() {

				asio::detail::scoped_lock lock(muxQueue);
				return deqQueue.size();
			}

			void clear() {

				asio::detail::scoped_lock lock(muxQueue);
				deqQueue.clear();
			}

			T pop_front() {

				asio::detail::scoped_lock lock(muxQueue);
				auto t = std::move(deqQueue.front());
				deqQueue.pop_front();
				return t;
			}

			T pop_back() {

				asio::detail::scoped_lock lock(muxQueue);
				auto t = std::move(deqQueue.back());
				deqQueue.pop_back();
				return t;
			}

			void wait() {

				while (empty()) {

					std::unique_lock<asio::detail::mutex> ul(muxBlocking);
					cvBlocking.wait(ul);
				}
			}

		protected:
			asio::detail::mutex muxQueue;
			std::deque<T> deqQueue;
			asio::detail::condition_variable cvBlocking;
			std::mutex muxBlocking;
		};
	}
}