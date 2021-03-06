#ifndef __SERVER_H
#define __SERVER_H
#include <string>
#include <vector>
#include <memory>
#include "../utils/noncopyable.h"
#include "netaddr.h"
#include "eventloop.h"
#include "eventloopthread.h"
#include "acceptor.h"
#include "callback.h"
namespace netcore
{
	class Server :NonCopyable
	{
	public:
		Server(EventLoop *loop, const NetAddr & addr, const std::string &name);
		~Server();

		void start(int threadnum = 0);
		void stop();

		void setConnectionCallback(const ConnectionCallback & cb) { connectionCallback_ = cb; }
		void setMessageCallback(const MessageCallback & cb) { messageCallback_ = cb; }

		void onConnection(const ConnectionPtr &conn) { connectionCallback_(conn); }
		void onMessage(const ConnectionPtr &conn, Buffer &buffer) { messageCallback_(conn, buffer); }

		void defaultConnectionCallback(const ConnectionPtr &conn);
		void defaultMessageCallback(const ConnectionPtr &conn, Buffer &buffer);

	private:
		EventLoop *loop_;
		NetAddr addr_;
		std::string name_;
		std::unique_ptr<Acceptor> acceptor_;
		bool running_;
		std::vector<std::unique_ptr<EventLoopThread>> threads_;

		ConnectionCallback connectionCallback_;
		MessageCallback messageCallback_;
	};
}

#endif
