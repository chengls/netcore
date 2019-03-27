#ifndef __EVENTLOOP_H
#define __EVENTLOOP_H
#include "../utils/noncopyable.h"
#include "../utils/blockingqueue.h"
#include "poller.h"
#include <thread>
#include <cassert>
#include <vector>
#include <functional>
namespace netcore
{
    class Channel;
    class EventLoop :NonCopyable
	{
	public:
		typedef std::function<void()> Func;

		EventLoop();

		~EventLoop();
		
		void loop();

		void quit();

		void updateChannel(Channel *);

		void runInLoop(Func &);

		bool inOwnThread() { return tid_ == std::this_thread::get_id(); } // TODO thread_����

		void assertInOwnThread() { assert(inOwnThread()); }

	private:
		Poller * defaultPoller();

	private:
		Poller * poller_;
		std::thread::id tid_;
        bool running_;
		BlockingQueue<Func> pendingFuncs_;
		
	};
}

#endif
