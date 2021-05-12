#pragma once

#include "net_common.h"
#include <deque>

namespace olc {
	namespace net {
		/// <summary>
		/// Thread Safe Double Queue
		/// </summary>
		/// <typeparam name="T"></typeparam>
		template <typename T>
		class ts_deque {
		public:
			ts_deque() = default;
			ts_deque(const ts_deque<T>&) = delete;
			virtual ~ts_deque() { clear(); }
		public:
			// Return the front item from deque and keep it in the deque
			const T& front() {
				std::scoped_lock lock(_mtx_deque);
				return _deque.front();
			}
			// Return the back item from deque and keep it in the deque
			const T& back() {
				std::scoped_lock lock(_mtx_deque);
				return _deque.back();
			}

			// Return the back item from deque and remove it in the deque
			T pop_back() {
				std::scoped_lock lock(_mtx_deque);
				auto temp = std::move(_deque.back());
				_deque.pop_back();
				return temp;
			}
			// Return the front item from deque and remove it in the deque
			T pop_front() {
				std::scoped_lock lock(_mtx_deque);
				auto temp = std::move(_deque.front());
				_deque.pop_front();
				return temp;
			}
			void push_back(const T& item) {
				std::scoped_lock lock(_mtx_deque);
				_deque.emplace_back(std::move(item));

				std::unique_lock<std::mutex> ul_blocking (_mtx_blocking) ;
				_cv_blocking.notify_one();

			}

			void push_front(const T& item) {
				std::scoped_lock lock(_mtx_deque);
				_deque.emplace_front(std::move(item));
				std::unique_lock<std::mutex> ul_blocking(_mtx_blocking);
				_cv_blocking.notify_one();
			}

			bool empty() {
				std::scoped_lock lock(_mtx_deque);
				return _deque.empty();
			}
			void wait() {
				while (empty()) {
					std::unique_lock<std::mutex> ul_blocking(_mtx_blocking);
					_cv_blocking.wait(ul_blocking);
				}
			}
			size_t count() {
				std::scoped_lock lock(_mtx_deque);
				return _deque.size();
			}
			void clear() {
				std::scoped_lock lock(_mtx_deque);
				return _deque.clear();
			}
		protected:
			std::mutex _mtx_deque;
			std::deque<T> _deque;

			std::condition_variable _cv_blocking;
			std::mutex _mtx_blocking;
		};
	}
}
