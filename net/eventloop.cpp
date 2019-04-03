#include "eventloop.h"
#include "channel.h"
#include "select.h"
#include "epoll.h"
#include "kqueue.h"
#include <stdlib.h>
#include "../utils/logger.h"
#include "../utils/threadpool.h"
using namespace netcore;

#ifndef _WIN32
namespace 
{
	class IgnoreSigPipe
	{
	public:
		IgnoreSigPipe()
		{
			::signal(SIGPIPE, SIG_IGN);
		}
	};
	IgnoreSigPipe initObj;
}
#endif

EventLoop::EventLoop()
	:poller_(defaultPoller()),
	tid_(std::this_thread::get_id()),
	running_(false),
	wakeupSock_(createWakeupFd()),
	//wakeupSock_(Socket::createSocket())
	wakeupChannel_(new Channel(wakeupFd_[1], this))
{
	wakeupChannel_->enableReading();
	//createWakeupFd();
}

EventLoop::~EventLoop()
{
	wakeupSock_.close();
}

void EventLoop::loop()
{
	running_ = true;
	
	while (running_)
	{
		poller_->poll();
		std::vector<Func> funcs = pendingFuncs_.getAll();
		for (const Func & func : funcs)
		{
			func();
		}
	}
}

void EventLoop::quit()
{
	if (inOwnThread())
	{
		quitInLoop();
	}
	else
	{
		runInLoop(std::bind(&EventLoop::quitInLoop, this));
	}
}

void EventLoop::quitInLoop()
{
	running_ = false;
}

void EventLoop::updateChannel(Channel *channel)
{
	assertInOwnThread();
	poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
	assertInOwnThread();
	poller_->removeChannel(channel);
}

void EventLoop::runInLoop(const Func &cb)
{
	if (inOwnThread())
	{
		cb();
	}
	else
	{
		pendingFuncs_.put(cb);
		wakeup();
	}
}

void EventLoop::wakeup()
{
	return;
	if (wakeupSock_.fd() == 0)
		return;

	char one = ' ';
	ssize_t n = wakeupSock_.write(&one, sizeof one);
	if (n != sizeof one)
	{
		LOG_ERROR << "EventLoop::wakeup error, errno = " << ERRNO << " " << wakeupSock_.fd();
	}
}

void EventLoop::createWakeupFdCallback(int wakeupFd)
{
	assertInOwnThread();
	wakeupChannel_ = std::unique_ptr<Channel>(new Channel(wakeupFd, this));
	//wakeupChannel_->enableReading();
}

int EventLoop::createWakeupFd()
{
	NetAddr serveraddr("127.0.0.1", 0);
	int len = static_cast<int>(sizeof(serveraddr.getAddr()));
	Socket sock(Socket::createSocket());

	sock.bind(serveraddr);
	printf("socket port %s\n", serveraddr.toIpPort().c_str());
	sock.listen(1);


	Socket sock_0(Socket::createSocket());
	wakeupFd_[0] = sock_0.fd();
	sock_0.connect(serveraddr);

	NetAddr peeraddr;
	wakeupFd_[1] = sock.accept(&peeraddr);

	sock.close();

	return wakeupFd_[0];
}

//void EventLoop::createWakeupFd()
//{
//	ThreadPool pool(2);
//	pool.start();
//	short port = 20000; // TODO
//	printf("port = %d\n", port);
//	pool.run([this, port]() 
//	{
//		NetAddr serveraddr("127.0.0.1", port);
//		int len = static_cast<int>(sizeof(serveraddr.getAddr()));
//		Socket sock(Socket::createSocket());
//		sock.setReuseAddr(true);
//		sock.setReusePort(true);
//		sock.bind(serveraddr);
//		sock.listen(5);
//
//		NetAddr peeraddr;
//		int connfd = sock.accept(&peeraddr);
//		if (connfd < 0)
//		{
//			LOG_ERROR << "createWakeupFd Failed, errno = " << ERRNO;
//		}
//		else
//		{
//			LOG_ERROR << "createWakeupFd Succeed, connfd = " << connfd;
//			sock.close();
//			this->runInLoop(std::bind(&EventLoop::createWakeupFdCallback, this, connfd));
//		}
//	});
//
//	pool.run([this, port]()
//	{
//		printf("run thread to connect\n");
//		NetAddr serveraddr("127.0.0.1", port);
//		int ret = this->wakeupSock_.connect(serveraddr);
//		if (ret < 0)
//		{
//			LOG_ERROR << "connect error " << ERRNO;
//		}
//	});
//}

Poller * EventLoop::defaultPoller()
{
#ifdef USE_EPOLL
	return new EpollPoller(this);
#elif USE_KQUEUE
	return new KqueuePoller(this);
#else
	return new SelectPoller(this);
#endif

}
