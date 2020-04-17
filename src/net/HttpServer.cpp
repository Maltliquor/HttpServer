#include "src/net/HttpServer.h"
#include "src/net/HttpRequest.h"
#include "src/net/HttpResponse.h"

using namespace serverlib;

namespace serverlib
{
namespace detail
{

	void defaultHttpCallback(const HttpRequest&, HttpResponse* resp){
		resp->setStatusCode(HttpResponse::NotFound404);
		resp->setContentType("text/html");
		resp->addHeader("Server", "Serverlib");
		resp->setCloseConnection(true);
		resp->setBody("<html><head><title>Web Blog</title></head>"
			"<body><h1>404 Not Found</h1>"
			"</body></html>");
	}

}  // namespace detail
}  // namespace serverlib

HttpServer::HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
					   const string& name,
                       TcpServer::Option option)
  : _server(loop, listenAddr, name, option),
    _http_callback(detail::defaultHttpCallback)
{
    _server.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, _1));       //接受新的Http连接时执行的函数
    _server.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, _1, _2, _3));  //接受到消息时执行的函数，主要进行url分析
}

void HttpServer::start(){
    _server.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn){
	if (conn->connected()) {
		conn->setContext(HttpRequest());
	}
}

void HttpServer::onMessage(const TcpConnectionPtr& conn,
                           std::vector<char> &buf, int len){
	
	printf("In HttpServer::onMessage()\n");
	HttpRequest* req = conn->getMutableContext();
	
	if (!req->parseRequest(buf, len)) { //如果请求分析的过程中出现错误
		conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
		conn->shutdown();
	}

	if (req->isGotAll()) { //如果请求分析完毕
		printf("In req->isGotAll()\n");
		onRequest(conn, *req);
		req->reset();
	}
}

void HttpServer::onRequest(const TcpConnectionPtr& conn, const HttpRequest& req){
	printf("In HttpServer::onRequest()\n");
	const string& connection = req.getHeader("Connection");
	bool client_close = (connection == "close" ||
		(req.getVersion() == HttpRequest::vHttp10 && connection != "Keep-Alive"));
	
	 HttpResponse response(client_close);
	
	//调用用户自定义的callback函数
	_http_callback(req, &response);
	
	std::string msg;
	response.getResponseMsg(msg);  //把response里面的信息组装成string，存放到msg内
	conn->send(msg);
	
	if (client_close){
		conn->shutdown();
	}
}
