#pragma once

#include <map>
#include <atomic>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "src/net/TcpConnection.h"
#include "src/net/Channel.h"
#include "src/net/Socket.h"

namespace serverlib
{

class EventLoop;
class EventLoopThreadPool;

///
/// TCP server, supports single-threaded and thread-pool models.
///
/// This is an interface class, so don't expose too much details.
class TcpServer
{
 public:
	typedef std::function<void(EventLoop*)> ThreadInitCallback;
	enum Option  {
		kNoReusePort,
		kReusePort,
	};

  //TcpServer(EventLoop* loop, const InetAddress& listenAddr);
	TcpServer(EventLoop* loop,
            const InetAddress& listenAddr,
			const std::string& nameArg,
            Option option = kNoReusePort);
	~TcpServer();  // force out-line dtor, for std::unique_ptr members.

	const std::string& ipPort() const { return _ip_port; }
	const std::string& name() const { return _name; }
	EventLoop* getLoop() const { return _loop; }
	void handleReadAndAcceptConnection();
	/// Set the number of threads for handling input.
	///
	/// Always accepts new connection in loop's thread.
	/// Must be called before @c start
	/// @param numThreads
	/// - 0 means all I/O in loop's thread, no thread will created.
	///   this is the default value.
	/// - 1 means all I/O in another thread.
	/// - N means a thread pool with N threads, new connections
	///   are assigned on a round-robin basis.
	void setThreadNum(int numThreads);
	void setThreadInitCallback(const ThreadInitCallback& cb)
	{ _thread_init_callback = cb; }
	/// valid after calling start()
	std::shared_ptr<EventLoopThreadPool> threadPool()
	{ return _thread_pool; }

	/// Starts the server if it's not listenning.
	///
	/// It's harmless to call it multiple times.
	/// Thread safe.
	void start();

	/// Set connection callback.
	/// Not thread safe.
	void setConnectionCallback(const EventCallback& cb)
	{ _connection_callback = cb; }

	/// Set message callback.
	/// Not thread safe.
	void setMessageCallback(const MessageCallback& cb)
	{ _message_callback = cb; }

	/// Set write complete callback.
	/// Not thread safe.
	void setWriteCompleteCallback(const EventCallback& cb)
	{ _write_complete_callback = cb; }

	typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;
	void setNewConnectionCallback(const NewConnectionCallback& cb)
	{ _new_connection_callback = cb; }
  
  
    void listen();
  
 private:  
	/// Not thread safe, but in loop
	void newConnection(int sockfd, const InetAddress& peerAddr);
	/// Thread safe.
	void removeConnection(const TcpConnectionPtr& conn);
	/// Not thread safe, but in loop
	void removeConnectionInLoop(const TcpConnectionPtr& conn);

	
	EventLoop* _loop; 
    const std::string _ip_port;
	const std::string _name;
	Socket _accept_socket;     //server所属的socket
    Channel _accept_channel;   //持有server socket的fd的channel，负责监听client的连接
    NewConnectionCallback _new_connection_callback;  //在handle_read中处理新建连接的回调函数
    bool _listening;
	bool _reuse_port;
	std::atomic<bool> _started;
	int _idle_fd;	
	
    std::shared_ptr<EventLoopThreadPool> _thread_pool;
    EventCallback _connection_callback;
    MessageCallback _message_callback;
    EventCallback _write_complete_callback;
    ThreadInitCallback _thread_init_callback;
    
    // always in loop thread
    int _next_conn_Id;
    std::map<std::string, TcpConnectionPtr> _connection_map;
	
};

}//namespace serverlib