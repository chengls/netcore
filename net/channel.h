#ifndef __CHANNEL_H
#define __CHANNEL_H
#include "../utils/noncopyable.h"
#include "eventloop.h"

namespace netcore
{
	class EventLoop;

	class Channel :NonCopyable
	{
	public:
		enum
		{
			EVT_NONE = 0,
			EVT_R = 1,
			EVT_W = 2,
			EVT_RW = EVT_R | EVT_W,
		};

		typedef std::function<void()> ReadableCallback;
		typedef std::function<void()> WritableCallback;

		Channel(int fd, EventLoop * loop) :fd_(fd), loop_(loop), evt_(EVT_NONE) {}

		int fd() { return fd_; }

		void remove() { assert(isNonEvent()); loop_->removeChannel(this); }
		void reallocate(int fd)
		{
			if (fd_ == fd)
				return;
			remove();
			fd_ = fd;
		}

		void enableReading() { evt_ |= EVT_R; update(); }
		void enableWriting() { evt_ |= EVT_W; update(); }
		void enableAll() { evt_ |= EVT_RW; update(); }

		void disableReading() { evt_ &= (~EVT_R); update(); }
		void disableWriting() { evt_ &= (~EVT_W); update(); }
		void disableAll() { evt_ = EVT_NONE; update(); }

		bool isReading() const { return evt_ & EVT_R; }
		bool isWriting() const { return evt_ & EVT_W; }
		bool isNonEvent() const { return !(evt_ & EVT_RW); }

		void handleReadable() { if (readableCallback_) readableCallback_(); }
		void handleWritable() { if (writableCallback_) writableCallback_(); }

		void setReadableCallback(const ReadableCallback & cb) { readableCallback_ = cb; }
		void setWritableCallback(const WritableCallback & cb) { writableCallback_ = cb; }

	private:
		void update() { loop_->updateChannel(this); }

	private:
		int fd_;
		EventLoop * loop_;
        int evt_;
		ReadableCallback readableCallback_;
		WritableCallback writableCallback_; 
	};
}

#endif