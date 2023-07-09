#pragma once

#include "Core.h"

#include <queue>
#include <mutex>
#include <condition_variable>

namespace Core
{
	template<typename T>
	class ThreadSafeQueue
	{
	public:

		ThreadSafeQueue()
			: m_Queue(), m_Mutex(), m_Conditional()
		{
		}

		~ThreadSafeQueue()
		{
		}

		void Enqueue(const T &value)
		{
			std::lock_guard<std::mutex> lock(m_Mutex);
			m_Queue.push(value);
			m_Size++;
			m_Conditional.notify_one();
		}

		void Dequeue()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			while (m_Queue.empty())
			{
				m_Conditional.wait(lock);
			}

			T &value = m_Queue.front();
			value.~T();

			m_Queue.pop();
			m_Size--;
		}

		T &Front()
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			while (m_Queue.empty())
			{
				m_Conditional.wait(lock);
			}

			return m_Queue.front();
		}

		T Get(uint32 index)
		{
			std::unique_lock<std::mutex> lock(m_Mutex);
			while (m_Queue.empty())
			{
				m_Conditional.wait(lock);
			}
			
			if (index >= m_Queue.size())
			{
				return {};
			}

			T result = {};
			std::queue<T> temp;
			uint32 current_index = 0;
			while (!m_Queue.empty())
			{
				if (current_index == index)
				{
					result = m_Queue.front();
				}

				++current_index;
				temp.push(m_Queue.front());
				m_Queue.pop();
			}

			while (!temp.empty())
			{
				m_Queue.push(temp.front());
				temp.pop();
			}

			return result;
		}

		uint32 Size() const
		{
			return m_Size;
		}

	private:

		std::queue<T> m_Queue;
		mutable std::mutex m_Mutex;
		std::condition_variable m_Conditional;

		uint32 m_Size = 0;
	};
}

