set(net_SRCS
  InetAddress.cpp
  Channel.cpp
  EventLoop.cpp
  EventLoopThread.cpp
  EventLoopThreadPool.cpp
  Poller.cpp
  EpollPoller.cpp
  Socket.cpp
  SocketOp.cpp
  TcpConnection.cpp
  TcpServer.cpp
  HttpRequest.cpp
  HttpResponse.cpp
  HttpServer.cpp
  )
  
add_library(serverlib_net ${net_SRCS})
target_link_libraries(serverlib_net serverlib_base)

install(TARGETS serverlib_net DESTINATION lib)

set(HEADERS
  InetAddress.h
  Channel.h
  EventLoop.h
  EventLoopThread.h
  EventLoopThreadPool.h
  Poller.h
  EpollPoller.h
  Socket.h
  SocketOp.h
  TcpConnection.h
  TcpServer.h
  HttpRequest.h
  HttpResponse.h
  HttpServer.h
  )
install(FILES ${HEADERS} DESTINATION include/serverlib/net)

# include_directories(${PROJECT_SOURCE_DIR}/base)
# add_executable(WebServer ${SRCS})
# target_link_libraries(WebServer libserver_base)