#pragma once

#include <stdint.h>
#include <string.h>  // memset
#include <string>
#include <assert.h>
#include <vector>

namespace serverlib
{
using std::string;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;


class TcpConnection;


	inline void memZero(void* p, size_t n){
	  memset(p, 0, n);
	}
	
	typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
	typedef std::function<void()> TimerCallback;
	typedef std::function<void (const TcpConnectionPtr&)> EventCallback;
	typedef std::function<void (const TcpConnectionPtr&,
								std::vector<char> &, int)> MessageCallback;
	
	
	//typedef std::function<void (const TcpConnectionPtr&,
	//							Buffer*,
	//							Timestamp)> MessageCallback;
	
	//typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
	//typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;
	//typedef std::function<void (const TcpConnectionPtr&)> WriteCompleteCallback;

    

	void defaultConnectionCallback(const TcpConnectionPtr& conn);
	void defaultMessageCallback(const TcpConnectionPtr& conn,
							std::vector<char> &, 
							int);
	//void defaultMessageCallback(const TcpConnectionPtr& conn,
	//							Buffer* buffer,
	//							Timestamp receiveTime);
							
}