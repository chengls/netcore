#include "connector.h"

using namespace netcore;

Connector::Connector(EventLoop *loop)
	:loop_(loop),
	connectorChannel_(Socket::createSocket(), loop),
	state_(Disconnected)
{
	
}
Connector::~Connector()
{
	disconnect();
}

void Connector::connect(const NetAddr & serveraddr)
{
	assert(state_ == Disconnected);
	loop_->assertInOwnThread();

	connectorChannel_.setWritableCallback(std::bind(&Connector::handleWritable, shared_from_this()));
	sock_ = Socket::createNonBlockingSocket();

	NetAddr localaddr(sock_.getLocalAddr());
	std::cout << "connector sock " << localaddr.toIpPort() << std::endl;

	int err = 0;
	int ret = 0;

	do {
		std::cout << "start connect to server " << serveraddr.toIpPort() << std::endl;
		ret = sock_.connect(serveraddr);
		err = ERRNO;
	} while (ret < 0 && (err == EAGAIN || err == EADDRINUSE || err == ECONNREFUSED));

	if (ret < 0)
	{
		if (errnoIsWouldBlock(err))
		{
			state_ = Connecting;
			connectorChannel_.reallocate(sock_.fd());
			connectorChannel_.enableWriting();
		}
		else
		{
			std::cout << "Connector::connect error " << ERRNO << " ret = " << ret << std::endl;
			sock_.close();
			return;
		}
	}
	else
	{
		onConnected();
	}
}

bool Connector::errnoIsWouldBlock(int err)
{
#ifdef _WIN32
	return (err == WSAEINPROGRESS || err == WSAEWOULDBLOCK);
#else
	return (err == EINPROGRESS || EWOULDBLOCK);
#endif
}

void Connector::onConnected()
{
	std::cout << "Connector:: onConnected" << std::endl;
	state_ = Connected;
	connectorChannel_.disableWriting();

	NetAddr peeraddr(sock_.getPeerAddr());
	connection_ = std::make_shared<Connection>(loop_, sock_.fd(), peeraddr);

	connection_->setConnectionCallback(connectionCallback_);
	connection_->setMessageCallback(messageCallback_);
	connection_->setClosedCallback(std::bind(&Connector::handleClosed, shared_from_this()));
	connection_->connectionEstablished();
}

void Connector::handleWritable()
{
	if (state_ == Connecting)
	{
		int err = sock_.getSocketError();
		if (err)
		{
			std::cout << "Connector::handleWritable error" << std::endl;
			sock_.close();
			return;
		}
		else
		{
			onConnected();
		}
	}
	else
	{
		sock_.close();
		return;
	}
}

void Connector::handleClosed()
{
	assert(state_ == Connected);
	disconnect();
}

void Connector::disconnect()
{
	if (state_ == Disconnected)
		return;
	state_ = Disconnected;
	connection_->connectionDestroyed();
	sock_.close();
}