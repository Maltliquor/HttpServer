#pragma once
#include <vector>
#include "src/net/TcpServer.h"

namespace serverlib
{
class HttpRequest;
class HttpResponse;

/// A simple embeddable HTTP server designed for report status of a program.
/// It is not a fully HTTP 1.1 compliant server, but provides minimum features
/// that can communicate with HttpClient and Web browser.
/// It is synchronous, just like Java Servlet.
class HttpServer
{
 public:
    typedef std::function<void (const HttpRequest&,
                              HttpResponse*)> HttpCallback;

	HttpServer(EventLoop* loop,
                       const InetAddress& listenAddr,
					   const string& name,
                       TcpServer::Option option = TcpServer::kNoReusePort);
	~HttpServer(){}			   

    EventLoop* getLoop() const { return _server.getLoop(); }

  /// Not thread safe, callback be registered before calling start().
  void setHttpCallback(const HttpCallback& cb)
  {    _http_callback = cb;  }

  void setThreadNum(int num_threads)
  {    _server.setThreadNum(num_threads);  }
  

  void start();

 private:
  void onConnection(const TcpConnectionPtr& conn);
  void onMessage(const TcpConnectionPtr& conn,
                 std::vector<char> &buf,
				 int len);
  void onRequest(const TcpConnectionPtr&, const HttpRequest&);

  TcpServer _server;
  HttpCallback _http_callback;
};

}  // namespace serverlib
