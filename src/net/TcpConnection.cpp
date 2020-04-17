#include <errno.h>

#include "src/net/TcpConnection.h"
#include "src/net/Channel.h"
#include "src/net/EventLoop.h"
#include "src/net/Socket.h"
#include "src/net/SocketOp.h"



using namespace serverlib;

void serverlib::defaultConnectionCallback(const TcpConnectionPtr& conn)
{
  printf("%s -> %s is %s\n", conn->localAddress().toIpPort().c_str() ,conn->peerAddress().toIpPort().c_str() ,(conn->connected() ? "UP" : "DOWN"));
}

void serverlib::defaultMessageCallback(const TcpConnectionPtr& conn, std::vector<char> &buf, int len)
{ 
}

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& name,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
  : _loop(loop), //_loop(CHECK_NOTNULL(loop)),
    _name(name),
    _state(kConnecting),
    _reading(true),
    _socket(new Socket(sockfd)),
    _channel(new Channel(loop, sockfd)),
    _local_addr(localAddr),
    _peer_addr(peerAddr)
{
    _channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
    
	printf( "TcpConnection::constructor/ctor[%s]  fd= %d\n" , _name.c_str(), sockfd);
    _socket->setKeepAlive(true);
}

TcpConnection::~TcpConnection(){
	printf("TcpConnection::deconstructor/dtor[%s] fd=%d state=%s\n" ,_name.c_str(), _channel->fd(), stateToString());
	assert(_state == kDisconnected);
}

void TcpConnection::shutdown(){
	if (_state == kConnected)  {
		setState(kDisconnecting);
		_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
	}
}

void TcpConnection::shutdownInLoop()
{
	_loop->assertInLoopThread();
	if (!_channel->isEnableWriting()) {
		// we are not writing
		_socket->shutdownWrite();
	}
}

void TcpConnection::send(const std::string& message){	
	if (_state == kConnected) {
		if (_loop->isInLoopThread()) {
		    sendInLoop(message);
		}
		else {
		    void (TcpConnection::*fp)(const std::string& message) = &TcpConnection::sendInLoop;
		    _loop->runInLoop(
			  std::bind(fp,
						this,   
						message));
		}
	}
}

void TcpConnection::sendInLoop(const std::string& message){
    sendInLoop(static_cast<const void*>(message.data()), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len){
	_loop->assertInLoopThread();
	ssize_t nwrote = 0;
	size_t remaining = len;
	bool faultError = false;
	if (_state == kDisconnected){
		return;
	}
	// if no thing in output queue, try writing directly
	if (!(_channel->isEnableWriting()== 0)) { //已开启写事件监听
		nwrote = sockets::write(_channel->fd(), data, len);
		if (nwrote >= 0) {
		    remaining = len - nwrote;
		    if (remaining == 0 && _write_complete_callback) {
				_buf_write.clear();
				if(_write_complete_callback)
					_loop->queueInLoop(std::bind(_write_complete_callback, shared_from_this()));
		    }
		}
		else // nwrote < 0
		{
		    nwrote = 0;
		    if (errno != EWOULDBLOCK) {
				perror("TcpConnection::sendInLoop");
				if (errno == EPIPE || errno == ECONNRESET){
				    faultError = true;
				}
		    }
		}
	}

    assert(remaining <= len);
	if (!faultError && remaining > 0) {
		//_buf_write中保存发送的message的内容
	    //使得下一次执行handlewrite时能够继续发送未完的信息
		if(_buf_write.empty()) 
			_buf_write = static_cast<const char*>(data)+nwrote;
		
		if (!_channel->isEnableWriting()) {
		    _channel->bindWritingEvent();   //_channel开启写事件监听
		}
	}
}

void TcpConnection::startRead(){
    _loop->runInLoop(std::bind(&TcpConnection::startReadInLoop, this));
}

void TcpConnection::startReadInLoop(){
	_loop->assertInLoopThread();
	if (!_reading || !_channel->isEnableReading()){
		_channel->bindReadingEvent();
		_reading = true;
	}
}

void TcpConnection::stopRead(){
    _loop->runInLoop(std::bind(&TcpConnection::stopReadInLoop, this));
}

void TcpConnection::stopReadInLoop(){
	_loop->assertInLoopThread();
	if (_reading || _channel->isEnableReading()) {
		_channel->unbindReadingEvent();
		_reading = false;
	}
}

void TcpConnection::connectEstablished(){
	_loop->assertInLoopThread();
	assert(_state == kConnecting);
	setState(kConnected);
	_channel->tieHolder(shared_from_this());
	_channel->bindReadingEvent();   //update epoller and start listening read event

	_connection_callback(shared_from_this());
}

void TcpConnection::connectDestroyed(){
	_loop->assertInLoopThread();
	if (_state == kConnected) {
		setState(kDisconnected);
		_channel->unbindAll();

		_connection_callback(shared_from_this());
	}
	_channel->remove();
}

void TcpConnection::handleRead(){
	printf("In TcpConnection::handleRead()...\n");
	_loop->assertInLoopThread();
	int savedErrno = 0;
	std::vector<char> buf;
	buf.clear();
	ssize_t n = sockets::readFd(_channel->fd(), buf, &savedErrno);
	printf("In fd %d: read buf len=%d, content=%s", _channel->fd(), n, &*(buf.begin()));
	if (n > 0){
		_message_callback(shared_from_this(), buf, n);
	}
	else if (n == 0){
		handleClose();
	}
	else {
		errno = savedErrno;
		perror("TcpConnection::handleRead");
		handleError();
	}
}

void TcpConnection::handleWrite(){
	printf("In TcpConnection::handleWrite()...\n");
	
	_loop->assertInLoopThread();
	
	if (_channel->isEnableWriting()) {
		//printf("Send to client:\n%s\n", _buf_write.c_str());
		//_buf结尾已加'\0'
		//ssize_t sockets::write(int sockfd, const void *buf, size_t count)
		ssize_t nwrote = sockets::write(_channel->fd(), &*(_buf_write.begin()), _buf_write.size());

		if (nwrote > 0){
		    if (static_cast<unsigned>(nwrote) == _buf_write.size()){ //_buf_write中的数据全部发送完毕
				_buf_write.clear();
				_channel->unbindWritingEvent();
				if (_write_complete_callback){
					_loop->queueInLoop(std::bind(_write_complete_callback, shared_from_this()));
				}
				if (_state == kDisconnecting){
				  shutdownInLoop();
				}
		    }
			else{ //nwrote < _buf_write.size(), 数据未发送完
				int remaining = _buf_write.size() - nwrote;
				_buf_write = _buf_write.substr(nwrote, remaining);
			}
		}
		else{//n <= 0
		    perror("TcpConnection::handleWrite");
		}
	}
	else { //_channel is unable to Write()
		printf("Connection fd = %d is down, no more writing\n" , _channel->fd());
	}
        
}

void TcpConnection::handleClose(){
	printf("In TcpConnection::handleClose()...\n");
	_loop->assertInLoopThread();
	printf("fd = %d state = %s" , _channel->fd() , stateToString());
	assert(_state == kConnected || _state == kDisconnecting );
	// we don't close fd, leave it to dtor, so we can find leaks easily.
	_buf_write.clear();
	setState(kDisconnected);
	_channel->unbindAll();

	TcpConnectionPtr guardThis(shared_from_this());
	_connection_callback(guardThis);
	// must be the last line
	_close_callback(guardThis);
}

void TcpConnection::handleError(){
	int err = sockets::getSocketError(_channel->fd());
	std::string info;
	info = "TcpConnection::handleError [" + _name + "] - SO_ERROR = "+ 
			to_string(err) + " "+  strerror(err);
	perror(info.c_str());
}

const char* TcpConnection::stateToString() const
{
  switch (_state) {
    case kDisconnected:
      return "kDisconnected";
    case kConnecting:
      return "kConnecting";
    case kConnected:
      return "kConnected";
    case kDisconnecting:
      return "kDisconnecting";
    default:
      return "unknown state";
  }
}

