#pragma once

#include <queue>
#include <mutex>

template<class T>
class SafeQueue {

private:
	std::queue<T> m_queue;
	std::mutex m_mutex;

public:
	bool empty();
	int size();
	void push(T elem);
	bool try_pop(T& elem);
};

template <class T>
bool SafeQueue<T>::empty() 
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_queue.empty();
}

template <class T>
int SafeQueue<T>::size() 
{
	std::unique_lock<std::mutex> lock(m_mutex);
	return m_queue.size();
}

template <class T>
void SafeQueue<T>::push(T elem)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_queue.push(std::move(elem));
}

template <class T>
bool SafeQueue<T>::try_pop(T& elem)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (m_queue.empty()) {
		return false;
	}

	elem = std::move(m_queue.front());
	m_queue.pop();

	return true;
}