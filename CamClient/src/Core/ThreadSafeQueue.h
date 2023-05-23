#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class ThreadSafeQueue
{
public:

	ThreadSafeQueue()
		: q(), m(), c()
	{
	}

	~ThreadSafeQueue()
	{
	}

	void Enqueue(const T &t)
	{
		std::lock_guard<std::mutex> lock(m);
		q.push(t);
		c.notify_one();
	}

	T Dequeue()
	{
		std::unique_lock<std::mutex> lock(m);
		while (q.empty())
		{
			c.wait(lock);
		}

		T value = q.front();
		q.pop();
		return value;
	}

private:

	std::queue<T> q;
	mutable std::mutex m;
	std::condition_variable c;
};

