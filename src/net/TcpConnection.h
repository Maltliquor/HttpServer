#include <memory>
#include <netinet/tcp.h>
#include <string>
#include <vector>
#include <string.h>
#include <assert.h>

#include "src/base/Defination.h"
#include "src/net/HttpRequest.h"
#include "src/net/InetAddress.h"
// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;


	
namespace serverlib
{

class Channel;
class EventLoop;
class Socket;

//using std::placeholders::_1;


/// TCP connection, for both client and server usage.

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
 public:
	/// Constructs a TcpConnection with a connected sockfd
	///
	/// User should not create this object.
    TcpConnection(EventLoop* loop,
					 const std::string& name,
					 int sockfd,
					 const InetAddress& localAddr,
					 const InetAddress& peerAddr);
    ~TcpConnection();

	EventLoop* getLoop() const { return _loop; }
	const std::string& name() const { return _name; }
	const InetAddress& localAddress() const { return _local_addr; }
	const InetAddress& peerAddress() const { return _peer_addr; }
	bool connected() const { return _state == kConnected; }
	bool disconnected() const { return _state == kDisconnected; }

	void shutdown();
	void send(const std::string& message);

	// reading or not
	void startRead();
	void stopRead();
	bool isReading() const { return _reading; }; // NOT thread safe, may race with start/stopReadInLoop

	//FIXME: const boost::any& context？
	void setContext(HttpRequest context)
	{ _context = context; }
	
	//FIXME: const boost::any& getContext() const?
	HttpRequest getContext() const
    { return _context; }
	
	//FIXME: boost::any* getMutableContext()?
    HttpRequest* getMutableContext()
    { return &_context; }


	void setConnectionCallback(const EventCallback& cb)
	{ _connection_callback = cb; }

	void setMessageCallback(const MessageCallback& cb)
	{ _message_callback = cb; }

	void setWriteCompleteCallback(const EventCallback& cb)
	{ _write_complete_callback = cb; }

	void setCloseCallback(const EventCallback& cb)
	{ _close_callback = cb; }

	// called when TcpServer accepts a new connection
	void connectEstablished();   // should be called only once
	// called when TcpServer has removed me from its map
	void connectDestroyed();  // should be called only once

 private:
	enum StateE { 
		kDisconnected, 
		kDisconnecting, 
		kConnecting, 
		kConnected  
	};
	void handleRead();
	void handleWrite();
	void handleClose();
	void handleError();
	void sendInLoop(const std::string& message);
	//void sendInLoop(const StringPiece& message);
	void sendInLoop(const void* message, size_t len);
	void shutdownInLoop();
	// void shutdownAndForceCloseInLoop(double seconds);
	void forceCloseInLoop();
	void setState(StateE s) { _state = s; }
	const char* stateToString() const;
	void startReadInLoop();
	void stopReadInLoop();

	EventLoop* _loop;
	const std::string _name;
	StateE _state;  // FIXME: use atomic variable
	
	bool _reading;
	// we don't expose those classes to client.
	std::unique_ptr<Socket> _socket;
	std::unique_ptr<Channel> _channel;
	const InetAddress _local_addr;
	const InetAddress _peer_addr; 
	EventCallback _connection_callback;  //客户端连接建立成功后执行的回调函数
	MessageCallback _message_callback;     //接受到客户端传来的消息时执行的回调函数
	EventCallback _write_complete_callback;
	EventCallback _close_callback;
	//FIXME: boost::any _context？
	HttpRequest _context;
	// FIXME: creationTime_, lastReceiveTime_
	//        bytesReceived_, bytesSent_
	
	std::string _buf_write;
};


}//namespace serverlib