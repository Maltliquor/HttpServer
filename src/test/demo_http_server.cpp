#include "src/net/HttpServer.h"
#include "src/net/HttpRequest.h"
#include "src/net/HttpResponse.h"
#include "src/net/EventLoop.h"

#include <iostream>
#include <map>
#include <string>


using namespace serverlib;

bool benchmark = false;

void onHttpCallback(const HttpRequest& req, HttpResponse* resp){
	printf("In demo::onHttpCallback()...\n");
    printf("Headers: %s %s\n" ,req.methodString(),req.path().c_str());
    if (!benchmark)  {
		const std::map<std::string, std::string>& headers = req.headers();
		for (const auto& header : headers) {
			printf("%s : %s\n" ,header.first.c_str() ,header.second.c_str());
		}
    }
	
	if(req.path() == "/test" ){//Webbench测试
		resp->setStatusCode(HttpResponse::OK200);
		resp->setContentType("text/html");
		resp->addHeader("Server", "Serverlib");
		resp->setBody("<html><head><title>Web Blog</title></head>"
		"<body><h1>This is a test</h1>"
		"</body></html>");
		//resp->setCloseConnection(true);
		return;
	}
	
	std::string file_dir = "../../../src/test/Webpage";
	std::string url_path = req.path();
	
	if (req.path() == "/" || req.path() == "/index.html")  {
		url_path = "html/index.html";
	}
	resp->setStatusCode(HttpResponse::OK200);
	resp->setContentTypeByUrl(url_path);
	resp->addHeader("Server", "Serverlib");
		
	std::string body;		
	if(!resp->readURL(file_dir, url_path, body)){//没有成功读取文件
		resp->setStatusCode(HttpResponse::NotFound404);
		resp->setContentType("text/html");
		resp->addHeader("Server", "Serverlib");
		resp->setBody("<html><head><title>Web Blog</title></head>"
		"<body><h1>404 Not Found</h1>"
		"</body></html>");
		resp->setCloseConnection(true);
		
	}
	else resp->setBody(body);
	
	if (!benchmark)  {
		const std::map<std::string, std::string>& headers = resp->headers();
		for (const auto& header : headers) {
			printf("%s : %s\n" ,header.first.c_str() ,header.second.c_str());
		}
    }
}

int main(int argc, char* argv[])
{
  int num_threads = 2;
  if (argc > 1)
  {
    benchmark = true;
    num_threads = atoi(argv[1]);
  }
  
  
  EventLoop loop;
  //初始化httpserver
  //初始化包括的流程有
  HttpServer server(&loop, InetAddress(80), "dummy");  
  server.setHttpCallback(onHttpCallback);
  server.setThreadNum(num_threads);
  server.start();
  loop.loop();
}