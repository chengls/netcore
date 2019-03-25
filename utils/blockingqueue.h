#ifndef __BLOCKING_QUEUE_H
#define __BLOCKING_QUEUE_H
#include <queue>
#include <condition_variable>
#include <mutex>
#include "noncopyable.h"

namespace netcore
{
	template<typename T>
	class BlockQueue:NonCopyable
	{
	public:
		T & get();
		void put(T &);
		bool tryGet(T &);

	private:
		std::mutex mutex_;
		std::condition_variable cond_;
		std::queue<T> queue_;
	};
}

#endif