#include "httpserver.h"
#include <functional>
#include "../../utils/timestamp.h"
using namespace netcore;

HttpServer::HttpServer(EventLoop *loop, const NetAddr &addr, const std::string &name)
	:loop_(loop),
	server_(loop, addr, name),
	httpCallback_(std::bind(&HttpServer::defalutHttpCallback, this, std::placeholders::_1, std::placeholders::_2))
{
	server_.setConnectionCallback(std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
	server_.setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2));
}

HttpServer::~HttpServer()
{
	stop();
}

void HttpServer::onMessage(const ConnectionPtr &conn, Buffer &buffer)
{
	int fd = conn->fd();
	if (ctx_.find(fd) == ctx_.end())
	{
		conn->shutdown();
	}

	HttpContext & http_ctx = ctx_[fd];
	bool ret = http_ctx.parse(&buffer);
	if (!ret)
	{
		conn->send("HTTP/1.1 404 Bad Request\r\n\r\n");
		conn->shutdown();
	}
	else if (http_ctx.parseDone())
	{
		httpparser::HttpResponse response;
		httpCallback_(http_ctx.request(), &response);
		Buffer buf(65536);
		buf.append(response.toStr());
		conn->send(buf);
		if (!response.getKeepAlive())
			conn->shutdown();
		http_ctx.reset();
	}
}

void HttpServer::onConnection(const ConnectionPtr &conn)
{
	int fd = conn->fd();
	if (conn->isConnected())
	{
		assert(ctx_.find(fd) == ctx_.end());
		ctx_[fd] = HttpContext();
	}
	else
	{
		assert(ctx_.find(fd) != ctx_.end());
		ctx_.erase(fd);
	}
}

void HttpServer::defalutHttpCallback(const httpparser::HttpRequest &request, httpparser::HttpResponse * response)
{
	response->setStatus(HTTP_STATUS_OK);
	response->setKeepAlive(true, true);
	response->setBody(std::string("hello"), true);
}