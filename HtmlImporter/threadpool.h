#pragma once
#pragma once

#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <condition_variable>

#include "threadqueue.h"

class ThreadPool {
private:

	bool m_shutdown;
	SafeQueue<std::function<void()>> m_queue;
	std::vector<std::thread> m_threads;
	std::mutex conditional_mutex;
	std::condition_variable conditional_lock;

	class ThreadWorker {
	private:
		int m_id;
		ThreadPool* m_pool;
	public:
		ThreadWorker(ThreadPool* pool, const int id)
			: m_pool(pool), m_id(id) {}

		void operator()() {
			std::function<void()> func;
			bool dequeued;
			while (!m_pool->m_shutdown) {
				{
					std::unique_lock<std::mutex> lock(m_pool->conditional_mutex);
					if (m_pool->m_queue.empty()) {
						m_pool->conditional_lock.wait(lock);
					}
					dequeued = m_pool->m_queue.try_pop(func);
				}
				if (dequeued) {
					func();
				}
			}
		}
	};
	
public:
	ThreadPool(const int n_threads)
		: m_threads(std::vector<std::thread>(n_threads)), m_shutdown(false) {}

	ThreadPool(const ThreadPool&) = delete;
	ThreadPool(ThreadPool&&) = delete;

	ThreadPool& operator=(const ThreadPool&) = delete;
	ThreadPool& operator=(ThreadPool&&) = delete;

	void init() {
		for (unsigned int i = 0; i < m_threads.size(); ++i) {
			m_threads[i] = std::thread(ThreadWorker(this, i));
		}
	}

	// Waits until threads finish their current task and shutdowns the pool
	void shutdown() {
		m_shutdown = true;
		conditional_lock.notify_all();

		for (unsigned int i = 0; i < m_threads.size(); ++i) {
			if (m_threads[i].joinable()) {
				m_threads[i].join();
			}
		}
	}

	// Submit a function to be executed asynchronously by the pool
	template<typename F, typename...Args>
	auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {

		// Create function and Encapsulate it into a shared ptr
		std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

		// Wrap packaged task into void function
		std::function<void()> wrapper_func = [task_ptr]() {
			(*task_ptr)();
		};

		m_queue.push(wrapper_func);
		conditional_lock.notify_one();

		return task_ptr->get_future();
	}
};