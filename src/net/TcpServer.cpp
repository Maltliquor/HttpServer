#include "src/net/TcpServer.h"

#include "src/net/EventLoop.h"
#include "src/net/EventLoopThreadPool.h"
#include "src/net/SocketOp.h"

#include <stdio.h>  // snprintf

using namespace serverlib;

TcpServer::TcpServer(EventLoop* loop,
                     const InetAddress& listenAddr,
					 const std::string& name,
                     Option option)
  : _loop(loop),  //CHECK_NOTNULL(loop),           //绑定EventLoop
    _ip_port(listenAddr.toIpPort()),       //获取IP+port的string
	_name(name), //用于命名每个创建的tcpconnection的name，name会作为键值key存放在connection_map中，方便快速调出tcpconnection
    _accept_socket(sockets::createNonblockingOrDie(listenAddr.family())), //创建非阻塞的socket
    _accept_channel(loop, _accept_socket.fd()),   //将创建得到的socket文件描述符封装到channel中
    _listening(false),
	_reuse_port(false),
	_started(false),
    _idle_fd(::open("/dev/null", O_RDONLY | O_CLOEXEC)), 
    _thread_pool(new EventLoopThreadPool(loop, name)),   	
    _connection_callback(defaultConnectionCallback),   //连接成功时执行的回调函数
    _message_callback(defaultMessageCallback),         //接收消息时执行的回调函数
    _next_conn_Id(1)
{
	//accept一个新的socket
	assert(_idle_fd >= 0);
	assert(_accept_socket.fd() >= 0);
    _accept_socket.setReuseAddr(true);
    _accept_socket.setReusePort(_reuse_port);
    _accept_socket.bindAddress(listenAddr);
	printf("TcpServer Established: fd:%d, address:%s \n",
			_accept_socket.fd(), _ip_port.c_str());
			
    _accept_channel.setReadCallback(
        std::bind(&TcpServer::handleReadAndAcceptConnection, this));
		
    setNewConnectionCallback(
        std::bind(&TcpServer::newConnection, this, _1, _2));//NewConnectionCallback在handleRead中执行
}

TcpServer::~TcpServer(){
    _loop->assertInLoopThread();

	for (auto& item : _connection_map) {
		TcpConnectionPtr conn(item.second);
		item.second.reset();
		conn->getLoop()->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
	}
}

void TcpServer::handleReadAndAcceptConnection(){   //执行读事件,接受新的客户端连接
    _loop->assertInLoopThread();
    InetAddress clent_addr;
	
    //FIXME loop until no more
    int connfd = _accept_socket.accept(&clent_addr); //调用系统中的::accept(4)
    if (connfd >= 0) {
        if (_new_connection_callback) {
			_new_connection_callback(connfd, clent_addr);//此处的回调函数相当于执行TcpServer::newConnection
        }
		else{
			sockets::close(connfd);
		}
	}
	else {
		if (errno == EMFILE) {//EMFILE表示用户的fd已用尽
			::close(_idle_fd);
			_idle_fd = ::accept(_accept_socket.fd(), NULL, NULL);
			::close(_idle_fd);
			_idle_fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
		}
	}
}

void TcpServer::newConnection(int sockfd, const InetAddress& client_addr){
	_loop->assertInLoopThread();
	EventLoop* io_loop = _thread_pool->getNextLoop();  //从threadpool中取出一个loop, 采用round-robin调度
	char buf[64];
	snprintf(buf, sizeof buf, "-%s#%d", _ip_port.c_str(), _next_conn_Id);
	++_next_conn_Id;
	std::string conn_name = _name + buf;

    printf( "TcpServer::newConnection [%s] - new connection [%s] from %s\n" ,	
			_name.c_str(), conn_name.c_str(), client_addr.toIpPort().c_str());
        
    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TcpConnectionPtr conn(new TcpConnection(io_loop,
                                          conn_name,
                                          sockfd,
                                          localAddr,
                                          client_addr));
										  
    _connection_map[conn_name] = conn;
	 //同一个server建立的每一个connection的callback都是相同的，是由server统一提供的
    conn->setConnectionCallback(_connection_callback); 
    conn->setMessageCallback(_message_callback);
    conn->setWriteCompleteCallback(_write_complete_callback);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, _1)); // FIXME: unsafe
    io_loop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::setThreadNum(int num_threads){
  assert(0 <= num_threads);
  _thread_pool->setThreadNum(num_threads);
}

void TcpServer::start(){
	if (!_started) {
		_thread_pool->start(_thread_init_callback);  //启动线程池，

		assert(!_listening);
		_loop->runInLoop(std::bind(&TcpServer::listen, this));
			
		_started = true;
	}
}

void TcpServer::listen(){
  _loop->assertInLoopThread();
  _listening = true;
  _accept_socket.listen();  //监听socket
  _accept_channel.bindReadingEvent();   //将socket注册到epoll函数中
}


void TcpServer::removeConnection(const TcpConnectionPtr& conn){
  // FIXME: unsafe
  _loop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn)
{
  _loop->assertInLoopThread();
  printf("TcpServer::removeConnectionInLoop [%s] - connection %s\n" ,
			_name.c_str(), conn->name().c_str());
  size_t n = _connection_map.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->getLoop();
  ioLoop->queueInLoop(
      std::bind(&TcpConnection::connectDestroyed, conn));
}